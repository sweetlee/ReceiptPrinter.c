//
//  printer_driver_bbamboo.c
//  AdyenPOSTerminal
//
//  Created by Taras Kalapun on 1/6/15.
//
//

#include "printer_driver_bbamboo.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#undef _printer_driver
#define _printer_driver "BlueBamboo"


// the P25 can handle 3000 bytes at a time, minus the 5 byte frame header
// but then it sends too many ACK messages (more than one for each frame).
// the docs state: For iOS limit, if baudrate is 57600, then max package should limit to 2k.
// I have no clue what the baudrate is, but limiting to 2K frames gives expected behaviour.
#define MAX_FRAME_SIZE (2000 - 5)
//#define MAX_FRAME_SIZE (35 - 5)

// sent by printer //

static unsigned char ACK = 0x03;

// sent to printer //

static unsigned char LF[] = { 0x0a };
static unsigned char PAPERFEED[] = { 0x1b, 0x4a, 0x77 };

//static unsigned char REVERSEON[] = { 0x1d, 0x42, 1 };
static unsigned char ALIGNLEFT[] = { 0x1b, 0x61, 0 };
static unsigned char ALIGNMIDDLE[] = { 0x1b, 0x61, 1 };
static unsigned char ALIGNRIGHT[] = { 0x1b, 0x61, 2 };
//static unsigned char INVERSEPRINTINGON[] = { 0x1d, 0x42, 1 };       // GS B     turn white/black reverse printing mode on/off
//static unsigned char INVERSEPRINTINGOFF[] = { 0x1d, 0x42, 0 };      // GS B
static unsigned char UNDERLINEON[] = { 0x1b, 0x2d, 2 };             // ESC -    underline on, line thickness 2
static unsigned char UNDERLINEOFF[] = { 0x1b, 0x2d, 0 };            // ESC -    underline off

static unsigned char OPERATION_FLAG[] = { 0x55, 0x66, 0x77, 0x88, 0x44 };  // Operation flag + operation type (print)

//0x00 - 32 dot, 0x01 - 24 dot....
static unsigned char STANDARD_MODE[] = { 0x1b, 0x21, 0x01 };        // ESC !    select print mode
static unsigned char SELECT_UTF8[] = { 0x1b, 0x52, 0x65 };          // ESC R    select international character set (utf8)


static int
drv_init(printer_driver_data *data) {
    data->dots_per_line = 384;
    data->characters_per_line = 24;
    return 0;
}

static int
drv_did_input_bytes(bytebuf *ob, PrinterState state, printer_driver_data *data) {
    _printer_log("drv_did_input_bytes");
    //if (state == PrinterStatePreparing) {
        // we only expect to receive the printWidth from the printer
        
        int length = bytebuf_size(ob);
        char *in_bytes = malloc(sizeof(char)* length);
        int size = bytebuf_take_head(ob, in_bytes, length);
        
        if (size <= 0) {
            return 0; // ACK not received
        }
        
        if (size >= 5 && in_bytes[4] == ACK) {
            free(in_bytes);
            return 1; // ACK received !
        } else {
            free(in_bytes);
        }
    //}
    return 0; // ACK not received
}

static int
drv_will_output(bytebuf *ob, bytebuf *content, PrinterState state, printer_driver_data *data) {
    _printer_log("drv_will_output, state: %i", state);
    
    if (state == PrinterStatePreparing) {
        // do we need this?
        bytebuf_append_bytes(ob, OPERATION_FLAG, 5);
        
        bytebuf_append_bytes(ob, STANDARD_MODE, 3);
        bytebuf_append_bytes(ob, SELECT_UTF8, 3);
    }
    
    else
        if (state == PrinterStateReadyForPrinting || state == PrinterStatePrinting) {
            
            int len = bytebuf_next_block_size(content);
            
            if (len == 0) {
                return 0;
            }
            
            
            bytebuf_append_bytes(ob, OPERATION_FLAG, 5);
            
            char buf [len];
            bytebuf_take_head_block(content, buf, len);
            bytebuf_append_bytes(ob, buf, len);
            //free(buf);
            
            //bytebuf_dump(ob, 1);
            
            return 1; // Waiting for ACK
        }
    return 0;
}

static int
drv_render_will_begin(bytebuf *ob, printer_driver_data *data) {
    _printer_log("rendering begin");
    
    ob->block_size = MAX_FRAME_SIZE;
    
    return 0;
}

static int
drv_render_end(bytebuf *ob, printer_driver_data *data) {
    _printer_log("rendering end");
    
    // Paper feed
    bytebuf_append_bytes(ob, PAPERFEED, 3);
    
    // Paper cut
    // not supported
    
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
        bytebuf_append_bytes(ob, UNDERLINEON, 3);
    }
    
    bytebuf_append_bytes(ob, str, (int)strlen(str));
    
    // End markup
    if(markup & 0x03) { // Revert back to left alignment
        bytebuf_append_bytes(ob, ALIGNLEFT, 3);
    }
    if(markup & PrinterLineMarkupBold) {
        bytebuf_append_bytes(ob, UNDERLINEOFF, 3);
    }
    
    bytebuf_append_bytes(ob, LF, 1);
    
    return 0;
}

static int
drv_render_image(bytebuf *ob, const unsigned char* bytes, uint16_t size, float width, float height, printer_driver_data *data)
{
    int num_lines = (int)height;
    //int bytesPerRow = size / num_lines;
    
    
    // print graphic (binary) ^GFB,byteCount,graphicFieldCount,bytesPerRow,binaryData(append after format)
    
    //int imageX = (int) ((data->dots_per_line - width) / 2);
    
    unsigned int l_width = size / num_lines;
    
    
    
    
    unsigned int maxLinesPerFrame = (MAX_FRAME_SIZE - 10) / l_width; // max frame size - markup & ESC bytes
    unsigned int requiredFrames = (num_lines/maxLinesPerFrame) + 1;
    
    //unsigned int max_w = maxLinesPerFrame * l_width;
    //char tmp_byte[max_w];
    
    for(int i=0; i<requiredFrames; i++) {
        int numLines = num_lines - (i*maxLinesPerFrame);
        if(numLines > maxLinesPerFrame)
            numLines = maxLinesPerFrame;
        
        // by removing the frameBuffer, we force the creation of a new one so
        // our image commands will be together in the new frame
        // TODO
        
        bytebuf_append_bytes_new_block(ob, ALIGNMIDDLE, 3);
        
        unsigned char printBitImageHorzMode[] = { 0x1b, 0x58, 0x31, (unsigned char) l_width, (unsigned char) numLines};
        bytebuf_append_bytes(ob, printBitImageHorzMode, 5);
        
        unsigned int index1 = i*maxLinesPerFrame * l_width;
        unsigned int tmp_size = numLines * l_width;

        char *tmp_byte = malloc(tmp_size);
        memcpy(tmp_byte, &bytes[index1], tmp_size);
        bytebuf_append_bytes(ob, tmp_byte, tmp_size);
        free(tmp_byte);
        
        //[self appendDataToFrameBuffer:[imageData subdataWithRange:NSMakeRange(i*maxLinesPerFrame*l_width, numLines * l_width)]];
    }
    
    bytebuf_append_bytes(ob, LF, 1);
    bytebuf_append_bytes(ob, ALIGNLEFT, 3);
    
    
    
//    char *s;
//    int s_size = asprintf(&s, "^FO%d,%d,%d^GFB,%d,%d,%d,",
//                          imageX, data->current_line_y, 0,
//                          size, (bytesPerRow * num_lines), bytesPerRow);
//    if (s) {
//        bytebuf_append_bytes(ob, s, s_size);
//        free(s);
//    }
//    
//    
//    bytebuf_append_bytes(ob, bytes, size);
//    
//    data->current_line_y += num_lines + 30;
    
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
printer_driver_bbamboo_new() {
    
    static const printer_driver cb_default = {
        NULL,
        
        drv_init,
        
        NULL,
        drv_did_input_bytes,
        drv_will_output,
        NULL,
        
        drv_render_will_begin,
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
printer_driver_bbamboo_free(printer_driver *driver) {
    free(driver);
}