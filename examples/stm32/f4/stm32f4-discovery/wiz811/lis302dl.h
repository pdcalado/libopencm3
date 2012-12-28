#ifndef LIS302DL_H
#define LIS302DL_H

#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/spi.h>

#define LIS_CLOCK RCC_APB2ENR_SPI1EN
#define LIS_SPI SPI1
#define LIS_CS_PORT GPIOE
#define LIS_CS_PIN GPIO3

#define LIS_WHOAMI 0x0F
#define LIS_ID 0x3B
#define LIS_CTRL1 0x20
#define LIS_CTRL2 0x21
#define LIS_CTRL3 0x22
#define LIS_HPFILTER 0x23
#define LIS_STATUS 0x27
#define LIS_OUTX 0x29
#define LIS_OUTY 0x2B
#define LIS_OUTZ 0x2D

// Control Register 1 masks
#define LIS_CTRL1_DR_100 (0 << 7)
#define LIS_CTRL1_DR_400 (1 << 7)
#define LIS_CTRL1_PWR_DOWN (0 << 6)
#define LIS_CTRL1_ACTIVE (1 << 6)
#define LIS_CTRL1_FS_2G (0 << 5)
#define LIS_CTRL1_FS_8G (1 << 5)
#define LIS_CTRL1_STP (1 << 4)
#define LIS_CTRL1_STM (1 << 3)
#define LIS_CTRL1_ZON (1 << 2)
#define LIS_CTRL1_ZOFF (0 << 2)
#define LIS_CTRL1_YON (1 << 1)
#define LIS_CTRL1_YOFF (0 << 1)
#define LIS_CTRL1_XON (1 << 0)
#define LIS_CTRL1_XOFF (0 << 0)

// Control Register 2 masks
#define LIS_CTRL2_BOOT (1 << 6)

// Status register masks
#define ZYXDA (1 << 3)
#define ZDA (1 << 2)
#define YDA (1 << 1)
#define XDA (1 << 0)

void lis_set_cs(void)
{
  gpio_set(LIS_CS_PORT, LIS_CS_PIN);
}

void lis_clear_cs(void)
{
  gpio_clear(LIS_CS_PORT, LIS_CS_PIN);
}

void lis_setup(void)
{
  /* Enable clock for peripheral SPI */
  rcc_peripheral_enable_clock(&RCC_APB2ENR, LIS_CLOCK);

  /* Same thing for GPIOE */
  rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPEEN);

  /* chip select */
  gpio_mode_setup(LIS_CS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LIS_CS_PIN);
  /* set to high which is not-selected */
  lis_set_cs();

  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,
		  /* serial clock */
		  GPIO5 |
		  /* master in/slave out */
		  GPIO6 |
		  /* master out/slave in */
		  GPIO7);
  gpio_set_af(GPIOA, GPIO_AF5, GPIO5 | GPIO6 | GPIO7);

  spi_disable_crc(LIS_SPI);
  spi_init_master(LIS_SPI, SPI_CR1_BAUDRATE_FPCLK_DIV_32,
		  /* high or low for the peripheral device */
		  SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
		  /* CPHA: Clock phase: read on rising edge of clock */
		  SPI_CR1_CPHA_CLK_TRANSITION_2,
		  /* DFF: Date frame format (8 or 16 bit) */
		  SPI_CR1_DFF_8BIT,
		  /* Most or Least Sig Bit First */
		  SPI_CR1_MSBFIRST);

  spi_enable_software_slave_management(LIS_SPI);
  spi_set_nss_high(LIS_SPI);

  /* spi_clear_mode_fault(LIS_SPI); */

  spi_enable(LIS_SPI);
}

u8 lis_read(u8 addr, u8* data)
{
  lis_clear_cs();

  u8 ignore;

  spi_send(LIS_SPI, addr | 0x80);

  spi_send(LIS_SPI, 0x00);

  ignore = spi_read(LIS_SPI);

  *data = spi_read(LIS_SPI);

  while (!(SPI_SR(LIS_SPI) & SPI_SR_TXE))
    ;

  while (SPI_SR(LIS_SPI) & SPI_SR_BSY)
    ;

  lis_set_cs();

  return ignore;
}

void lis_write(u8 addr, u8 data)
{
  lis_clear_cs();

  u8 ignore;

  spi_send(LIS_SPI, addr & 0x7F);

  spi_send(LIS_SPI, data);

  ignore = spi_read(LIS_SPI);

  ignore = spi_read(LIS_SPI);

  (void)ignore;

  while (!(SPI_SR(LIS_SPI) & SPI_SR_TXE))
    ;

  while (SPI_SR(LIS_SPI) & SPI_SR_BSY)
    ;

  lis_set_cs();
}

// Id check
u8 lis_whoami(void)
{
  u8 data;

  lis_read(LIS_WHOAMI, &data);

  if (data == LIS_ID)
    return 1;
  else
    return 0;
}

// Enable all axis
u8 lis_enable_xyz(void)
{
  u8 ctrl1;
  lis_read(LIS_CTRL1, &ctrl1);

  lis_write(LIS_CTRL1, ctrl1 | LIS_CTRL1_XON | LIS_CTRL1_YON | LIS_CTRL1_ZON);

  lis_read(LIS_CTRL1, &ctrl1);

  if ((ctrl1 & (LIS_CTRL1_XON | LIS_CTRL1_YON | LIS_CTRL1_ZON)) != (LIS_CTRL1_XON | LIS_CTRL1_YON | LIS_CTRL1_ZON))
    return 0;

  return 1;
}

// Check for data
u8 lis_check_new_xyz(void)
{
  u8 data;
  lis_read(LIS_STATUS, &data);

  if (data & (ZYXDA))
    return 1;
  else
    return 0;
}

void lis_out_x(u8* data)
{
  lis_read(LIS_OUTX, data);
}

void lis_out_y(u8* data)
{
  lis_read(LIS_OUTY, data);
}

void lis_out_z(u8* data)
{
  lis_read(LIS_OUTZ, data);
}

void lis_read_xyz(u8* data)
{
  lis_out_x(&data[0]);
  lis_out_y(&data[1]);
  lis_out_z(&data[2]);
}

u8 lis_power_up(void)
{
  u8 ctrl1;
  lis_read(LIS_CTRL1, &ctrl1);

  lis_write(LIS_CTRL1, ctrl1 | LIS_CTRL1_ACTIVE);

  lis_read(LIS_CTRL1, &ctrl1);

  if (ctrl1 & LIS_CTRL1_ACTIVE)
    return 1;

  return 0;
}

u8 lis_basic_init(void)
{
  if (!lis_whoami())
    return 0;

  if (!lis_enable_xyz())
    return 0;

  if (!lis_power_up())
    return 0;
  
  return 1;
}

u8 lis_power_down(void)
{
  u8 ctrl1;
  lis_read(LIS_CTRL1, &ctrl1);

  lis_write(LIS_CTRL1, ctrl1 & ~LIS_CTRL1_ACTIVE);

  lis_read(LIS_CTRL1, &ctrl1);

  if (!(ctrl1 & LIS_CTRL1_ACTIVE))
    return 1;

  return 0;
}

// Get configured G
u8 lis_fs_g(void)
{
  u8 ctrl;
  lis_read(LIS_CTRL1, &ctrl);

  if (ctrl & LIS_CTRL1_FS_8G)
    return 8;
  else
    return 2;
}

// Configure 2 or 8 g
u8 lis_configure_g(u8 gs)
{
  u8 ctrl1;
  lis_read(LIS_CTRL1, &ctrl1);

  if (gs == 2)
  {
    lis_write(LIS_CTRL1, ctrl1 & ~LIS_CTRL1_FS_8G);
    return 1;
  }
  else if (gs == 8)
  {
    lis_write(LIS_CTRL1, ctrl1 | LIS_CTRL1_FS_8G);
    return 1;
  }
  else
  {
    return 0;
  }
}

#endif
