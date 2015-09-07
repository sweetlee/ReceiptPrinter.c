# Printer.c
C code that converts one byte stream into other depending on printer driver.

Current implementations:
* ZPL (Zebra)
* ESCPOS (Epson)
* BlueBamboo
* Text (for testing ?)

Also includes `bytebuf` (modified)

## Example

### Objective-C

``` obj-c
printer_t *printer;

- (void)dealloc {
    // dealloc streams
    
    if (printer) {
        printer_free(printer);
        printer = NULL;
    }
}

- (void)initPrinter {
    
    if (!printer) {
        printer = printer_new(self.printerType);
    }
    
    if (printer) {
        if (self.charactersPerLine > 0) {
            printer->data.characters_per_line = self.charactersPerLine;
        }
        if (self.dotsPerLine > 0) {
            printer->data.dots_per_line = self.dotsPerLine;
        }
        
        printer_init(printer);
    }
}

- (void)addImage:(UIImage *)image {
    CGSize size = image.size;
    int contentWidth = printer_content_width(printer);
    float scale = 1.0;
    
    // make sure the image fits on the receipt
    if (size.width > contentWidth) {
        scale = contentWidth / size.width;
        scale = MIN(1.0f, scale); // never increase size
        size = CGSizeMake( (NSInteger)(size.width * scale), (NSInteger)(size.height * scale) );
    }
    
    NSData *data = [image generateMonochromeBitmapAtSize:size];
    printer_add_image_data(printer, data.bytes, data.length, size.width, size.height);
}

- (void)addReceiptLines:(NSArray*)lines {
	// for loop
	printer_add_line_key_value(printer,
                                       (char *)line.name.UTF8String,
                                       (char *)line.value.UTF8String,
                                       (int)markup);
}

- (void)renderReceipt {

    printer_content_begin(printer);
    
    if (self.receiptLogo) {
        [self addImage:self.receiptLogo];
    }
    
    [self addReceiptLines:receipt.header];
    [self addReceiptLines:receipt.content];
    [self addReceiptLines:receipt.footer];
    
    printer_content_end(printer);
    printer_print(printer);
}

- (void)streamHasSpaceAvailable {
    if (printer == NULL) {
        return;
    }
    
    if (printer_state_finished(printer)) {
        [self printFinishedWithError:nil];
        return;
    }
    
    if (printer_state_ready(printer)) {
        [self renderReceipt];
    }
    
    // Printer wants something to say?
    int len = 0;
    char *bytes = printer_output_bytes(printer, &len);
    
    if (bytes && len > 0) {
        [_outputStream write:(void *)bytes maxLength:len];
    }
    
    if (printer_state_finished(printer)) {
        [self printFinishedWithError:nil];
        return;
    }
}

```
