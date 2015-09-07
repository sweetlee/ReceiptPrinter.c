//
//  printer_device_text.h
//  AdyenPOSTerminal
//
//  Created by Taras Kalapun on 12/3/14.
//
//

#ifndef printer_driver_text_
#define printer_driver_text_

#include <stdio.h>
#include "printer.h"

printer_driver *
printer_driver_text_new();

void
printer_driver_text_free(printer_driver *driver);

#endif
