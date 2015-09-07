//
//  printer.c
//
//  Created by Taras Kalapun on 11/12/14.
//  Copyright (c) 2014 Taras Kalapun. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "printer.h"

#include "printer_driver_text.h"
#include "printer_driver_escpos.h"
#include "printer_driver_zebra.h"
#include "printer_driver_bbamboo.h"

static void
printer_clear_buffers(printer_t *printer) {
    if (!printer) return;
    
    if (printer->content) bytebuf_clear(printer->content);
    if (printer->in_buf) bytebuf_clear(printer->in_buf);
    if (printer->out_buf) bytebuf_clear(printer->out_buf);
}

static int
printer_change_state(printer_t *printer) {
    if (!printer) return -1;
    
    if (printer->waiting_for_ack) {
        _printer_log("state can't change. waiting for ACK.");
        return 0;
    }
    
    if (printer->next_state != printer->state) {
        _printer_log("state %i -> %i", printer->state, printer->next_state);
        printer->state = printer->next_state;
    }
    
    
    // If we really are finished, clear buffers
    if (printer->state == PrinterStateFinished) {
        printer->allow_printing = 0;
        printer_clear_buffers(printer);
    }
    return 1;
}

printer_t *
printer_new(PrinterType type) {
    
    printer_t *printer;
    printer_driver *driver;
    
    switch (type) {
//        case PrinterTypeBlueBamboo:
        case PrinterTypeESCPOS:
            driver = printer_driver_escpos_new();
            break;
        case PrinterTypeZebra:
            driver = printer_driver_zebra_new();
            break;
        case PrinterTypeBlueBamboo:
            driver = printer_driver_bbamboo_new();
            break;
            
        case PrinterTypeText:
            driver = printer_driver_text_new();
            break;
        default:
            driver = printer_driver_text_new();
            break;
    }
    
    /* Prepare the driver */
    printer = malloc(sizeof(printer_t));
    printer->type = type;
    
    printer->driver = driver;

    printer->content = bytebuf_create();
    printer->in_buf = bytebuf_create();
    printer->out_buf = bytebuf_create();
    
    printer->waiting_for_ack = 0;
    
    printer->data.dots_per_line = 0;
    printer->data.characters_per_line = 0;
    printer->data.current_line_y = 0;
    
    
    return printer;
}

void
printer_free(printer_t *printer) {
    if (!printer) return;
    
    // Free the printer driver
    switch (printer->type) {
            //        case PrinterTypeBlueBamboo:
        case PrinterTypeESCPOS:
            printer_driver_escpos_free(printer->driver);
            break;
        case PrinterTypeZebra:
            printer_driver_zebra_free(printer->driver);
            break;
        case PrinterTypeBlueBamboo:
            printer_driver_bbamboo_free(printer->driver);
            break;
            
        case PrinterTypeText:
            printer_driver_text_free(printer->driver);
            break;
        default:
            break;
    }
    
    // Free the buffers
    if (printer->content) bytebuf_dispose(printer->content);
    if (printer->in_buf) bytebuf_dispose(printer->in_buf);
    if (printer->out_buf) bytebuf_dispose(printer->out_buf);
    
    free(printer);
}



void
printer_init(printer_t *printer) {
    if (!printer) return;
    
    printer->waiting_for_ack = 0;
    printer->allow_printing = 0;
    
    if (printer->driver->init) {
        int ack = printer->driver->init(&printer->data);
        if (ack) {
            _printer_log("state * -> PrinterStatePreparing");
            printer->state = PrinterStatePreparing;
        } else {
            _printer_log("state * -> PrinterStateReadyForPrinting");
            printer->state = PrinterStateReadyForPrinting;
        }
    } else {
        _printer_log("state * -> PrinterStatePreparing");
        printer->state = PrinterStatePreparing;
    }
    
    
    printer_clear_buffers(printer);
}

void
printer_print(printer_t *printer) {
    if (!printer) return;
    printer->allow_printing = 1;
}

int
printer_waiting(printer_t *printer) {
    if (!printer) return -1;
    return printer->waiting_for_ack;
}

int
printer_state_ready(printer_t *printer) {
    if (!printer) return -1;
    
    if (printer->state == PrinterStateReadyForPrinting) {
        return 1;
    }
    return 0;
}

int
printer_state_finished(printer_t *printer) {
    if (!printer) return -1;
    
    if (!printer->waiting_for_ack &&
        printer->state == PrinterStatePrinting &&
        bytebuf_size(printer->content) == 0)
    {
        printer->next_state = PrinterStateFinished;
        printer_change_state(printer);
    }
    
    if (printer->state == PrinterStateFinished) {
        return 1;
    }
    return 0;
}


void
printer_connection_has_bytes(printer_t *printer) {
//    printer->driver->will_input_bytes();
}

void
printer_connection_has_space(printer_t *printer) {
//    printer->driver->will_output_bytes();
    
//    if (printer->state == PrinterStatePreparing) {
//        printer->state = PrinterStateReadyForPrinting;
//    }
//    else if (printer->state == PrinterStatePrinting) {
//        
//    }
}

void *
printer_output_bytes(printer_t *printer, int* length) {
    if (!printer) return NULL;
    
    for (int i = printer->state; i<=PrinterStateFinished; i++) {
        
        // Skip if waiting for ACK
        if (printer->waiting_for_ack) {
            _printer_log("printer_output_bytes SKIPPING");
            return NULL;
        }
        
        // Skip if not allowed to print
        if (printer->state == PrinterStatePreparing && !printer->allow_printing) {
            return NULL;
        }
        
        
        if (!printer->out_buf) {
            printer->out_buf = bytebuf_create();
        }
        
        if (printer->driver->will_output_bytes)
            printer->waiting_for_ack = printer->driver->will_output_bytes(printer->out_buf, printer->content, printer->state, &printer->data);
        
        char *data = NULL;
        
        if (bytebuf_size(printer->out_buf) > 0) {
            data = bytebuf_convert_to_bytes(printer->out_buf, length);
            // nil out_buf after conversion
            printer->out_buf = NULL;
        }
        
        
        // Assume Printing if we were Ready and there is a data
        if (data && printer->state == PrinterStateReadyForPrinting) {
            printer->next_state = PrinterStatePrinting;
            printer->state = PrinterStatePrinting;
        }
        else if (i<PrinterStateFinished) {
            printer->next_state = i+1;
        }
        
        printer_change_state(printer);
        
        if (data) {
            
            if (printer->driver->did_output_bytes)
                printer->driver->did_output_bytes(printer->state, &printer->data);
            
            return data;
        }
        
    }
    return NULL;
}

int
printer_input_write(printer_t *printer, const void *buffer, size_t len) {
    if (!printer) return -1;
    
    if (!buffer || len == 0) {
        return 0;
    }
    
    if (!printer->in_buf) {
        printer->in_buf = bytebuf_create();
    }
    
//    if (printer->driver->will_input_bytes)
//        printer->driver->will_input_bytes(printer->in_buf, &printer->state, &printer->data);
//    
    int r =  bytebuf_append_bytes(printer->in_buf, buffer, (int)len);
    
    int ack_received = 1;
    if (printer->driver->did_input_bytes)
        ack_received = printer->driver->did_input_bytes(printer->in_buf, printer->state, &printer->data);
    
    if (printer->waiting_for_ack) {
        
        if (ack_received) {
            printer->waiting_for_ack = 0;
            // now we can change printer state
            printer_change_state(printer);
        }
    }
    
    return r;
}

#pragma mark - Content

int printer_content_width(printer_t *printer) {
    if (!printer) return 0;
    
    return printer->data.dots_per_line;
}

int printer_content_begin(printer_t *printer) {
    if (!printer) return 0;
    
    // Disallow printing
    printer->allow_printing = 0;
    
    // Actual header will be added in the end
    
    if (printer->content == NULL) {
        printer->content = bytebuf_create();
    }
    
    if (printer->driver->render_will_begin)
        printer->driver->render_will_begin(printer->content, &printer->data);
    
    return 0;
}

int printer_content_end(printer_t *printer) {
    if (!printer) return 0;
    
    if (printer->content == NULL) {
        printer->content = bytebuf_create();
    }
    
    // Begin head
    if (printer->driver->render_begin) {
        // Dump the content
        int size;
        //int block_size = printer->content->block_size;
        char *bytes = bytebuf_convert_to_bytes(printer->content, &size);
        
        // Create new one
        printer->content = bytebuf_create();
        
        // start head
        printer->driver->render_begin(printer->content, &printer->data);
        

        // Restore block size if applicable
        //printer->content->block_size = block_size;
        
        // Add content
        bytebuf_append_bytes(printer->content, bytes, size);
        free(bytes);
    }
    
    // End head
    if (printer->driver->render_end)
        printer->driver->render_end(printer->content, &printer->data);
    
    // Allow printing
    printer->allow_printing = 1;
    
    return 0;
}


int
printer_add_line(printer_t *printer, char *line, PrinterLineMarkup markup) {
    if (!printer) return 0;
    
    _printer_log("adding line");
    
    if (printer->content == NULL) {
        printer->content = bytebuf_create();
    }
    
    char *buf = line;
    
    // extend the --- to full line
    if (strcmp("---", line) == 0) {
        int len = printer->data.characters_per_line;
        if (len > 3) {
            buf = (char *)malloc(len+1);
            memset(buf, '-', len);
            buf[len] = 0;
        }
    
    }
    
    if (printer->driver->render_line)
        printer->driver->render_line(printer->content, buf, markup, &printer->data);
    
    if (buf != line) free(buf);
    
    return 0;
}

int
printer_add_line_key_value(printer_t *printer, char *key, char *value, PrinterLineMarkup markup) {
    if (!printer) return 0;
    
    _printer_log("adding line with key & value");
    
    if (printer->content == NULL) {
        printer->content = bytebuf_create();
    }
    
    if (key == NULL) {
        key = "";
    }
    
    if (value == NULL) {
        value = "";
    }
    
    if (printer->driver->render_line_key_value)
        printer->driver->render_line_key_value(printer->content, key, value, markup, &printer->data);
    
    return 0;
}

int
printer_add_image_data(printer_t *printer, const unsigned char* bytes, uint16_t size, float width, float height) {
    if (!printer) return 0;
    
    if (printer->driver->render_image)
        printer->driver->render_image(printer->content, bytes, size, width, height, &printer->data);
    
    return 0;
}
