/* hermitretro_zxzero.c: Hermit Retro ZXZero support
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
#include "peripherals/joystick.h"
#include "keyboard.h"
#include "module.h"
#include "ui/scaler/scaler.h"
#include "settings.h"
#include "utils.h"

#ifdef BUILD_HERMITRETRO_ZXZERO

#include "gpio_common.h"
#include "gpio_membrane.h"
#include "gpio_joystick.h"
#include "hermitretro_zxzero.h"

//#define DEBUG 1
#undef DEBUG
#ifdef DEBUG
FILE *debugFile = NULL;
#endif
extern int ui_widget_level;

/** Fuse menu button debounce delay */
#define HERMITRETRO_ZXZERO_MENU_DEBOUNCE_IN_MS 250L

static module_info_t hermitretro_zxzero_module_info = {
};

/* Init/shutdown functions. Errors aren't important here */
int
hermitretro_zxzero_init( void *context )
{
  int error;
  char *start_scaler;

  int rv = _hermitretro_zxzero_init();
  if ( rv != 0 ) {
    return rv;
  }

  module_register( &hermitretro_zxzero_module_info );

  /** Default to autosaving settings */
  settings_current.autosave_settings = 1;

  /** Default to having a Kempston joystick available and the default choice */
  settings_current.joy_kempston = 1;
  settings_current.joystick_1_output = JOYSTICK_TYPE_KEMPSTON;
  
  /** Disable the status bar */
  settings_current.statusbar = 0;

  /** Ensure full-screen as we don't want the SDL cursor */
  settings_current.full_screen = 1;

  /** Default to 2x (double-size) scaler as opposed to normal */
  /** Normal tends to smooth a bit */
  /** This code is just lifted from fuse.c */
  start_scaler = utils_safe_strdup( "2x" );

  error = scaler_select_id( start_scaler ); libspectrum_free( start_scaler );
  if ( error ) return error;

  return rv;
}

void
hermitretro_zxzero_end( void )
{
}

void
hermitretro_zxzero_register_startup( void )
{
  startup_manager_module dependencies[] = {
    STARTUP_MANAGER_MODULE_LIBSPECTRUM,
    STARTUP_MANAGER_MODULE_SETUID,
    STARTUP_MANAGER_MODULE_GPIO_COMMON
  };
  startup_manager_register( STARTUP_MANAGER_MODULE_HERMITRETRO_ZXZERO, 
                            dependencies,
                            ARRAY_SIZE( dependencies ), hermitretro_zxzero_init,
                            hermitretro_zxzero_end, NULL );
}

/**
 * Initialise all the pins we need
 */
int
_hermitretro_zxzero_init( void )
{
#ifdef DEBUG
    debugFile = fopen( "/tmp/fuse.txt", "wb" );
#endif

  if ( gpioInit == 0 ) {
    return 0;
  }

  /** Fuse menu button */
  bcm2835_gpio_fsel( HERMITRETRO_ZXZERO_FUSE_MENU_PIN, BCM2835_GPIO_FSEL_INPT );

  /** Zelux switch */
  /**
   * There is a pull resistor dictating the default state of this pin
   * Read the pin first, then set it as an output
   */
  bcm2835_gpio_fsel( HERMITRETRO_ZXZERO_ZELUX_PWR_PIN, BCM2835_GPIO_FSEL_INPT );
  zeluxState = bcm2835_gpio_lev( HERMITRETRO_ZXZERO_ZELUX_PWR_PIN );
  bcm2835_gpio_fsel( HERMITRETRO_ZXZERO_ZELUX_PWR_PIN, BCM2835_GPIO_FSEL_OUTP );

  return 0;
}

/**
 * Read the fuse menu button, the keyboard membrane and the joystick
 */
void
hermitretro_zxzero_poll( void )
{

  if ( gpioInit == 0 ) {
    return;
  }

  if ( debounceEvent( HERMITRETRO_ZXZERO_MENU_DEBOUNCE_IN_MS ) ) {
    return;
  }

#ifdef DEBUG2
  fprintf( debugFile, "kempston: %d\n", kempston_value );
  fflush( debugFile );
#endif

  /** Check to see whether we're pressing the "popup menu" button */
  if ( bcm2835_gpio_lev( HERMITRETRO_ZXZERO_FUSE_MENU_PIN ) == LOW ) {
#ifdef DEBUG
    fprintf( debugFile, "fuse menu pin == LOW\n" );
#endif

    input_event_t fuse_event;

    /** Check extra debouncing so we don't just pop the menu away then back again */
    if ( debounceEvent( HERMITRETRO_ZXZERO_MENU_DEBOUNCE_IN_MS ) ) {
      return;
    }

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

void
hermitretro_zxzero_toggleZelux( void )
{
  if ( gpioInit == 0 ) {
    return;
  }

  if ( zeluxState == 0 ) {
    zeluxState = 1;
    bcm2835_gpio_set( HERMITRETRO_ZXZERO_ZELUX_PWR_PIN );
  } else {
    zeluxState = 0;
    bcm2835_gpio_clr( HERMITRETRO_ZXZERO_ZELUX_PWR_PIN );
  }
}

#endif /** BUILD_HERMITRETRO_ZXZERO */
