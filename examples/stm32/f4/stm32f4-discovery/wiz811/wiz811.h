#ifndef WIZ811_H
#define WIZ811_H

#include <ctype.h>

#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/spi.h>

#include <string.h>

#include "serial.h"

#define WIZ_SPI SPI2
#define WIZ_CLOCK RCC_APB1ENR_SPI2EN
#define WIZ_SPI_PORT GPIOB
#define WIZ_NSS GPIO12
#define WIZ_SCK GPIO13
#define WIZ_MISO GPIO14
#define WIZ_MOSI GPIO15
#define WIZ_RESET_PORT GPIOD
#define WIZ_RESET GPIO10

#define WIZ_GWR 0x0001
#define WIZ_SUBR 0x0005
#define WIZ_SHAR 0x0009
#define WIZ_SIPR 0x000F

// Mode register
#define WIZ_MR 0x0000
// S / W Reset
#define WIZ_MR_RST   (1 << 7)
// Ping block mode
#define WIZ_MR_PB    (1 << 4)
// Disable or enable PPPOE mode
#define WIZ_MR_PPOE  (1 << 3)
// Address Auto-Increment in Indirect Bus I/F
#define WIZ_MR_AI    (1 << 1)
// Indirect bus I/F mode
#define WIZ_MR_IND    (1 << 0)

// Interrupt register
#define WIZ_IR 0x0015
// Conflict
#define WIZ_IR_CONFLICT (1 << 7)
// Unreachable
#define WIZ_IR_UNREACH  (1 << 6)
// PPPOE connection closed
#define WIZ_IR_PPOE     (1 << 5)
// Socket interrupts
#define WIZ_IR_S3INT    (1 << 3)
#define WIZ_IR_S2INT    (1 << 2)
#define WIZ_IR_S1INT    (1 << 1)
#define WIZ_IR_S0INT    (1 << 0)

// Interrupt mask register
#define WIZ_IMR 0x0016
// IP Conflict Enable
#define WIZ_IMR_IR7 (1 << 7)
// Destination unreachable Enable
#define WIZ_IMR_IR6 (1 << 6)
// PPPoE Close Enable
#define WIZ_IMR_IR5 (1 << 5)
// 4 Should be zero  (1 << 4)
// Occurrence of Socket N Interrupt Enable
#define WIZ_IMR_IR3 (1 << 3)
#define WIZ_IMR_IR2 (1 << 2)
#define WIZ_IMR_IR1 (1 << 1)
#define WIZ_IMR_IR0 (1 << 0)

// Retry time value registers (RTR + 1)
// 1 means 100 us, default is 200ms (2000)
#define WIZ_RTR 0x0017

// Retry count register RCR
// If retransmission occurs more than the
// number recorded in RCR, Timeout Interrupt
// (TIMEOUT bit of Socket n Interrupt Register
// (Sn_IR) is set as ‘1’) will occur.
#define WIZ_RCR 0x0019

// RX Memory Size Register
// Default is 2K each socket and total usable (sum) is 8K
#define WIZ_RMSR 0x001A

// TX Memory Size Register
#define WIZ_TMSR 0x001B

// 2Kbytes each socket
#define WIZ_MEM_2K_EACH 0x55

// Authentication Type in PPPoE mode
// 0xC023 PAP
// 0xC223 CHAP
#define WIZ_PATR 0x001C

// PPP Link Control Protocol Request Timer Register
#define WIZ_PTIMER 0x0028

// PPP Link Control Protocol Magic number Register
#define WIZ_PMAGIC 0x0029

// Unreachable IP Address Register
#define WIZ_UIPR 0x002A

// Unreachable Port Register
#define WIZ_UPORT 0x002E

// Socket n Mode Register
#define WIZ_SNMR_0 0x0400
#define WIZ_SNMR_1 0x0500
#define WIZ_SNMR_2 0x0600
#define WIZ_SNMR_3 0x0700
// Socket mode masks
#define WIZ_SNMR_MULTI (1 << 7)
#define WIZ_SNMR_NDMC (1 << 5)
#define WIZ_SNMR_CLOSED 0x00
#define WIZ_SNMR_TCP 0x01
#define WIZ_SNMR_UDP 0x02
#define WIZ_SNMR_IPRAW 0x03
#define WIZ_SNMR_MACRAW 0x04
#define WIZ_SNMR_PPPOE 0x05

// Socket n Command Register
#define WIZ_SNCR_0 0x0401
#define WIZ_SNCR_1 0x0501
#define WIZ_SNCR_2 0x0601
#define WIZ_SNCR_3 0x0701
// Socket controls
#define WIZ_SNCR_OPEN 0x01
#define WIZ_SNCR_LISTEN 0x02
#define WIZ_SNCR_CONNECT 0x04
#define WIZ_SNCR_DISCON 0x08
#define WIZ_SNCR_CLOSE 0x10
#define WIZ_SNCR_SEND 0x20
#define WIZ_SNCR_SENDMAC 0x21
#define WIZ_SNCR_SENDKEEP 0x22
#define WIZ_SNCR_RECV 0x40

// Socket n Interrupt Register
// Clear by writing 1
#define WIZ_SNIR_0 0x0402
#define WIZ_SNIR_1 0x0502
#define WIZ_SNIR_2 0x0602
#define WIZ_SNIR_3 0x0702
// Masks
#define WIZ_SNIR_SENDOK (1 << 4)
#define WIZ_SNIR_TIMEOUT (1 << 3)
#define WIZ_SNIR_RECV (1 << 2)
#define WIZ_SNIR_DISCON (1 << 1)
#define WIZ_SNIR_CON (1 << 0)

// Socket n Status Register
#define WIZ_SNSR_0 0x0403
#define WIZ_SNSR_1 0x0503
#define WIZ_SNSR_2 0x0603
#define WIZ_SNSR_3 0x0703
// Masks
#define WIZ_SNSR_CLOSED 0x00
#define WIZ_SNSR_INIT 0x13
#define WIZ_SNSR_LISTEN 0x14
#define WIZ_SNSR_ESTAB 0x17
#define WIZ_SNSR_CLOSEWAIT 0x1C
#define WIZ_SNSR_UDP 0x22
#define WIZ_SNSR_IPRAW 0x32
#define WIZ_SNSR_MACRAW 0x42
#define WIZ_SNSR_PPPOE 0x5F
#define WIZ_SNSR_SYNSENT 0x15
#define WIZ_SNSR_SYNRECV 0x16
#define WIZ_SNSR_FINWAIT 0x18
#define WIZ_SNSR_CLOSING 0x1A
#define WIZ_SNSR_TIMEWAIT 0x1B
#define WIZ_SNSR_LASTACK 0x1D
#define WIZ_SNSR_ARP 0x11 // also 0x21 and 0x31

// Socket n Source Port Register
#define WIZ_SSPR_0 0x0404
#define WIZ_SSPR_1 0x0504
#define WIZ_SSPR_2 0x0604
#define WIZ_SSPR_3 0x0704

// Socket n Destination Hardware Address Register
#define WIZ_DHAR_0 0x0406
#define WIZ_DHAR_1 0x0506
#define WIZ_DHAR_2 0x0606
#define WIZ_DHAR_3 0x0706

// Socket n Destination IP Address Register
#define WIZ_DIPR_0 0x040C
#define WIZ_DIPR_1 0x050C
#define WIZ_DIPR_2 0x060C
#define WIZ_DIPR_3 0x070C

// Socket n Destination Port Register
#define WIZ_DPORT_0 0x0410
#define WIZ_DPORT_1 0x0510
#define WIZ_DPORT_2 0x0610
#define WIZ_DPORT_3 0x0710

// Socket n Maximum Segment Size Register
#define WIZ_MSS_0 0x0412
#define WIZ_MSS_1 0x0512
#define WIZ_MSS_2 0x0612
#define WIZ_MSS_3 0x0712

// Socket n IP Protocol Register
#define WIZ_PROTO_0 0x0414
#define WIZ_PROTO_1 0x0514
#define WIZ_PROTO_2 0x0614
#define WIZ_PROTO_3 0x0714

// Socket n IP Type Of Service Register
#define WIZ_TOS_0 0x0415
#define WIZ_TOS_1 0x0515
#define WIZ_TOS_2 0x0615
#define WIZ_TOS_3 0x0715

// Socket n IP Time To Live Register
#define WIZ_TTL_0 0x0416
#define WIZ_TTL_1 0x0516
#define WIZ_TTL_2 0x0616
#define WIZ_TTL_3 0x0716

// Socket n TX Free Size Register
#define WIZ_SNTXFSR_0 0x0420

// Socket n TX Read Pointer Register
#define WIZ_SNTXRR_0 0x0422

// Socket n TX Write Pointer Register
#define WIZ_SNTXWR_0 0x0424

// RX Received Size Register
#define WIZ_SNRXRSR_0 0x0426

// Socket n RX Read Pointer Register
#define WIZ_SNRXRD_0 0x0428

// Socket 0 base address
#define WIZ_S0_TX_BASE 0x4000;
#define WIZ_S0_RX_BASE 0x6000;

#define NUM_ATTEMPTS 10

// GPIO setup
/* PB12 -> SPI2_NSS */
/* PB13 -> SPI2_SCK */
/* PB14 -> SPI2_MISO */
/* PB15 -> SPI2_MOSI */
/* PD10 -> /RESET */

struct wiz_init
{
  u8 mr;
  u8 imr;
  u32 rtr;
  u8 rcr;
};

typedef struct wiz_init* wiz_init_t;

struct wiz_net_config
{
  char* gwr_addr;
  char* mac_addr;
  char* subnet_addr;
  char* source_addr;
};

typedef struct wiz_net_config* wiz_net_config_t;

// Will reset for a zero value
void wiz811_reset(u8 reset)
{
  if (reset)
    gpio_set(WIZ_RESET_PORT, WIZ_RESET);
  else
    gpio_clear(WIZ_RESET_PORT, WIZ_RESET);
}

void wiz811_setup(void)
{
  // GPIO SETUP
  /* Enable GPIOB clock */
  rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPBEN);

  /* Enable GPIOD clock */
  rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPDEN);

  /* Enable D10 as output since it will be connected to reset */
  gpio_mode_setup(WIZ_RESET_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, WIZ_RESET);

  /* All spi2 pins as alternate functions */
  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, WIZ_NSS | WIZ_SCK | WIZ_MISO | WIZ_MOSI);

  /* Set alternate function to SPI2 */
  gpio_set_af(GPIOB, GPIO_AF5, WIZ_NSS | WIZ_SCK | WIZ_MISO | WIZ_MOSI);

  // SPI SETUP
  /* Enable clock for peripheral SPI */
  rcc_peripheral_enable_clock(&RCC_APB1ENR, WIZ_CLOCK);

  spi_init_master(WIZ_SPI, SPI_CR1_BAUDRATE_FPCLK_DIV_32, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
		  SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);

  spi_enable_ss_output(WIZ_SPI);
  spi_set_nss_high(WIZ_SPI);

  // Set /RESET pin to high
  wiz811_reset(1);
}

u8 __wiz811_read(u16 addr, u8* data)
{
  u8 sync;

  spi_send(WIZ_SPI, 0x0F);

  spi_send(WIZ_SPI, addr >> 8);
  sync = spi_read(WIZ_SPI);

  spi_send(WIZ_SPI, addr);
  sync = spi_read(WIZ_SPI);

  spi_send(WIZ_SPI, 0x00);
  sync = spi_read(WIZ_SPI);

  *data = spi_read(WIZ_SPI);

  /* Wait for transfer finished. */
  while (!(SPI_SR(WIZ_SPI) & SPI_SR_TXE))
    ;

  while (SPI_SR(WIZ_SPI) & SPI_SR_BSY)
    ;

  if (sync == 0x02)
    return 1;
  else
    return 0;
}

u8 __wiz811_write(u16 addr, u8 value)
{
  u8 data;

  spi_send(WIZ_SPI, 0xF0);

  spi_send(WIZ_SPI, addr >> 8);
  data = spi_read(WIZ_SPI);

  spi_send(WIZ_SPI, addr);
  data = spi_read(WIZ_SPI);

  spi_send(WIZ_SPI, value);
  data = spi_read(WIZ_SPI);

  data = spi_read(WIZ_SPI);

  /* Wait for transfer finished. */
  while (!(SPI_SR(WIZ_SPI) & SPI_SR_TXE))
    ;

  while (SPI_SR(WIZ_SPI) & SPI_SR_BSY)
    ;

  if (data == 0x03)
    return 1;
  else
    return 0;
}

u8 wiz811_read_reg(u16 addr, u8* data)
{
  u8 res = 0;
  u8 i;

  for (i = 0; i < NUM_ATTEMPTS; i++)
  {
    spi_enable(WIZ_SPI);

    res = __wiz811_read(addr, data);

    spi_disable(WIZ_SPI);

    if (res)
      break;
  }

  if (!res)
    printf("failed to read\r\n");

  return res;
}

u8 wiz811_write_reg(u16 addr, u8 value)
{
  u8 res = 0;
  u8 i;

  for (i = 0; i < NUM_ATTEMPTS; i++)
  {
    spi_enable(WIZ_SPI);

    res = __wiz811_write(addr, value);

    spi_disable(WIZ_SPI);

    if (res)
      break;
  }

  if (!res)
    printf("failed to write\r\n");

  return res;
}

u8 wiz811_write_multiple_reg(u16 addr, u8* data, u16 size)
{
  u16 i;

  for (i = 0; i < size; ++i)
    if (!wiz811_write_reg(addr + i, data[i]))
      return 0;

  return 1;
}

u8 wiz811_read_multiple_reg(u16 addr, u8* data, u16 size)
{
  u16 i;

  for (i = 0; i < size; ++i)
    if (!wiz811_read_reg(addr + i, &data[i]))
      return 0;

  return 1;
}

void ip_to_bytes(char* ip_addr, u8* data)
{
  unsigned index = 0;

  u8 len = strlen(ip_addr);
  u8 i;

  for (i = 0; i < len; ++i)
  {
    if (isdigit((unsigned char)ip_addr[i]))
    {
      data[index] *= 10;
      data[index] += ip_addr[i] - '0';
    }
    else
    {
      index++;
    }
  }
}

void mac_to_bytes(char* mac_addr, u8* data)
{
  unsigned index = 0;

  u8 len = strlen(mac_addr);
  u8 i;

  for (i = 0; i < len; ++i)
  {
    char c = mac_addr[i];

    if (c >= '0' && c <= '9')
    {
      data[index] <<= 4;
      data[index] |= mac_addr[i] - '0';
    }
    else if (c >= 'a' && c <= 'f')
    {
      data[index] <<= 4;
      data[index] |= 10 + (u8)c - 'a';
    }
    else if (c >= 'A' && c <= 'F')
    {
      data[index] <<= 4;
      data[index] |= 10 + (u8)c - 'A';
    }
    else
    {
      index++;
    }
  }
}

// Write ip address to set of registers
inline u8 wiz811_write_ip_bytes(u16 start_reg, u8* data)
{
  return wiz811_write_multiple_reg(start_reg, data, 4);
}

// Read ip address from set of registers
inline u8 wiz811_read_ip_bytes(u16 start_reg, u8* data)
{
  return wiz811_read_multiple_reg(start_reg, data, 4);
}

// Write ip address as array of chars
u8 wiz811_write_ip(u16 start_reg, char* ip_addr)
{
  u8 data[4] = {0};

  ip_to_bytes(ip_addr, data);

  return wiz811_write_ip_bytes(start_reg, data);
}

// Write source hardware address (SHAR or MAC)
inline u8 wiz811_write_mac_bytes(u16 start_reg, u8* data)
{
  return wiz811_write_multiple_reg(start_reg, data, 6);
}

// Read mac address
inline void wiz811_read_mac_bytes(u16 start_reg, u8* data)
{
  wiz811_read_multiple_reg(start_reg, data, 6);
}

// Write mac address as array of chars
u8 wiz811_write_mac(u16 start_reg, char* mac_addr)
{
  u8 data[6] = {0};

  mac_to_bytes(mac_addr, data);

  return wiz811_write_mac_bytes(start_reg, data);
}

// Enable ping block
u8 wiz811_enable_pingblock(void)
{
  u8 data;

  if (!wiz811_read_reg(WIZ_MR, &data))
    return 0;

  return wiz811_write_reg(WIZ_MR, WIZ_MR_PB | data);
}

// Disable ping block
u8 wiz811_disable_pingblock(void)
{
  u8 data;

  if (!wiz811_read_reg(WIZ_MR, &data))
    return 0;

  return wiz811_write_reg(WIZ_MR, data & ~WIZ_MR_PB);
}

// Set retry time value RTR (in microseconds)
u8 wiz811_set_retry_time(u32 time)
{
  u16 value = time / 100;

  if (wiz811_write_reg(WIZ_RTR, value >> 8))
    if (wiz811_write_reg(WIZ_RTR + 1, value))
      return 1;

  return 0;
}

// Set retry count register
u8 wiz811_set_retry_count(u8 value)
{
  return wiz811_write_reg(WIZ_RCR, value);
}

// Basic settings
u8 wiz811_basic_init(wiz_init_t winit)
{
  if(!wiz811_write_reg(WIZ_MR, winit->mr))
    return 0;

  if (!wiz811_write_reg(WIZ_IMR, winit->imr))
    return 0;

  if (!wiz811_set_retry_time(winit->rtr))
    return 0;

  if (!wiz811_set_retry_count(winit->rcr))
    return 0;

  return 1;
}

// Set network information
u8 wiz811_network_config(wiz_net_config_t netconf)
{
  if (!wiz811_write_ip(WIZ_GWR, netconf->gwr_addr))
    return 0;

  if (!wiz811_write_ip(WIZ_SUBR, netconf->subnet_addr))
    return 0;

  if (!wiz811_write_ip(WIZ_SIPR, netconf->source_addr))
    return 0;

  if (!wiz811_write_mac(WIZ_SHAR, netconf->mac_addr))
    return 0;

  return 1;
}

// Set socket mode
inline u8 wiz811_set_socket_mode(u16 socket, u8 mode)
{
  return wiz811_write_reg(WIZ_SNMR_0 | (socket << 8), mode);
}

// Socket command
inline u8 wiz811_socket_command(u16 socket, u8 command)
{
  return wiz811_write_reg(WIZ_SNCR_0 | (socket << 8), command);
}

// Socket status
inline u8 wiz811_socket_status(u16 socket, u8* status)
{
  return wiz811_read_reg(WIZ_SNSR_0 | (socket << 8), status);
}

// Socket destination hardware address
inline u8 wiz811_socket_dest_mac(u16 socket, char* mac)
{
  return wiz811_write_mac(WIZ_DHAR_0 | (socket << 8), mac);
}

// Socket destination ip address
inline u8 wiz811_socket_dest_ip(u16 socket, char* ip)
{
  return wiz811_write_ip(WIZ_DIPR_0 | (socket << 8), ip);
}

// Set destination port
u8 wiz811_socket_dest_port(u16 socket, u16 port)
{
  if (wiz811_write_reg(WIZ_DPORT_0 | (socket << 8), port >> 8))
    return wiz811_write_reg((WIZ_DPORT_0 | (socket << 8)) + 1, port);

  return 0;
}

// Set source port
u8 wiz811_socket_source_port(u16 socket, u16 port)
{
  if (wiz811_write_reg(WIZ_SSPR_0 | (socket << 8), port >> 8))
    return wiz811_write_reg((WIZ_SSPR_0 | (socket << 8)) + 1, port);

  return 0;
}

// Get received size
u8 wiz811_socket_recv_size(u16 socket, u16* size)
{
  u8 temp;

  if (!wiz811_read_reg(WIZ_SNRXRSR_0 | (socket << 8), &temp))
    return 0;

  *size = (u16)temp << 8;

  if (!wiz811_read_reg((WIZ_SNRXRSR_0 | (socket << 8)) + 1, &temp))
    return 0;

  *size |= temp;

  return 1;
}

#endif
