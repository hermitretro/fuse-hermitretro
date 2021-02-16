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
 * Initialise the MCP23008 pins
 */
int
_hermitretro_controller_init( void )
{
  /**
   * Open the I2C bus if needed. It might have been opened elsewhere
   * already
   */
  if ( i2cbus == -1 ) {
    char *i2cbusName = I2CBUSNAME;
    if (( i2cbus = open( i2cbusName, O_RDWR ) ) < 0 )
    {
      printf( "Failed to open the i2c bus" );
      return 1;
    }
  }

  /** Setup the default pins on the MCP23008 */
  int addr = 0x20;
  if (ioctl( i2cbus, I2C_SLAVE, addr ) < 0)
  {
    printf("Failed to acquire bus access and/or talk to slave.\n");
    return 1;
  }

  /** Set Port Expander B pins to inputs*/
  buffer[0] = 0x00; //IODIR
  buffer[1] = 0xFF; //Set all to input
  length = 2;                     //<<< Number of bytes to write
  if ( write( i2cbus, buffer, length) != length )
  {
    /* ERROR HANDLING: i2c transaction failed */
    printf("Failed to write to the i2c bus. length = %d\n",length);
    return 1;
  }

  return 0;
}

int readReg(unsigned char reg_addr, unsigned char *data){

    unsigned char *inbuff, outbuff;
    int retVal = -1;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    outbuff = reg_addr;
    messages[0].addr = 0x20;
    messages[0].flags= 0;
    messages[0].len = sizeof(outbuff);
    messages[0].buf = &outbuff;

    inbuff = data;
    messages[1].addr = 0x20;
    messages[1].flags = I2C_M_RD;
    messages[1].len = sizeof(*inbuff);
    messages[1].buf = inbuff;

    packets.msgs = messages;
    packets.nmsgs = 2;

    retVal = ioctl(file_i2c, I2C_RDWR, &packets);
    if ( retVal < 0 ) {
        perror("Read from I2C Device failed");
    }

    return retVal;
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

  int rv = readReg(0x09, buffer);
  printf( "ui_i2cbuttons_poll(): %d, %d\n", rv, buffer[0] );

  input_event_t fuse_event;
  fuse_event.type = INPUT_EVENT_JOYSTICK_PRESS;
  if ( ( buffer[0] & 0x01 ) == 0x01 ) {
    fuse_event.types.joystick.button = INPUT_JOYSTICK_LEFT;
    updateLastEventTime();
    input_event( &fuse_event );
  }
  if ( ( buffer[0] & 0x02 ) == 0x02 ) {
    fuse_event.types.joystick.button = INPUT_JOYSTICK_RIGHT;
    updateLastEventTime();
    input_event( &fuse_event );
  }
  if ( ( buffer[0] & 0x04 ) == 0x04 ) {
    fuse_event.types.joystick.button = INPUT_JOYSTICK_UP;
    updateLastEventTime();
    input_event( &fuse_event );
  }
  if ( ( buffer[0] & 0x08 ) == 0x08 ) {
    fuse_event.types.joystick.button = INPUT_JOYSTICK_DOWN;
    updateLastEventTime();
    input_event( &fuse_event );
  }
  if ( ( buffer[0] & 0x10 ) == 0x10 ) {
    fuse_event.types.joystick.button = INPUT_JOYSTICK_FIRE_3;
    updateLastEventTime();
    input_event( &fuse_event );
  }
  if ( ( buffer[0] & 0x20 ) == 0x20 ) {
    fuse_event.types.joystick.button = INPUT_JOYSTICK_FIRE_4;
    updateLastEventTime();
    input_event( &fuse_event );
  }
  if ( ( buffer[0] & 0x40 ) == 0x40 ) {
    fuse_event.types.joystick.button = INPUT_JOYSTICK_FIRE_1;
    updateLastEventTime();
    input_event( &fuse_event );
  }
  if ( ( buffer[0] & 0x80 ) == 0x80 ) {
    fuse_event.types.joystick.button = INPUT_JOYSTICK_FIRE_2;
    updateLastEventTime();
    input_event( &fuse_event );
  }
}

#endif /** BUILD_HERMITRETRO_ZXZERO */
