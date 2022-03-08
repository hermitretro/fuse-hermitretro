/* hermitretro_lyra.h: Hermit Retro ZXZero -- miscellaneous
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

#ifndef FUSE_PERIPHERALS_HERMITRETRO_LYRA_H
#define FUSE_PERIPHERALS_HERMITRETRO_LYRA_H

#define HERMITRETRO_LYRA_VERSION "1.0"
#define HERMITRETRO_LYRA_URL "https://hermitretro.com/lyra.html"

#define TYPE_MASK           0b11000000
#define B0_TYPE             0b00000000
#define B1_TYPE             0b01000000
#define VOLUME_UP_MASK      0b00000100
#define VOLUME_DOWN_MASK    0b00000010
#define BOTTOM_RIGHT_3_MASK 0b00000001
#define B2_TYPE             0b10000000
#define TOP_LEFT_MASK       0b00100000
#define LEFT_UP_MASK        0b00010000
#define LEFT_LEFT_MASK      0b00001000
#define LEFT_RIGHT_MASK     0b00000100
#define LEFT_DOWN_MASK      0b00000010
#define TOP_RIGHT_MASK      0b00000001
#define B3_TYPE             0b11000000
#define RIGHT_UP_MASK       0b00100000
#define RIGHT_LEFT_MASK     0b00010000
#define RIGHT_RIGHT_MASK    0b00001000
#define RIGHT_DOWN_MASK     0b00000100
#define BOTTOM_RIGHT_1_MASK 0b00000010
#define BOTTOM_RIGHT_2_MASK 0b00000001

#include "bcm2835.h"

#define HERMITRETRO_LYRA_MAX_VOLUME 40
extern int volumeLevel;    /** 0..100% */

int _hermitretro_lyra_init();

int hermitretro_lyra_init( void *context );
void hermitretro_lyra_end( void );
void hermitretro_lyra_register_startup( void );

void hermitretro_lyra_poll( void );

#endif			/* #ifndef FUSE_PERIPHERALS_HERMITRETRO_LYRA_H */
