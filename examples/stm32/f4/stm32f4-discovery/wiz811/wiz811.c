/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2011 Stephen Caudle <scaudle@doceme.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define DEBUG 1

#include <stdio.h>
#include <stdlib.h>

#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include "leds.h"
#include "temperature.h"
#include "serial.h"
#include "lis302dl.h"
#include "connection.h"

typedef enum
{
  ST_INIT,
  ST_UPDATE,
  ST_RELEASE,
  ST_ERROR,
  ST_DONE
} main_state;

main_state m_state;

struct packet_data
{
  // Configured G top value
  u8 fs_g;
  // Motion data
  u8 data[3];
  // Temperature
  float tchip;
  // Is this chunk new
  u8 isnew;
} m_data;

u16 cycles = 0;

/* Set STM32 to 168 MHz. */
void clock_setup(void)
{
  rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_168MHZ]);
}

u8 initialize(void)
{
  printf("Welcome\r\n");

  led_set(LED_GREEN);

  temperature_setup();

  lis_setup();

  if (!lis_basic_init())
    return 0;

  m_data.fs_g = lis_fs_g();

  m_data.isnew = 0;

  if (!connection_setup())
    return 0;

  m_state = ST_UPDATE;

  return 1;
}

u8 update(void)
{
  /* printf("Updating\r\n"); */
  led_set(LED_BLUE);
  led_clear(LED_GREEN);

  u8* dummy_data = NULL;
  if (!connection_run(dummy_data, 0))
    return 0;

#if 0
  if (lis_check_new_xyz())
  {
    lis_read_xyz(m_data.data);

    u8 data[8];
    data[0] = m_data.fs_g;
    memcpy(&data[1], m_data.data, 3);
    memcpy(&data[4], (void*)&m_data.tchip, sizeof(float));

    u8 rv = connection_run(data, 4);

    if (rv == 2)
      ++cycles;
    else if (!rv)
      return 0;
  }
#endif

  return 1;
}

u8 release(void)
{
  printf("Releasing\r\n");

  if (!lis_power_down())
    return 0;

  if (!connection_close())
    return 0;

  led_set(LED_GREEN);

  m_state = ST_DONE;

  return 1;
}

void error(void)
{
  printf("Error at %d\r\n", cycles);
  led_clear(LED_GREEN);
  led_clear(LED_BLUE);
  led_set(LED_RED);
}

void done(void)
{
  printf("Done\r\n");
  led_clear(LED_RED);
  led_set(LED_GREEN);
  led_set(LED_BLUE);
}

int main(void)
{
  // Setup routines
  clock_setup();
  leds_setup(LED_GREEN | LED_RED | LED_BLUE);
  serial_setup();

  m_state = ST_INIT;

  while (1)
  {
    switch (m_state)
    {
      case ST_INIT:
	if (!initialize())
	  m_state = ST_ERROR;
	break;
      case ST_UPDATE:
	if (!update())
	  m_state = ST_ERROR;
	break;
      case ST_RELEASE:
	if (!release())
	  m_state = ST_ERROR;
	break;
      case ST_ERROR:
	error();
	m_state = ST_DONE;
	break;
      default:
	break;
    }

    if (cycles > 2000 && (m_state == ST_UPDATE))
      m_state = ST_RELEASE;

    if (m_state == ST_DONE)
      return 0;
  }

  return 0;
}
