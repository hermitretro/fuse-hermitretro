/* gpio_joystick_pins.h: GPIO joystick pins
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

#ifndef FUSE_PERIPHERALS_GPIO_JOYSTICK_PINS_H
#define FUSE_PERIPHERALS_GPIO_JOYSTICK_PINS_H

#include "bcm2835.h"

/** BCM2835 physical pin numbers for the various GPIO pins */
#ifdef BUILD_GPIO_JOYSTICK
#define PERIPHERALS_GPIO_JOYSTICK_UP_PIN RPI_BPLUS_GPIO_J8_07
#define PERIPHERALS_GPIO_JOYSTICK_DOWN_PIN RPI_BPLUS_GPIO_J8_08
#define PERIPHERALS_GPIO_JOYSTICK_LEFT_PIN RPI_BPLUS_GPIO_J8_10
#define PERIPHERALS_GPIO_JOYSTICK_RIGHT_PIN RPI_BPLUS_GPIO_J8_12
#define PERIPHERALS_GPIO_JOYSTICK_FIRE1_PIN RPI_BPLUS_GPIO_J8_11
#define PERIPHERALS_GPIO_JOYSTICK_FIRE2_PIN RPI_BPLUS_GPIO_J8_13
#else
#error "GPIO joystick code for unknown platform"
#endif

#endif			/* #ifndef FUSE_PERIPHERALS_GPIO_JOYSTICK_PINS_H */
