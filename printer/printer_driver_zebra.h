//
//  printer_driver_zebra.h
//  Shared-C
//
//  Created by Taras Kalapun on 12/4/14.
//  Copyright (c) 2014 Adyen. All rights reserved.
//

#ifndef printer_driver_zebra__
#define printer_driver_zebra__

#include <stdio.h>
#include "printer.h"

printer_driver *
printer_driver_zebra_new();

void
printer_driver_zebra_free(printer_driver *driver);

#endif