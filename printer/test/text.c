#include "describe/describe.h"
#include "printer.h"

describe("Printer", {
    it("Prints to out_buf", {
        
        
        
        printer_t *printer = printer_new(PrinterTypeText);
        
        
        printer_add_line(printer, "N Line L", PrinterLineMarkupNone | PrinterLineMarkupAlignmentLeft);
        printer_add_line(printer, "N Line R", PrinterLineMarkupNone | PrinterLineMarkupAlignmentRight);
        printer_add_line(printer, "N Line C", PrinterLineMarkupNone | PrinterLineMarkupAlignmentCenter);
        
        printer_add_line(printer, "B Line L", PrinterLineMarkupBold | PrinterLineMarkupAlignmentLeft);
        printer_add_line(printer, "B Line R", PrinterLineMarkupBold | PrinterLineMarkupAlignmentRight);
        printer_add_line(printer, "B Line C", PrinterLineMarkupBold | PrinterLineMarkupAlignmentCenter);
        
        
        printer_connection_has_space(printer);
        
        int len = 0;
        char *buf = printer_output_bytes(printer, &len);
        
        assert_str_equal(buf, "N Line L (x01)\nN Line R (x02)\nN Line C (x03)\nB Line L (x05)\nB Line R (x06)\nB Line C (x07)\n");
        
        printer_free(printer);
        
        
    });

    
});