//
//  printer_driver_zebra.c
//  Shared-C
//
//  Created by Taras Kalapun on 12/4/14.
//  Copyright (c) 2014 Adyen. All rights reserved.
//

#include "printer_driver_zebra.h"


#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "printer_strings.h"

#undef _printer_driver
#define _printer_driver "Zebra"


static int
drv_init(printer_driver_data *data) {
    data->dots_per_line = 386;
    data->characters_per_line = 0; // not used
    return 0;
}

static int
drv_did_input_bytes(bytebuf *ob, PrinterState state, printer_driver_data *data) {
    _printer_log("drv_did_input_bytes");
    if (state == PrinterStatePreparing) {
        // we only expect to receive the printWidth from the printer
        
        int length = bytebuf_size(ob);
        char *in_bytes = malloc(sizeof(char)* length);
        int size = bytebuf_take_head(ob, in_bytes, length);

        if (size <= 0) {
            return 1; // ACK received
        }
        
        int printWidth = str_extract_int(in_bytes);
        free(in_bytes);

        _printer_log("drv_did_input_bytes found print width: %d", printWidth);
        
        if (printWidth > 0) {
            data->dots_per_line = printWidth;
        }
    }
    return 1; // ACK received
}

static int
drv_will_output(bytebuf *ob, bytebuf *content, PrinterState state, printer_driver_data *data) {
    _printer_log("drv_will_output, state: %i", state);
    
    if (state == PrinterStatePreparing) {
        
        // command to get dots_per_line
        const char *cmd = "! U1 setvar \"device.languages\" \"zpl\"\r\n! U1 getvar \"ezpl.print_width\" \r\n";
        bytebuf_append_bytes(ob, cmd, (int)strlen(cmd));
        
        // we are waiting for responce (ACK)
        return 1;
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
drv_render_will_begin(bytebuf *ob, printer_driver_data *data) {
    _printer_log("drv_render_will_begin");
    data->current_line_y = 100;
    return 0;
}

static int
drv_render_begin(bytebuf *ob, printer_driver_data *data) {
    _printer_log("drv_render_begin");
//    static char *labelStart     = "^XA";
//    static char *continuousMode = "^MNN";
//    static char *labelHome      = "^LH0,0";
//    static char *labelLength    = "^LL%d";
    
    char *s;
    // start, continuousMode, home, length
    int size = asprintf(&s, "^XA^MNN^LH0,0^LL%d", data->current_line_y + 60);
    if (s) {
        bytebuf_append_bytes(ob, s, size);
        free(s);
    }
    return 0;
}

static int
drv_render_end(bytebuf *ob, printer_driver_data *data) {
    _printer_log("drv_render_end");
    static char *labelEnd = "^XZ";
    bytebuf_append_bytes(ob, labelEnd, sizeof(labelEnd));
    return 0;
}


static int
drv_render_line_nl(bytebuf *ob, const char *str, const PrinterLineMarkup markup, printer_driver_data *data, int nl) {
    _printer_log("rendering line");

//    static char *setVarCommand    = "! U1 setvar";
//    static char *getVarCommand    = "! U1 getvar";
//    static char *printWidth       = "ezpl.print_width";

    // font: ^A0(fontNr)N(ormal),height,width
    static char *fontEmphasized   = "^A0N,26,31";
    static char *fontNormal       = "^A0N,20,24";
    static const int horizontalMargin         = 10;
    static const int emphasizedFontLineHeight = 31;
    static const int normalFontLineHeight     = 27;
    
    // ^FOx,y,justification (0=left, 1=right, 2=auto)
    static char *fieldOrigin      = "^FO%d,%d,%d";
    
    // width of label, max lines, space between lines, justification (L,C,R,J), hanging indent
    static char *fieldOriginAndBlock = "^FO%d,%d,%d^FB%d,%d,%d,%s,%d";
    
//    static char *fieldData        = "^FD";
//    static char *fieldSeparator   = "^FS";
    
    
    
    
    // Start markup
    switch(markup & 0x03) {
        case PrinterLineMarkupAlignmentLeft: {
            char *s;
            int size = asprintf(&s, fieldOrigin, horizontalMargin, data->current_line_y, 0);
            if (s) {
                bytebuf_append_bytes(ob, s, size);
                free(s);
            }
        }
            break;
        case PrinterLineMarkupAlignmentRight: {
            int margin = data->dots_per_line - horizontalMargin;
            char *s;
            int size = asprintf(&s, fieldOrigin, margin, data->current_line_y, 1);
            if (s) {
                bytebuf_append_bytes(ob, s, size);
                free(s);
            }
        }
            break;
        case PrinterLineMarkupAlignmentCenter: {
            // vars: width of label, max lines, space between lines, justification (L,C,R,J), hanging indent
            char *s;
            int size = asprintf(&s, fieldOriginAndBlock,
                                0, data->current_line_y, 0,
                                data->dots_per_line, 1, 0, "C", 0);
            if (s) {
                bytebuf_append_bytes(ob, s, size);
                free(s);
            }
        }
            break;
        default:
            break;
    }
    if (markup & PrinterLineMarkupBold) {
        bytebuf_append_bytes(ob, fontEmphasized, 10);
    } else {
        bytebuf_append_bytes(ob, fontNormal, 10);
    }
    
    // Increase line number
    if (nl)
        data->current_line_y += markup & PrinterLineMarkupBold ? emphasizedFontLineHeight : normalFontLineHeight;
    
    // add some extra space after each block of lines (header, body, footer)
    //data->current_line_y += 30;
    
    
    char *s;
    int size = asprintf(&s, "^FD%s^FS", str);
    if (s) {
        bytebuf_append_bytes(ob, s, size);
        free(s);
    }
    
    return 0;
}

static int
drv_render_line(bytebuf *ob, const char *str, const PrinterLineMarkup markup, printer_driver_data *data) {
    _printer_log("rendering line");
    return drv_render_line_nl(ob, str, markup, data, 1);
}

static int
drv_render_line_key_value(bytebuf *ob, const char *key, const char *value, const PrinterLineMarkup markup, printer_driver_data *data) {
    _printer_log("rendering line key & val");
    
    PrinterLineMarkup mk = PrinterLineMarkupNone;
    if (markup & PrinterLineMarkupBold) {
        mk |= PrinterLineMarkupBold;
    }
    
    drv_render_line_nl(ob, key, PrinterLineMarkupAlignmentLeft | mk, data, 0);
    drv_render_line_nl(ob, value, PrinterLineMarkupAlignmentRight | mk, data, 0);
    
    static const int emphasizedFontLineHeight = 31;
    static const int normalFontLineHeight     = 27;
    
    // Increase line number
    data->current_line_y += markup & PrinterLineMarkupBold ? emphasizedFontLineHeight : normalFontLineHeight;
    
    return 0;
}

static int
drv_render_image(bytebuf *ob, const unsigned char* bytes, uint16_t size, float width, float height, printer_driver_data *data) {
    int num_lines = height;
    int bytesPerRow = size / num_lines;
    
    
    // print graphic (binary) ^GFB,byteCount,graphicFieldCount,bytesPerRow,binaryData(append after format)
    
    int imageX = (int) ((data->dots_per_line - width) / 2);
    
    //static char *fieldOrigin      = "^FO%d,%d,%d";
    // print graphic (binary) ^GFB,byteCount,graphicFieldCount,bytesPerRow,binaryData(append after format)
    //static char *printGraphic     = "^GFB,%d,%d,%d,";
    
    //[self appendFormatToData:fieldOrigin, imageX, currentLineY, 0];
    //[self appendFormatToData:printGraphic, imageData.length, bytesPerRow * numLines, bytesPerRow];

    char *s;
    int s_size = asprintf(&s, "^FO%d,%d,%d^GFB,%d,%d,%d,",
                        imageX, data->current_line_y, 0,
                        size, (bytesPerRow * num_lines), bytesPerRow);
    if (s) {
        bytebuf_append_bytes(ob, s, s_size);
        free(s);
    }
    
    
    bytebuf_append_bytes(ob, bytes, size);
    
    data->current_line_y += num_lines + 30;
    
    return 0;
}

printer_driver *
printer_driver_zebra_new() {
    
    static const printer_driver cb_default = {
        NULL,
        
        drv_init,
        
        NULL,
        drv_did_input_bytes,
        drv_will_output,
        NULL,
        
        drv_render_will_begin,
        drv_render_begin,
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
printer_driver_zebra_free(printer_driver *driver) {
    free(driver);
}

