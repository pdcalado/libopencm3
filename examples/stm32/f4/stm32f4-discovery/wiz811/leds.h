#ifndef LEDS_H
#define LEDS_H

#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>

#define LED_PORT GPIOD
#define LED_GREEN GPIO12
#define LED_ORANGE GPIO13
#define LED_RED GPIO14
#define LED_BLUE GPIO15

void leds_setup(u16 leds)
{
  /* Enable GPIOD clock */
  rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPDEN);

  /* Set LED's pins as outputs */
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, leds);
}

inline void led_set(u16 leds)
{
  gpio_set(LED_PORT, leds);
}

inline void led_clear(u16 leds)
{
  gpio_clear(LED_PORT, leds);
}

inline void led_toggle(u16 leds)
{
  gpio_toggle(LED_PORT, leds);
}

#endif
