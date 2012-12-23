#ifndef SERIAL_H
#define SERIAL_H

/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2011 Stephen Caudle <scaudle@doceme.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/usart.h>

#include <stdio.h>
#include <errno.h>

#define CHAR_LIMIT 30

void serial_setup(void)
{
#if DEBUG
  /* Enable GPIOD clock for USARTs. */
  rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPAEN);

  /* Enable clocks for USART2. */
  rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_USART2EN);

  /* Setup GPIO pins for USART2 transmit. */
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);

  /* Setup USART2 TX pin as alternate function. */
  gpio_set_af(GPIOA, GPIO_AF7, GPIO2);

  /* Setup USART2 parameters. */
  usart_set_baudrate(USART2, 115200);
  usart_set_databits(USART2, 8);
  usart_set_stopbits(USART2, USART_STOPBITS_1);
  usart_set_mode(USART2, USART_MODE_TX);
  usart_set_parity(USART2, USART_PARITY_NONE);
  usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

  /* Finally enable the USART. */
  usart_enable(USART2);
#endif
}

inline void serial_put(u8 value)
{
#if DEBUG
  usart_send_blocking(USART2, value);
#else
  (void)value;
#endif
}

void serial_send(char* str)
{
#if DEBUG
  u8 i = 0;

  while (((*str) != '\0') || (i < CHAR_LIMIT))
  {
    serial_put(*str++);
    ++i;
  }
#else
  (void)str;
#endif
}

// REPLACE ASAP
void serial_write_ip(u8* data)
{
  u8 i;

  for (i = 0; i < 4; ++i)
  {
    if (data[i] / 100)
    {
      serial_put((data[i] / 100) + '0');
      serial_put(((data[i] % 100) / 10) + '0');
      serial_put((data[i] % 10) + '0');
    }
    else
    {
      if ((data[i] % 100) / 10)
	serial_put(((data[i] % 100) / 10) + '0');

      if (data[i] % 10)
	serial_put((data[i] % 10) + '0');
      else
	serial_put('0');
    }

    if (i != 3)
      serial_put('.');
  }
}

int _write(int file, char *ptr, int len)
{
#if DEBUG
  if (file == 1)
  {
    u8 i;

    for (i = 0; i < len; ++i)
      serial_put((u8)*ptr++);

    return i;
  }

  errno = EIO;
  return -1;
#else
  (void)file;
  (void)ptr;
  (void)len;

  return 0;
#endif
}

#endif
