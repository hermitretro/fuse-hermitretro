/* hermitretro_controller.h: Hermit Retro -- controller
   Copyright (c) 2021 Alligator Descartes <https://hermitretro.com>

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

#ifndef FUSE_PERIPHERALS_HERMITRETRO_CONTROLLER_H
#define FUSE_PERIPHERALS_HERMITRETRO_CONTROLLER_H

#include "bcm2835.h"

int hermitretro_controller_init( void *context );
void hermitretro_controller_end( void );
void hermitretro_controller_register_startup( void );

void hermitretro_controller_poll( void );

int _hermitretro_controller_init();

#endif			/* #ifndef FUSE_PERIPHERALS_HERMITRETRO_CONTROLLER_H */
