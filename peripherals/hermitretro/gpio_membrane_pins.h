/* gpio_membrane_pins.h: GPIO membrane keyboard pins
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

#ifndef FUSE_PERIPHERALS_GPIO_MEMBRANE_PINS_H
#define FUSE_PERIPHERALS_GPIO_MEMBRANE_PINS_H

#include "bcm2835.h"

/** BCM2835 physical pin numbers for the various keyboard membrane pins */
#ifdef BUILD_HERMITRETRO_ZXZERO
#define PERIPHERALS_GPIO_MEMBRANE_5_0_PIN RPI_BPLUS_GPIO_J8_37
#define PERIPHERALS_GPIO_MEMBRANE_5_1_PIN RPI_BPLUS_GPIO_J8_35
#define PERIPHERALS_GPIO_MEMBRANE_5_2_PIN RPI_BPLUS_GPIO_J8_40
#define PERIPHERALS_GPIO_MEMBRANE_5_3_PIN RPI_BPLUS_GPIO_J8_38
#define PERIPHERALS_GPIO_MEMBRANE_5_4_PIN RPI_BPLUS_GPIO_J8_36

#define PERIPHERALS_GPIO_MEMBRANE_8_0_PIN RPI_BPLUS_GPIO_J8_33
#define PERIPHERALS_GPIO_MEMBRANE_8_1_PIN RPI_BPLUS_GPIO_J8_31
#define PERIPHERALS_GPIO_MEMBRANE_8_2_PIN RPI_BPLUS_GPIO_J8_29
#define PERIPHERALS_GPIO_MEMBRANE_8_3_PIN RPI_BPLUS_GPIO_J8_32
#define PERIPHERALS_GPIO_MEMBRANE_8_4_PIN RPI_BPLUS_GPIO_J8_26
#define PERIPHERALS_GPIO_MEMBRANE_8_5_PIN RPI_BPLUS_GPIO_J8_22
#define PERIPHERALS_GPIO_MEMBRANE_8_6_PIN RPI_BPLUS_GPIO_J8_18
#define PERIPHERALS_GPIO_MEMBRANE_8_7_PIN RPI_BPLUS_GPIO_J8_16

#else
#error "GPIO membrane code for unknown platform"
#endif

#endif			/* #ifndef FUSE_PERIPHERALS_GPIO_MEMBRANE_PINS_H */
