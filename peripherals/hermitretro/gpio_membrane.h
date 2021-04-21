/* gpio_membrane.h: GPIO Membrane keyboard
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

#ifndef FUSE_PERIPHERALS_GPIO_MEMBRANE_H
#define FUSE_PERIPHERALS_GPIO_MEMBRANE_H

#include "gpio_membrane_pins.h"

#define NUM_COLS 5
#define NUM_ROWS 8

/** Shift key position */
#define SHIFT_ROW 5
#define SHIFT_COL 4

/** Symbol shift position */
#define SYMBOL_SHIFT_ROW 7
#define SYMBOL_SHIFT_COL 3

extern unsigned char keysPressed[NUM_ROWS][NUM_COLS];

int gpio_membrane_init( void *context );
void gpio_membrane_end( void );
void gpio_membrane_register_startup( void );

void gpio_membrane_poll( void );

int _gpio_membrane_init();

#endif			/* #ifndef FUSE_PERIPHERALS_GPIO_MEMBRANE_H */
