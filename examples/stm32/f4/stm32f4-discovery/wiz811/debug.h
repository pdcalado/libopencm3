//***************************************************************************
// Copyright (C) 2012 Laboratorio de Sistemas e Tecnologia Subaquatica      *
// Departamento de Engenharia Electrotecnica e de Computadores              *
// Rua Dr. Roberto Frias, 4200-465 Porto, Portugal                          *
//***************************************************************************
// Author: Ricardo Martins                                                  *
// Author: Pedro Calado (adapted to stm32f4 discovery)                      *
//***************************************************************************
// $Id::                                                                  $:*
//***************************************************************************
//                                                                          *
//***************************************************************************

#ifndef DEBUG_H_INCLUDED_
#define DEBUG_H_INCLUDED_

/* ISO C headers. */
#include <stdio.h>
#include <errno.h>

/* STM32 headers */
#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/usart.h>

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/**
 * printf() replacement.
 */
#if defined(DEBUG)
#  define debug(...) printf(__VA_ARGS__)
#else
#  define debug(...) do { } while(0)
#endif

/**
 * Print the name of the calling function to the debug channel.
 */
#if defined(DEBUG)
#  define debug_func() debug("\n*** %s ***\n", __FUNCTION__);
#else
#  define debug_func() do { } while(0)
#endif

#if defined(DEBUG)
/**
 * Output one character to the debug channel.
 * @param[in] byte byte to output.
 * @param[in] stream ignored.
 * @return 1.
 */
/* static inline int */
/* debug_put(char byte, FILE* stream) */
/* { */
/*   (void)stream; */
/*   usart_send_blocking(USART2, byte); */
/*   return 1; */
/* } */

/* static FILE debug_stdout = FDEV_SETUP_STREAM(debug_put, NULL, _FDEV_SETUP_WRITE); */

  int _write(int file, char *ptr, int len)
  {
    int i;

    if (file == 1)
    {
      for (i = 0; i < len; ++i)
	usart_send_blocking(USART2, ptr[i]);
      return i;
    }

    errno = EIO;
    return -1;
  }

#endif

/**
 * Initialize debug output channel. The baud rate is 57600.
 */
static inline void
debug_setup(void)
{
#if defined(DEBUG)
  /* Enable GPIOD clock for USARTs. */
  rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPAEN);

  /* Enable clocks for USART2. */
  rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_USART2EN);

  /* Setup GPIO pins for USART2 transmit. */
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);

  /* Setup USART2 TX pin as alternate function. */
  gpio_set_af(GPIOA, GPIO_AF7, GPIO2);

  /* Setup USART2 parameters. */
  usart_set_baudrate(USART2, 38400);
  usart_set_databits(USART2, 8);
  usart_set_stopbits(USART2, USART_STOPBITS_1);
  usart_set_mode(USART2, USART_MODE_TX);
  usart_set_parity(USART2, USART_PARITY_NONE);
  usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

  /* Finally enable the USART. */
  usart_enable(USART2);
#endif
}

#endif
