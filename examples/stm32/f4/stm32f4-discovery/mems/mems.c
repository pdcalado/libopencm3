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

#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/spi.h>

/* Set STM32 to 168 MHz. */
void clock_setup(void)
{
  rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_168MHZ]);
}

// SPI setup
// Setting up baud rate to be fpclk/256
// Mems sensor expects polarity to be 1
// Mems sensor expects phase to be 1 (strobe on second edge)
// data frame format might be 8 or 16 (start with 8)
// Most significant bit first

void spi_setup(void)
{
  /* Enable clock for peripheral SPI */
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_SPI1EN);

  /* use spi init master from libopencm3 */
  /* spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_4, SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE, */
  /* 		   SPI_CR1_CPHA_CLK_TRANSITION_2, SPI_CR1_DFF_8BIT, */
  /* 		   SPI_CR1_MSBFIRST); */

  spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_4, SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
		  SPI_CR1_CPHA_CLK_TRANSITION_2, SPI_CR1_DFF_8BIT,
		  SPI_CR1_MSBFIRST);

  spi_set_bidirectional_mode(SPI1); // do we need this?? I guess
  
  spi_enable_software_slave_management(SPI1);
  spi_set_nss_high(SPI1);
}

// GPIO setup
// PA5 -> SPC / SCK ... alternate function SP1_SCK
// PA6 -> SDO ... alternate function SP1_MISO
// PA7 -> SDA ... alternate function SP1_MOSI

// PE2 -> CS ... output PUP

void gpio_setup(void)
{
  /* Enable GPIOA clock */
  rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPAEN);

  /* Enable GPIOB clock */
  rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPBEN);

  /* Enable GPIOD clock */
  rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPDEN);

  /* Same thing for GPIOE */
  rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPEEN);

  /* Set GPIO2 (in GPIO port E) to 'output push-pull'. */
  /* Using API functions: */
  gpio_mode_setup(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO2 | GPIO3);

  /* Enable green and red led */
  gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO12 | GPIO13 | GPIO14 | GPIO15);

  /* GPIOs A5 and A7 must be outputs, GPIO A6 must be input */
  /* Setup GPIO pins for alternate function spi */
  /* gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5 | GPIO6 | GPIO7); */
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7);

  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO3);

  /* Setup GPIOA pins alternate function spi */
  /* gpio_set_af(GPIOA, GPIO_AF5, GPIO5 | GPIO6 | GPIO7); */
  gpio_set_af(GPIOA, GPIO_AF5, GPIO6 | GPIO7);
  gpio_set_af(GPIOB, GPIO_AF5, GPIO3);
}

void set_CS(bool flag)
{
  flag? gpio_set(GPIOE, GPIO3) : gpio_clear(GPIOE, GPIO3);
  flag? gpio_set(GPIOE, GPIO2) : gpio_clear(GPIOE, GPIO2);
}

// Read something from mems
// since the remaining two are protocol dependent
u8 read_mems(u16 data)
{
  // wait a bit then set chip select
  u16 i;
  for (i = 0; i < 1000; i++)	/* Wait a bit. */
    __asm__("nop");
  
  // set the gpioe2 (or 3!?) to low before transmitting
  set_CS(false);

  spi_send(SPI1, data);
  
  // write all zeros because we will be reading next
  // note nothing should have been received yet
  
  /* spi_write(SPI1,0x00); */
  
  // now read
  u8 recv;

  // this one actually does not matter (should be all zeros)
  recv = spi_read(SPI1);

  spi_send(SPI1,0x00);
  
  // this one does
  spi_read(SPI1);

  gpio_set(GPIOD, GPIO12);
  
  // DISABLE PROPERLY
  /* while (!(SPI_SR(SPI1) & SPI_SR_TXE)) */
  /*   ; */

  /* while (!(SPI_SR(SPI1) & SPI_SR_RXNE)) */
  /*   ; */

  /* while (SPI_SR(SPI1) & SPI_SR_BSY) */
  /*   ; */

  // not busy then we can quit
  u32 reg32;
  reg32 = SPI_CR1(SPI1);
  reg32 &= ~(SPI_CR1_SPE); /* Disable SPI. */
  SPI_CR1(SPI1) = reg32;

  // wait a bit then set chip select
  for (i = 0; i < 1000; i++)	/* Wait a bit. */
    __asm__("nop");

  set_CS(true);

  return recv;
}

int main(void)
{
  clock_setup();

  gpio_setup();

  set_CS(true);

  // wait a bit then setup spi
  u16 i;
  for (i = 0; i < 1000; i++)	/* Wait a bit. */
    __asm__("nop");
  
  spi_setup();

  spi_enable(SPI1);

  for (i = 0; i < 100; i++)	/* Wait a bit. */
    __asm__("nop");

  /* if (!(SPI_CR1(SPI1) & SPI_CR1_SPE)) */
  /*   gpio_set(GPIOD, GPIO14); */
  /* else */
  /*   gpio_set(GPIOD, GPIO12); */
  
  // buffer to send
  u8 buffer = 0b10001111;
  u8 data;
  data = read_mems(buffer);

  if (data == 0x3B)
    gpio_set(GPIOD, GPIO15);	/* LED on */
  else if (data == 0xFF)
    gpio_set(GPIOD, GPIO13);
  else if (data == 0x00)
    gpio_set(GPIOD, GPIO14);
  else
    gpio_set(GPIOD, GPIO13);

  return 0;
}
