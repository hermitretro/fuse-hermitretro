/* gpio_common.c: common GPIO functions
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

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <config.h>

#include "compat.h"
#include "infrastructure/startup_manager.h"
#include "input.h"
#include "keyboard.h"
#include "module.h"
#include "settings.h"

#if defined(BUILD_GPIO_MEMBRANE) || defined(BUILD_GPIO_JOYSTICK)

//#define DEBUG
#undef DEBUG
#ifdef DEBUG
extern FILE *debugFile;
#endif

#include "bcm2835.h"

#include "gpio_common.h"

/** Debouncing during menu operation */
static unsigned long long lastEventTime = 0;

static module_info_t gpio_common_module_info = {
};

/* Init/shutdown functions. Errors aren't important here */
int
gpio_common_init( void *context )
{
  bcm2835_init();

  updateLastEventTime();

  module_register( &gpio_common_module_info );

  return 0;
}

void
gpio_common_end( void )
{
}

void
gpio_common_register_startup( void )
{
  startup_manager_module dependencies[] = {
    STARTUP_MANAGER_MODULE_LIBSPECTRUM,
    STARTUP_MANAGER_MODULE_SETUID
  };
  startup_manager_register( STARTUP_MANAGER_MODULE_GPIO_COMMON, dependencies,
                            ARRAY_SIZE( dependencies ), gpio_common_init,
                            gpio_common_end, NULL );
}

/**
 * Update the timestamp of the last valid event. Debounced events should be
 * discarded by the caller
 */
void updateLastEventTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  lastEventTime =
    (unsigned long long)(tv.tv_sec) * 1000 +
    (unsigned long long)(tv.tv_usec) / 1000;
}

/**
 * Debounce the input event against the required interval.
 * Returns: 1 if the caller should discard the duplicate event
 *          0 if the caller should use the event
 */
int debounceEvent( unsigned long long debounceInterval ) {

  /** Because the calls to subsequent menus are nested, we need to also
   * track "key" repeats which are acceptable within the same UI level
   * but not across levels, i.e., the menus don't just collapse immediately
   * This code is pretty awful.
   */
  struct timeval tv;
  gettimeofday(&tv, NULL);
  unsigned long long t =
      (unsigned long long)(tv.tv_sec) * 1000 +
      (unsigned long long)(tv.tv_usec) / 1000;
  if ( (t - lastEventTime) < debounceInterval ) {
    return 1;
  }

  return 0;
}

#endif /** BUILD_GPIO_MEMBRANE || BUILD_GPIO_JOYSTICK */

