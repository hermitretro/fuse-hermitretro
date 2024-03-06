/* gpio_joystick.c: GPIO joystick support
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

#ifdef BUILD_GPIO_JOYSTICK

#include "gpio_common.h"
#include "gpio_joystick.h"
#include "gpio_joystick_pins.h"

//#define DEBUG
#undef DEBUG
#ifdef DEBUG
extern FILE *debugFile;
#endif

extern int ui_widget_level;

/** Debouncing during menu operation */
#define GPIO_JOYSTICK_DEBOUNCE_IN_MS 50L

/** Pressed state to speed up unpresses... */
unsigned char joystickPressed[19] = { 0x00 };   /** 4 directions + 15 fire variants (see input.h) */

void pressJoystick( input_key key ) {
#ifdef DEBUG
  fprintf( debugFile, "PRESS JOYSTICK: %d\n", key );
  fflush( debugFile );
#endif

  joystickPressed[key - INPUT_JOYSTICK_UP] = 1;

  input_event_t fuse_event;
  fuse_event.type = INPUT_EVENT_JOYSTICK_PRESS;
  fuse_event.types.joystick.which = 0;
  fuse_event.types.joystick.button = key;

  /**
   * Update the last event time before issuing the
   * event to avoid ghost keypresses
   */
  updateLastEventTime();
  input_event( &fuse_event );
}

void unpressJoystick( input_key key ) {

#ifdef DEBUG
  fprintf( debugFile, "UNPRESS JOYSTICK: %d\n", key );
  fflush( debugFile );
#endif

  joystickPressed[key - INPUT_JOYSTICK_UP] = 0;

  input_event_t fuse_event;
  fuse_event.type = INPUT_EVENT_JOYSTICK_RELEASE;
  fuse_event.types.joystick.which = 0;
  fuse_event.types.joystick.button = key;
  input_event( &fuse_event );
}

static module_info_t gpio_joystick_module_info = {
};

/* Init/shutdown functions. Errors aren't important here */
int
gpio_joystick_init( void *context )
{
  /** gpio_common_init() should be been called before this */
  if ( gpioInit == 0 ) {
    return 0;
  }

  int rv = _gpio_joystick_init();

  module_register( &gpio_joystick_module_info );

  return rv;
}

void
gpio_joystick_end( void )
{
}

void
gpio_joystick_register_startup( void )
{
  startup_manager_module dependencies[] = {
    STARTUP_MANAGER_MODULE_LIBSPECTRUM,
    STARTUP_MANAGER_MODULE_SETUID,
    STARTUP_MANAGER_MODULE_GPIO_COMMON
  };
  startup_manager_register( STARTUP_MANAGER_MODULE_GPIO_JOYSTICK, dependencies,
                            ARRAY_SIZE( dependencies ), gpio_joystick_init,
                            gpio_joystick_end, NULL );
}

/**
 * Initialise all the pins we need
 */
int
_gpio_joystick_init( void )
{
    bcm2835_gpio_fsel( PERIPHERALS_GPIO_JOYSTICK_UP_PIN, BCM2835_GPIO_FSEL_INPT );
    bcm2835_gpio_fsel( PERIPHERALS_GPIO_JOYSTICK_DOWN_PIN, BCM2835_GPIO_FSEL_INPT );
    bcm2835_gpio_fsel( PERIPHERALS_GPIO_JOYSTICK_LEFT_PIN, BCM2835_GPIO_FSEL_INPT );
    bcm2835_gpio_fsel( PERIPHERALS_GPIO_JOYSTICK_RIGHT_PIN, BCM2835_GPIO_FSEL_INPT );
    bcm2835_gpio_fsel( PERIPHERALS_GPIO_JOYSTICK_FIRE1_PIN, BCM2835_GPIO_FSEL_INPT );
    bcm2835_gpio_fsel( PERIPHERALS_GPIO_JOYSTICK_FIRE2_PIN, BCM2835_GPIO_FSEL_INPT );

    bcm2835_gpio_set_pud( PERIPHERALS_GPIO_JOYSTICK_UP_PIN, BCM2835_GPIO_PUD_UP );
    bcm2835_gpio_set_pud( PERIPHERALS_GPIO_JOYSTICK_DOWN_PIN, BCM2835_GPIO_PUD_UP );
    bcm2835_gpio_set_pud( PERIPHERALS_GPIO_JOYSTICK_LEFT_PIN, BCM2835_GPIO_PUD_UP );
    bcm2835_gpio_set_pud( PERIPHERALS_GPIO_JOYSTICK_RIGHT_PIN, BCM2835_GPIO_PUD_UP );
    bcm2835_gpio_set_pud( PERIPHERALS_GPIO_JOYSTICK_FIRE1_PIN, BCM2835_GPIO_PUD_UP );
    bcm2835_gpio_set_pud( PERIPHERALS_GPIO_JOYSTICK_FIRE2_PIN, BCM2835_GPIO_PUD_UP );

  return 0;
}

/**
 * Read the fuse menu button, the keyboard membrane and the joystick
 */
void
gpio_joystick_poll( void )
{

  if ( gpioInit == 0 ) {
    return;
  }

  /** Make the joystick events less flighty when in widget mode */
  unsigned long long debounceInterval = GPIO_JOYSTICK_DEBOUNCE_IN_MS;
  if ( ui_widget_level > -1 ) {
    debounceInterval *= 3;
  }

  if ( debounceEvent( debounceInterval ) ) {
#ifdef DEBUG
    fprintf( debugFile, "gpio_joystick: debounce0\n" );
    fflush( debugFile );
#endif
    return;
  }

  /** Check the joystick pins */
  if ( bcm2835_gpio_lev( PERIPHERALS_GPIO_JOYSTICK_LEFT_PIN ) == LOW ) {
    pressJoystick( INPUT_JOYSTICK_LEFT );
  } else {
    if ( joystickPressed[INPUT_JOYSTICK_LEFT - INPUT_JOYSTICK_UP] ) {
        unpressJoystick( INPUT_JOYSTICK_LEFT );
    }
  }
  if ( bcm2835_gpio_lev( PERIPHERALS_GPIO_JOYSTICK_RIGHT_PIN ) == LOW ) {
    pressJoystick( INPUT_JOYSTICK_RIGHT );
  } else {
    if ( joystickPressed[INPUT_JOYSTICK_RIGHT - INPUT_JOYSTICK_UP] ) {
        unpressJoystick( INPUT_JOYSTICK_RIGHT );
    }
  }
  if ( bcm2835_gpio_lev( PERIPHERALS_GPIO_JOYSTICK_UP_PIN ) == LOW ) {
    pressJoystick( INPUT_JOYSTICK_UP );
  } else {
    if ( joystickPressed[INPUT_JOYSTICK_UP - INPUT_JOYSTICK_UP] ) {
        unpressJoystick( INPUT_JOYSTICK_UP );
    }
  }
  if ( bcm2835_gpio_lev( PERIPHERALS_GPIO_JOYSTICK_DOWN_PIN ) == LOW ) {
    pressJoystick( INPUT_JOYSTICK_DOWN );
  } else {
    if ( joystickPressed[INPUT_JOYSTICK_DOWN - INPUT_JOYSTICK_UP] ) {
        unpressJoystick( INPUT_JOYSTICK_DOWN );
    }
  }
  if ( bcm2835_gpio_lev( PERIPHERALS_GPIO_JOYSTICK_FIRE1_PIN ) == LOW ) {
    pressJoystick( INPUT_JOYSTICK_FIRE_1 );
  } else {
    if ( joystickPressed[INPUT_JOYSTICK_FIRE_1 - INPUT_JOYSTICK_UP] ) {
        unpressJoystick( INPUT_JOYSTICK_FIRE_1 );
    }
  }
  if ( bcm2835_gpio_lev( PERIPHERALS_GPIO_JOYSTICK_FIRE2_PIN ) == LOW ) {
    pressJoystick( INPUT_JOYSTICK_FIRE_2 );
  } else {
    if ( joystickPressed[INPUT_JOYSTICK_FIRE_2 - INPUT_JOYSTICK_UP] ) {
        unpressJoystick( INPUT_JOYSTICK_FIRE_2 );
    }
  }
}

#endif /** BUILD_GPIO_JOYSTICK */
