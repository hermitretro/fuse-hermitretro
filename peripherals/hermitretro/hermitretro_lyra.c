/* hermitretro_lyra.c: Hermit Retro Lyra support
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

#ifdef BUILD_HERMITRETRO_LYRA

#include "gpio_common.h"
#include "hermitretro_lyra.h"
#include "wiringSerial.h"

//#define DEBUG 1
#undef DEBUG
#ifdef DEBUG
FILE *debugFile = NULL;
#endif
extern int ui_widget_level;

/** Fuse menu button debounce delay */
#define HERMITRETRO_LYRA_DEBOUNCE_IN_MS 50L

/** Serial port */
int serialfd = -1;

#define TOP_LEFT_INDEX 0
#define LEFT_UP_INDEX 1
#define LEFT_LEFT_INDEX 2
#define LEFT_RIGHT_INDEX 3
#define LEFT_DOWN_INDEX 4
#define TOP_RIGHT_INDEX 5
#define RIGHT_UP_INDEX 6
#define RIGHT_LEFT_INDEX 7
#define RIGHT_RIGHT_INDEX 8
#define RIGHT_DOWN_INDEX 9
#define BUTTON_RIGHT_1_INDEX 10
#define BUTTON_RIGHT_2_INDEX 11
#define BUTTON_RIGHT_3_INDEX 12
#define VOLUME_UP_INDEX 13
#define VOLUME_DOWN_INDEX 14

uint8_t buttonsPressed[15] = { 0 };

static module_info_t hermitretro_lyra_module_info = {
};

/* Init/shutdown functions. Errors aren't important here */
int
hermitretro_lyra_init( void *context )
{
  int rv = _hermitretro_lyra_init();
  if ( rv != 0 ) {
    return rv;
  }

  module_register( &hermitretro_lyra_module_info );

  /** Default to autosaving settings */
  settings_current.autosave_settings = 1;

  /** Default to having a Kempston joystick available and the default choice */
  settings_current.joy_kempston = 1;
  settings_current.joystick_1_output = JOYSTICK_TYPE_KEMPSTON;
  
  /** Disable the status bar */
  settings_current.statusbar = 0;

  /** Ensure full-screen as we don't want the SDL cursor */
  settings_current.full_screen = 1;

  /**
   * Don't override the default scaler here. The display initialisation
   * order is very deferred meaning it'll either get overwritten or it'll
   * cause weird issues
   */

  return rv;
}

void
hermitretro_lyra_end( void )
{
  if ( serialfd >= 0 ) {
    serialClose( serialfd );
  }

  serialfd = -1;
}

void
hermitretro_lyra_register_startup( void )
{
  startup_manager_module dependencies[] = {
    STARTUP_MANAGER_MODULE_LIBSPECTRUM,
    STARTUP_MANAGER_MODULE_SETUID,
    STARTUP_MANAGER_MODULE_GPIO_COMMON,
    STARTUP_MANAGER_MODULE_DISPLAY
  };
  startup_manager_register( STARTUP_MANAGER_MODULE_HERMITRETRO_LYRA, 
                            dependencies,
                            ARRAY_SIZE( dependencies ), hermitretro_lyra_init,
                            hermitretro_lyra_end, NULL );
}

/**
 * Initialise what we need...
 */
int
_hermitretro_lyra_init( void )
{
#ifdef DEBUG
    debugFile = fopen( "/tmp/fuse.txt", "wb" );
#endif

  if ( gpioInit == 0 ) {
    printf( "Failed to init GPIO\n" );
    return -1;
  }

  /** Initialise the serial port which communicates with the ATMega32u4 */
  serialfd = serialOpen( "/dev/ttyACM0", 9600 );
  if ( serialfd < 0 ) {
    printf( "Failed to open serial port: %d\n", serialfd );
    return -1;
  }

  return 0;
}

/**
 * Press a button
 */
void
pressButton( uint8_t buttonIndex ) {

  buttonsPressed[buttonIndex] = 1;

  switch ( buttonIndex ) {
    case VOLUME_UP_INDEX: {
      volumeLevel += 10;
      if ( volumeLevel > HERMITRETRO_LYRA_MAX_VOLUME ) {
        volumeLevel = HERMITRETRO_LYRA_MAX_VOLUME;
      }
      /** Set ALSA volume */
      return;
    }
    case VOLUME_DOWN_INDEX: {
      volumeLevel -= 10;
      if ( volumeLevel < 0 ) {
        volumeLevel = 0;
      }
      /** Set ALSA volume */
      break;
    }
    case TOP_LEFT_INDEX: {
      /** Display the Fuse menu -- this requires some special handling to stop it being really jittery */
input_event_t fuse_event;

      /** Check extra debouncing so we don't just pop the menu away then back again */
      if ( debounceEvent( HERMITRETRO_LYRA_DEBOUNCE_IN_MS ) ) {
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
    default: {
      break;
    }
  }

  //input_event();
}

/** Button release */
void
releaseButton( uint8_t buttonIndex ) {

  if ( buttonsPressed[buttonIndex] ) {
    printf( "release: %d\n", buttonIndex );
  } else {
    //printf( "don't release: %d\n", buttonIndex );
  }

  buttonsPressed[buttonIndex] = 0;
  //input_event();
}

/**
 * Read the fuse menu button, the keyboard membrane and the joystick
 */
void
hermitretro_lyra_poll( void )
{
  int rv = -1;

  /** Make the keypresses less flighty when in widget mode */
  unsigned long long debounceInterval = HERMITRETRO_LYRA_DEBOUNCE_IN_MS; // settings_current.debounce_interval;
  if ( ui_widget_level > -1 ) {
    debounceInterval *= 5;
  }

  if ( debounceEvent( debounceInterval ) ) {
//    printf( "debouncing\n" );
    return;
  }

  /** Request a packet from the 32u4 */
  serialPutchar( serialfd, 0xAA );
  serialFlush( serialfd );
  bcm2835_delay( 50 );

  /** Read the serial port */
  /** @@FIXME: Loop for 75ms. The ATMega32u4 should have responded by now */
  rv = serialDataAvail( serialfd );
//  printf( "avail: %d\n", rv );
  if ( rv == 5 ) {
    uint8_t buf[5] = { 0x00 }; 
    buf[0] = serialGetchar( serialfd );
    buf[1] = serialGetchar( serialfd );
    buf[2] = serialGetchar( serialfd );
    buf[3] = serialGetchar( serialfd );
    buf[4] = serialGetchar( serialfd );
//    printf( "got packets: %X %X %X %X %X\n", buf[0], buf[1], buf[2], buf[3], buf[4] );

    /** Decode packets */
    uint8_t checksum = (buf[0] + buf[1] + buf[2] + buf[3]) ^ 0xFF;
    if ( checksum != buf[4] ) {
        printf( "bad checksum: %X != %X\n", checksum, buf[4] );
        goto bailout;
    } else {
        //printf( "checksums match\n" );
    }

    /**
     * Packet structure (this needs to match the hermitretro-lyra-32u4 firmware)
     * Byte0 = 00000000 | BATTERY (quantise to 6 bits) 
     * Byte1 = 01000000 | VOLUME_UP << 2 | VOLUME_DOWN << 1 | BOTTOM_RIGHT_3
     * Byte2 = 10000000 | TOP_LEFT << 5 | LEFT_UP << 4 | LEFT_LEFT << 3 | LEFT_RIGHT << 2 | LEFT_DOWN << 1 | TOP_RIGHT
     * Byte3 = 11000000 | RIGHT_UP << 5 | RIGHT_LEFT << 4 | RIGHT_RIGHT << 3 | RIGHT_DOWN << 2 | BOTTOM_RIGHT_1 << 1 | BOTTOM_RIGHT_2
     * Byte4 = checksum

     */
    if ( (buf[0] & TYPE_MASK) == B0_TYPE ) {
      //printf( "got B0\n" );
      /** Decode the battery level */
    }

    /** These are mutually exclusive buttons */
    if ( (buf[1] & TYPE_MASK) == B1_TYPE ) {
      //printf( "got B1: " );
      if ( (buf[1] & VOLUME_UP_MASK) == VOLUME_UP_MASK ) {
        printf( "VOLUP\n" );
        goto bailout;
      }

      if ( (buf[1] & VOLUME_DOWN_MASK) == VOLUME_DOWN_MASK ) {
        printf( "VOLDOWN\n" );
        pressButton( VOLUME_DOWN_INDEX );
        goto bailout;
      } else {
        //printf( "VOLDOWN RELEASE\n" );
        releaseButton( VOLUME_DOWN_INDEX );
      }

      if ( (buf[1] & BOTTOM_RIGHT_3_MASK) == BOTTOM_RIGHT_3_MASK ) {
        printf( "BOTTOM_RIGHT_3\n" );
        goto bailout;
      }
    }

    /** These are generally mixable buttons, i.e., diagonals */
    if ( (buf[2] & TYPE_MASK) == B2_TYPE ) {
      //printf( "got B2: " );
      /** The TOP_LEFT and TOP_RIGHT are not mixable */
      if ( (buf[2] & TOP_LEFT_MASK) == TOP_LEFT_MASK ) {
        printf( "TOP_LEFT\n" );
        pressButton( TOP_LEFT_INDEX );
        goto bailout;
      } else {
        releaseButton( TOP_LEFT_INDEX );
      }

      if ( (buf[2] & TOP_RIGHT_MASK) == TOP_RIGHT_MASK ) {
        printf( "TOP_RIGHT\n" );
        goto bailout;
      }

      /** The remaining dpad buttons are mixable */
      if ( (buf[2] & LEFT_UP_MASK) == LEFT_UP_MASK ) {
        printf( "LEFT_UP\n" );
      }
      if ( (buf[2] & LEFT_LEFT_MASK) == LEFT_LEFT_MASK ) {
        printf( "LEFT_LEFT\n" );
      }
      if ( (buf[2] & LEFT_RIGHT_MASK) == LEFT_RIGHT_MASK ) {
        printf( "LEFT_RIGHT\n" );
      }
      if ( (buf[2] & LEFT_DOWN_MASK) == LEFT_DOWN_MASK ) {
        printf( "LEFT_DOWN\n" );
      }
    }

    /** These are generally mixable buttons, i.e., diagonals */
    if ( (buf[3] & TYPE_MASK) == B3_TYPE ) {
      //printf( "got B3: " );

      /** The BOTTOM_RIGHT_1 and BOTTOM_RIGHT_2 are not mixable */
      if ( (buf[3] & BOTTOM_RIGHT_1_MASK) == BOTTOM_RIGHT_1_MASK ) {
        printf( "BOTTOM_RIGHT_1\n" );
        goto bailout;
      }

      if ( (buf[3] & BOTTOM_RIGHT_2_MASK) == BOTTOM_RIGHT_2_MASK ) {
        printf( "BOTTOM_RIGHT_2\n" );
        goto bailout;
      }

      /** The remaining dpad buttons are mixable */
      if ( (buf[3] & RIGHT_UP_MASK) == RIGHT_UP_MASK ) {
        printf( "RIGHT_UP\n" );
      }
      if ( (buf[3] & RIGHT_LEFT_MASK) == RIGHT_LEFT_MASK ) {
        printf( "RIGHT_LEFT\n" );
      }
      if ( (buf[3] & RIGHT_RIGHT_MASK) == RIGHT_RIGHT_MASK ) {
        printf( "RIGHT_RIGHT\n" );
      }
      if ( (buf[3] & RIGHT_DOWN_MASK) == RIGHT_DOWN_MASK ) {
        printf( "RIGHT_DOWN\n" );
      }
    }
  }

#ifdef PANTS
  /** Check to see whether we're pressing the "popup menu" button */
  if ( bcm2835_gpio_lev( HERMITRETRO_LYRA_FUSE_MENU_PIN ) == LOW ) {
#ifdef DEBUG
    fprintf( debugFile, "fuse menu pin == LOW\n" );
#endif

    input_event_t fuse_event;

    /** Check extra debouncing so we don't just pop the menu away then back again */
    if ( debounceEvent( HERMITRETRO_LYRA_MENU_DEBOUNCE_IN_MS ) ) {
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
  }
#endif

bailout:
  /** 
   * Update the event time immediately before issuing the key.
   * Due to the calling of this poll method inside the ui/widget
   * code, it'll never debounce correctly and the menu will just
   * disappear immediately
   */
  updateLastEventTime();
//  input_event( &fuse_event );

  return;
}

#endif /** BUILD_HERMITRETRO_LYRA */
