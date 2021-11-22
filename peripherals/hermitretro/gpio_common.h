/* gpio_common.h: common GPIO functions
   Copyright (c) 2017-2021 Alligator Descartes <https://hermitretro.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

   Author contact information:

   E-mail: philip-fuse@shadowmagic.org.uk
   E-mail: alligator.descartes@hermitretro.com

*/

#ifndef FUSE_PERIPHERALS_GPIO_COMMON_H
#define FUSE_PERIPHERALS_GPIO_COMMON_H

#include "bcm2835.h"

extern int gpioInit;

int debounceEvent( unsigned long long debounceInterval );
void updateLastEventTime();

int gpio_common_init( void *context );
void gpio_common_end( void );
void gpio_common_register_startup( void );

#endif			/* #ifndef FUSE_PERIPHERALS_GPIO_COMMON_H */
