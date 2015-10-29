//
//  printer_driver_escpos.c
//  Shared-C
//
//  Created by Taras Kalapun on 12/4/14.
//  Copyright (c) 2014 Adyen. All rights reserved.
//

#include "printer_driver_escpos.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#undef _printer_driver
#define _printer_driver "ESCPOS"

static unsigned char LF[] = { 0x0a };
static unsigned char PAPERFEED[] = { 27, 0x4a, 0xff };
static unsigned char PAPERCUT[] = { 0x1d, 0x56, 0x1 };
//static unsigned char REVERSEON[] = { 0x1d, 0x42, 1 };
static unsigned char ALIGNLEFT[] = { 0x1b, 0x61, 0 };
static unsigned char ALIGNMIDDLE[] = { 0x1b, 0x61, 1 };
static unsigned char ALIGNRIGHT[] = { 0x1b, 0x61, 2 };
static unsigned char BOLDON[] = { 0x1b, 0x45, 1 };
static unsigned char BOLDOFF[] = { 0x1b, 0x45, 0 };
static unsigned char INIT[] = { 0x1b, 0x40 };
static unsigned char SWITCH_COMMAND[] = { 0x1b, 0x69, 0x61, 0x00 };
static unsigned char STANDARD_MODE[] = { 0x1b, 0x53 };
static unsigned char FLUSH_COMMAND[] = {0xFF, 0x0C};

static int
drv_init(printer_driver_data *data) {
    if (data->dots_per_line == 0)
        data->dots_per_line = 504;
    if (data->characters_per_line == 0)
        data->characters_per_line = 42;
    return 0;
}

static int
drv_will_output(bytebuf *ob, bytebuf *content, PrinterState state, printer_driver_data *data) {
    _printer_log("drv_will_output, state: %i", state);
    
    if (state == PrinterStatePreparing) {
        bytebuf_append_bytes(ob, SWITCH_COMMAND, 4);
        bytebuf_append_bytes(ob, INIT, 2);
        bytebuf_append_bytes(ob, STANDARD_MODE, 2);
    }
    
    else
    if (state == PrinterStateReadyForPrinting) {
        int len = bytebuf_size(content);
        char buf [len];
        bytebuf_take_head(content, buf, len);
        bytebuf_append_bytes(ob, buf, len);
        //free(buf);
    }
    return 0;
}

static int
drv_render_end(bytebuf *ob, printer_driver_data *data) {
    _printer_log("rendering end");
    
    // Flush
    bytebuf_append_bytes(ob, FLUSH_COMMAND, 2);
    
    // Paper feed
    bytebuf_append_bytes(ob, PAPERFEED, 3);
    
    // Paper cut
    bytebuf_append_bytes(ob, PAPERCUT, 3);
    
    return 0;
}

static int
drv_render_line(bytebuf *ob, const char *str, const PrinterLineMarkup markup, printer_driver_data *data) {
    _printer_log("rendering line");
    
    // Start markup
    switch(markup & 0x03) {
        case PrinterLineMarkupAlignmentLeft:
            bytebuf_append_bytes(ob, ALIGNLEFT, 3);
            break;
        case PrinterLineMarkupAlignmentRight:
            bytebuf_append_bytes(ob, ALIGNRIGHT, 3);
            break;
        case PrinterLineMarkupAlignmentCenter:
            bytebuf_append_bytes(ob, ALIGNMIDDLE, 3);
            break;
        default:
            break;
    }
    if(markup & PrinterLineMarkupBold) {
        bytebuf_append_bytes(ob, BOLDON, 3);
    }
    
    bytebuf_append_bytes(ob, str, (int)strlen(str));
    
    // End markup
    if(markup & 0x03) { // Revert back to left alignment
        bytebuf_append_bytes(ob, ALIGNLEFT, 3);
    }
    if(markup & PrinterLineMarkupBold) {
        bytebuf_append_bytes(ob, BOLDOFF, 3);
    }
    
    bytebuf_append_bytes(ob, LF, 1);
    
    return 0;
}

static int
drv_render_image(bytebuf *ob, const unsigned char* bytes, uint16_t size, float width, float height, printer_driver_data *data) {
    
    
    unsigned int i_height = height;
    unsigned int i_width = size / i_height;
    
    // Make sure we align left, we use offset for alignment
    bytebuf_append_bytes(ob, ALIGNMIDDLE, 3);
    
    // Select 24 dots linespacing
    unsigned char linespacing24[] = { 0x1b, 0x33, 0x18 };
    bytebuf_append_bytes(ob, linespacing24, sizeof(linespacing24));
    
    for (int l=0; l < i_height; l += 24) {
        
        // Center logo (using alignment commands works on some, but not all printers)
        unsigned char offset[] = { 0x1b, 0x24, 0x00, 0x00 };
        unsigned int offsetDots = (data->dots_per_line - i_width * 8)/2;
        offset[2] = offsetDots & 0xff;
        offset[3] = offsetDots >> 8;
        bytebuf_append_bytes(ob, offset, sizeof(offset));
        
        unsigned char header[] = { 0x1b, 0x2a, 0x21, 0x00, 0x00 }; // 24 dots high, double density
        header[3] = (i_width*8) & 0xff;
        header[4] = (i_width*8) >> 8;
        bytebuf_append_bytes(ob, header, sizeof(header));
        
        for(int c=0; c < i_width*8; c++) { // Column
            for(int b=0; b<3; b++) { //
                unsigned char value = 0;
                for(int bit=0; bit<8; bit++) {
                    unsigned int index = l * i_width + b * i_width * 8 + bit * i_width  + c / 8;
                    unsigned char byte = index < size ? bytes[index] : 0;
                    if(byte & 1 << (7-(c&7))) {
                        value |= 1 << (7-bit);
                    }
                }
                bytebuf_append_bytes(ob, &value, 1);
                
            }
        }
        bytebuf_append_bytes(ob, LF, 1);
    }
    
    // Back to default linespacing
    unsigned char defaultLinespacing[] = { 0x1b, 0x32 };
    bytebuf_append_bytes(ob, defaultLinespacing, sizeof(defaultLinespacing));
    
    return 0;
}

static int
drv_render_line_key_value(bytebuf *ob, const char *key, const char *value, const PrinterLineMarkup markup, printer_driver_data *data) {
    _printer_log("rendering line key & val");
    
    int len = data->characters_per_line;
    
    //+1 is because we need space between
    int pad = len - (int)(strlen(key) + strlen(value) +1);
    
    PrinterLineMarkup mk = PrinterLineMarkupNone;
    if (markup & PrinterLineMarkupBold) {
        mk |= PrinterLineMarkupBold;
    }
    
    if (pad >= 0) {
        len += 1; // 0-end
        char *buf = malloc(len);
        snprintf(buf, len, "%s %*s%s", key, pad, " ", value);
        drv_render_line(ob, buf, mk, data);
        free(buf);
    } else {
        // pad is negative, split key & val on different lines
        drv_render_line(ob, key, PrinterLineMarkupAlignmentLeft | mk, data);
        drv_render_line(ob, value, PrinterLineMarkupAlignmentRight | mk, data);
    }
    
    return 0;
}

printer_driver *
printer_driver_escpos_new() {
    
    static const printer_driver cb_default = {
        NULL,
        
        drv_init,
        
        NULL,
        NULL,
        drv_will_output,
        NULL,
        
        NULL,
        NULL,
        drv_render_end,
        drv_render_line,
        drv_render_image,
        drv_render_line_key_value
    };
    

    printer_driver *driver;
    
    
    /* Prepare the driver */
    driver = malloc(sizeof(printer_driver));
    memcpy(driver, &cb_default, sizeof(printer_driver));
    
//    driver->opaque = state;
    
    return driver;
}


void
printer_driver_escpos_free(printer_driver *driver) {
    free(driver);
}

