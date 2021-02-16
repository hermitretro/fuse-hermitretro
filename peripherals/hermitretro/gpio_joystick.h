/* gpio_joystick.h: GPIO joystick
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

#ifndef FUSE_PERIPHERALS_GPIO_JOYSTICK_H
#define FUSE_PERIPHERALS_GPIO_JOYSTICK_H

#include "gpio_joystick_pins.h"

int gpio_joystick_init( void *context );
void gpio_joystick_end( void );
void gpio_joystick_register_startup( void );

void gpio_joystick_poll( void );

int _gpio_joystick_init();

#endif			/* #ifndef FUSE_PERIPHERALS_GPIO_JOYSTICK_H */
