//
//  printer_device_text.c
//  AdyenPOSTerminal
//
//  Created by Taras Kalapun on 12/3/14.
//
//

#include "printer_driver_text.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#undef _printer_driver
#define _printer_driver "text"

static int
drv_init(printer_driver_data *data) {
    if (data->characters_per_line == 0) {
        data->characters_per_line = 42;
    }
    return 0;
}

static int
drv_render_line(bytebuf *ob, const char *str, const PrinterLineMarkup markup, printer_driver_data *data) {
    _printer_log("rendering line");
    
    int len = 0;
    int pad = 0;
    int str_len = (int)strlen(str);
    
    if (str_len > 0) {
        
        if ((markup & 0x03) == PrinterLineMarkupAlignmentLeft) {
            len = str_len;
        } else {
            len = data->characters_per_line;
            if ((markup & 0x03) == PrinterLineMarkupAlignmentRight) {
                pad = len - str_len;
            } else if ((markup & 0x03) == PrinterLineMarkupAlignmentCenter) {
                pad = (len - str_len) / 2;
                len = pad + str_len;
            }
            if (pad < 0) {
                len = str_len;
            }
        }
        
        len += 1; // 0-end
        char *buf = malloc(len);
        
        
        if (pad > 0) {
            snprintf(buf, len, "%*s%s", pad, " ", str);
        } else {
            snprintf(buf, len, "%s", str);
        }
        
        
        bytebuf_append_bytes(ob, buf, len-1);
        free(buf);
    }
    

    bytebuf_append_bytes(ob, "\n", 1);
    //free(buf);
    
    
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

static int
drv_will_output(bytebuf *ob, bytebuf *content, PrinterState state, printer_driver_data *data) {
    _printer_log("drv_will_output");
    //printer_driver_text_state *state_obj = data->opaque;
    
    if (state == PrinterStateReadyForPrinting) {
        //_printer_log("drv_will_output: taking head");
        int len = bytebuf_size(content);
        char buf [len];
        bytebuf_take_head(content, buf, len);
        bytebuf_append_bytes(ob, buf, len);
        //free(buf);
    }
    return 0; // Not waiting for ACK
}

static void
drv_did_output(PrinterState state, printer_driver_data *data) {
    _printer_log("drv_did_output");
    
//    // Next state
//    if (*state == PrinterStatePreparing) {
//        *state = PrinterStateReadyForPrinting;
//    } else if (*state == PrinterStateReadyForPrinting) {
//        *state = PrinterStatePrinting;
//    }
}

printer_driver *
printer_driver_text_new() {
    _printer_log("new");
    
    static const printer_driver cb_default = {
        NULL,
        
        drv_init,
        
        NULL,
        NULL,
        drv_will_output,
        drv_did_output,
        
        NULL,
        NULL,
        NULL,
        drv_render_line,
        NULL,
        drv_render_line_key_value
    };
    
    printer_driver *driver;
    
    /* Prepare the driver */
    driver = malloc(sizeof(printer_driver));
    memcpy(driver, &cb_default, sizeof(printer_driver));
    
    return driver;
}


void
printer_driver_text_free(printer_driver *driver) {
    free(driver);
}