//
//  printer_driver_bbamboo.h
//  AdyenPOSTerminal
//
//  Created by Taras Kalapun on 1/6/15.
//
//

#ifndef printer_driver_bbamboo__
#define printer_driver_bbamboo__

#include <stdio.h>
#include "printer.h"

printer_driver *
printer_driver_bbamboo_new();

void
printer_driver_bbamboo_free(printer_driver *driver);

#endif 
