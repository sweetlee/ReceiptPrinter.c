//
//  printer_driver_escpos.h
//  Shared-C
//
//  Created by Taras Kalapun on 12/4/14.
//  Copyright (c) 2014 Adyen. All rights reserved.
//

#ifndef printer_driver_escpos__
#define printer_driver_escpos__

#include <stdio.h>
#include "printer.h"

printer_driver *
printer_driver_escpos_new();

void
printer_driver_escpos_free(printer_driver *driver);

#endif 
