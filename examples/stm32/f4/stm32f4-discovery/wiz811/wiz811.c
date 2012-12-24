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
#include "serial.h"
#include "connection.h"

/* Set STM32 to 168 MHz. */
void clock_setup(void)
{
  rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_168MHZ]);
}

int main(void)
{
  // Setup routines
  clock_setup();
  leds_setup(LED_GREEN | LED_RED | LED_BLUE);
  serial_setup();

  printf("Welcome\r\n");

  if (connection_setup())
  {
    led_set(LED_GREEN);

    if (!udp_transmission())
    {
      led_set(LED_RED);
      printf("udp transmission failed\r\n");
    }
    else
    {
      led_set(LED_BLUE);

      u8 ints;

      if (!wiz811_read_reg(WIZ_IR, &ints))
	led_toggle(LED_GREEN);

      if (ints & WIZ_IR_CONFLICT)
      	led_set(LED_RED);

      if (ints & WIZ_IR_UNREACH)
	led_set(LED_RED);

      /* if (ints & WIZ_IR_S0INT) */
      /* 	led_set(LED_RED); */
    }
  }
  else
  {
    led_set(LED_RED);
    printf("connection setup failed\r\n");
  }

  return 0;
}
