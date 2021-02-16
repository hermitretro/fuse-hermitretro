/* hermitretro_controller.c: Hermit Retro Controller support
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

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <config.h>

#include "compat.h"
#include "infrastructure/startup_manager.h"
#include "input.h"
#include "keyboard.h"
#include "module.h"
#include "settings.h"

#ifdef BUILD_HERMITRETRO_ZXZERO

#include "gpio_common.h"
#include "gpio_membrane.h"
#include "gpio_joystick.h"
#include "hermitretro_controller.h"

//#define DEBUG 1
#undef DEBUG
#ifdef DEBUG
FILE *debugFile = NULL;
#endif
extern int ui_widget_level;

/** Controller debounce delay */
#define HERMITRETRO_CONTROLLER_DEBOUNCE_IN_MS 50L

static module_info_t hermitretro_controller_module_info = {

#ifdef PANTS
  /* .reset = */ NULL,
  /* .romcs = */ NULL,
  /* .snapshot_enabled = */ NULL,
  /* .snapshot_from = */ joystick_from_snapshot,
  /* .snapshot_to = */ joystick_to_snapshot,
#endif

};

/* Init/shutdown functions. Errors aren't important here */
int
hermitretro_controller_init( void *context )
{
  int rv = _hermitretro_controller_init();

  module_register( &hermitretro_controller_module_info );

  return rv;
}

void
hermitretro_controller_end( void )
{
}

void
hermitretro_controller_register_startup( void )
{
  startup_manager_module dependencies[] = {
    STARTUP_MANAGER_MODULE_LIBSPECTRUM,
    STARTUP_MANAGER_MODULE_SETUID
  };
  startup_manager_register( STARTUP_MANAGER_MODULE_HERMITRETRO_CONTROLLER, 
                            dependencies,
                            ARRAY_SIZE( dependencies ), 
                            hermitretro_controller_init,
                            hermitretro_controller_end, NULL );
}

/**
 * Initialise the I2C bus pins we need
 */
int
_hermitretro_controller_init( void )
{
  //----- OPEN THE I2C BUS -----
  printf("Setting up I2C\n");
  char *filename = (char*)"/dev/i2c-1";
  if ((file_i2c = open(filename, O_RDWR)) < 0)
  {
    printf("Failed to open the i2c bus");
    return 0;
  }


  return 0;
}

/**
 * Read the fuse menu button, the keyboard membrane and the joystick
 */
void
hermitretro_controller_poll( void )
{
#ifdef DEBUG
  fprintf( debugFile, "hermitretro_controller_poll(): widget_level: %d\n", ui_widget_level );
  fflush( debugFile );
#endif

  if ( debounceEvent( HERMITRETRO_ZXZERO_MENU_DEBOUNCE_IN_MS ) ) {
#ifdef DEBUG
    fprintf( debugFile, "hermitretro_controller: debounce0\n" );
    fflush( debugFile );
#endif
    return;
  }

  input_event_t fuse_event;

  /** Check to see whether we're pressing the "popup menu" button */
  if ( bcm2835_gpio_lev( HERMITRETRO_ZXZERO_FUSE_MENU_PIN ) == LOW ) {
#ifdef DEBUG
    fprintf( debugFile, "fuse menu pin == LOW\n" );
#endif
    /** Check extra debouncing so we don't just pop the menu away then back again */
    if ( debounceEvent( HERMITRETRO_ZXZERO_MENU_DEBOUNCE_IN_MS ) ) {
#ifdef DEBUG
        fprintf( debugFile, "debounce\n" );
        fflush( debugFile );
#endif
      return;
    }

#ifdef DEBUG
    fprintf( debugFile, "\nmenu button press: %d\n", ui_widget_level );
    fflush( debugFile );
#endif
    if ( ui_widget_level > -1 ) {
      fuse_event.types.key.native_key = INPUT_KEY_Escape;
      fuse_event.types.key.spectrum_key = INPUT_KEY_Escape;
    } else {
      fuse_event.types.key.native_key = INPUT_KEY_F1;
      fuse_event.types.key.spectrum_key = INPUT_KEY_F1;
    }
    fuse_event.type = INPUT_EVENT_KEYPRESS;

    /** 
     * Update the event time immediately before issuing the key.
     * Due to the calling of this poll method inside the ui/widget
     * code, it'll never debounce correctly and the menu will just
     * disappear immediately
     */
    updateLastEventTime();
    input_event( &fuse_event );

    return;
  }
}

#endif /** BUILD_HERMITRETRO_ZXZERO */
