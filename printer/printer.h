//
//  printer.h
//
//  Created by Taras Kalapun on 11/12/14.
//  Copyright (c) 2014 Taras Kalapun. All rights reserved.
//

#ifndef __printer
#define __printer

#include <stdint.h>
#include "bytebuf/bytebuf.h"


//#define _printer_log(fmt, ...) printf("Printer " _printer_driver ": " fmt "\n", ##__VA_ARGS__)
#define _printer_log(fmt, ...)
#define _printer_driver ""

typedef enum {
    PrinterTypeAutoDetect,
    PrinterTypeESCPOS,
    PrinterTypeZebra,
    PrinterTypeBlueBamboo,
    PrinterTypeText,
} PrinterType;

typedef enum {
    PrinterStatePreparing = 0,
    PrinterStateReadyForPrinting,
    PrinterStatePrinting,
    PrinterStateFinished // final state
} PrinterState;

typedef enum {
    PrinterLineMarkupNone = 0,
    
    PrinterLineMarkupAlignmentLeft = 0x01,
    PrinterLineMarkupAlignmentRight = 0x02,
    PrinterLineMarkupAlignmentCenter = 0x03,
    
    PrinterLineMarkupBold = 0x04
} PrinterLineMarkup;


struct printer_driver_data {
    uint16_t dots_per_line;
    uint16_t characters_per_line;
//    int input_buffer_size;
    int current_line_y;
};
typedef struct printer_driver_data printer_driver_data;

struct printer_driver {
    void *opaque; // state object
    
    int (*init)(printer_driver_data *data);
    
    /* connection callbacks - NULL skips the block */
    void (*will_input_bytes)(bytebuf *ob, PrinterState state, printer_driver_data *data);
    int (*did_input_bytes)(bytebuf *ob, PrinterState state, printer_driver_data *data);
    int (*will_output_bytes)(bytebuf *ob, bytebuf *content, PrinterState state, printer_driver_data *data);
    void (*did_output_bytes)(PrinterState state, printer_driver_data *data);
    
    /* rendering callbacks - NULL skips the block */
    int (*render_will_begin)(bytebuf *ob, printer_driver_data *data);
    int (*render_begin)(bytebuf *ob, printer_driver_data *data);
    int (*render_end)(bytebuf *ob, printer_driver_data *data);
    int (*render_line)(bytebuf *ob, const char *str, const PrinterLineMarkup markup, printer_driver_data *data);
    int (*render_image)(bytebuf *ob, const unsigned char* bytes, uint16_t size, float width, float height, printer_driver_data *data);
    int (*render_line_key_value)(bytebuf *ob, const char *key, const char *value, const PrinterLineMarkup markup, printer_driver_data *data);
};
typedef struct printer_driver printer_driver;

typedef struct {
    PrinterType type;
    PrinterState state;
    PrinterState next_state;

    printer_driver *driver;
    printer_driver_data data;
    
    int waiting_for_ack;
    int allow_printing;
    
    bytebuf *content;
    bytebuf *in_buf;
    bytebuf *out_buf;
    
} printer_t;

/// @name New / Free

printer_t *
printer_new(PrinterType type);

void
printer_free(printer_t *printer);

void
printer_init(printer_t *printer);

void
printer_print(printer_t *printer);

int
printer_waiting(printer_t *printer);

int
printer_state_ready(printer_t *printer);

int
printer_state_finished(printer_t *printer);


/// @name I/O

void
printer_connection_has_bytes(printer_t *printer);

void
printer_connection_has_space(printer_t *printer);

size_t
printer_output_size(printer_t *printer);

// printer -> stream
void *
printer_output_bytes(printer_t *printer, int* length);

// stream -> printer
int
printer_input_write(printer_t *printer, const void *buffer, size_t len);



/// @name Content

int printer_content_width(printer_t *printer);

int printer_content_begin(printer_t *printer);
int printer_content_end(printer_t *printer);
int printer_add_line_key_value(printer_t *printer, char *key, char *value, PrinterLineMarkup markup);
int printer_add_line(printer_t *printer, char *line, PrinterLineMarkup markup);
int printer_add_image_data(printer_t *printer, const unsigned char* bytes, uint16_t size, float width, float height);

#endif
