#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include  <libopencm3/stm32/f4/rcc.h>
#include  <libopencm3/stm32/f4/adc.h>

// Voltage Vsense at 25ºC
#define TS_V25 0.76f
// Average slope in Volt/ºC
#define TS_AVG_SLOPE 0.0025f
// Voltage supply
#define VSUPPLY 3.0f

/* Formula to compute actual chip temperature:
   (Vsense - V25) / AVG_SLOPE + 25 = T_A
*/

void temperature_setup(void)
{
  /* Enable clocks for internal temperature sensor */
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_ADC1EN);

  /* Set clock prescaling */
  adc_set_clk_prescale(ADC_CCR_ADCPRE_BY2);

  /* Single conversion mode */
  adc_set_single_conversion_mode(ADC1);

  /* Setting sample time */
  adc_set_sample_time(ADC1, ADC_CHANNEL16, ADC_SMPR1_SMP_28DOT5CYC);

  /* Define a regular sequence */
  u8 channels[] = {ADC_CHANNEL16};
  adc_set_regular_sequence(ADC1, 1, channels);

  /* Enable temperature sensor */
  adc_enable_temperature_sensor();

  /* Power on adc */
  adc_power_on(ADC1);
}

float temperature_read(void)
{
  adc_start_conversion_regular(ADC1);

  while (!adc_eoc(ADC1))
    ;

  u16 temp = adc_read_regular(ADC1);
  float v_sense = (float)temp / 0x0fff * VSUPPLY;

  return (v_sense - TS_V25) / TS_AVG_SLOPE + 25.0f;
}

#endif
