#ifndef CONNECTION_H
#define CONNECTIONH

#include "serial.h"
#include "wiz811.h"
#include "socket.h"

#define CONN_INTS WIZ_IMR_IR7 | WIZ_IMR_IR6 | WIZ_IMR_IR0

#define IP_ADDRESS "192.168.100.5"
#define GATEWAY_ADDRESS "192.168.100.2"
#define SUBNET_MASK "255.255.255.0"
#define MAC_ADDRESS "02.00.00.15.AC.68"

#define RETRY_TIME 300000 // in useconds
#define RETRY_COUNT 3

#define DESTINATION_MAC "00:26:18:14:ae:17"
#define DESTINATION_IP "192.168.100.2"
#define DESTINATION_PORT 6002
#define SOURCE_PORT 6002

#define RX_MEM WIZ_MEM_2K_EACH
#define TX_MEM WIZ_MEM_2K_EACH

#define SOCKET_MODE_0 WIZ_SNMR_UDP
#define SOCKET_MODE_1 WIZ_SNMR_CLOSED
#define SOCKET_MODE_2 WIZ_SNMR_CLOSED
#define SOCKET_MODE_3 WIZ_SNMR_CLOSED

u8 connection_setup(void)
{
  // Initialize wiz811
  wiz811_setup();

  u32 i;
  for (i = 0; i < 120000000; i++)
    __asm__("nop");

  serial_send("wiz setup done\r\n");

  // Configure
  struct wiz_init winit;
  winit.mr = 0x00;
  winit.imr = CONN_INTS;
  winit.rtr = RETRY_TIME;
  winit.rcr = RETRY_COUNT;

  if (!wiz811_basic_init(&winit))
    return 0;

  /* serial_send("wiz basic init done\r\n"); */

  // Configure network
  struct wiz_net_config wnetconf;

  char* gaddr = GATEWAY_ADDRESS;
  char* macaddr = MAC_ADDRESS;
  char* submask = SUBNET_MASK;
  char* ipaddr = IP_ADDRESS;

  wnetconf.gwr_addr = &gaddr[0];
  wnetconf.mac_addr = &macaddr[0];
  wnetconf.subnet_addr = &submask[0];
  wnetconf.source_addr = &ipaddr[0];

  if (!wiz811_network_config(&wnetconf))
    return 0;

  /* serial_send("network configured\r\n"); */

  // Configure sockets
  if (!wiz811_set_socket_mode(0, SOCKET_MODE_0) || !wiz811_set_socket_mode(1, SOCKET_MODE_1) ||
      !wiz811_set_socket_mode(2, SOCKET_MODE_2) || !wiz811_set_socket_mode(3, SOCKET_MODE_3))
    return 0;

  return 1;
}

// UDP transmission attempt
u8 udp_transmission(void)
{
  struct socket_init sock_init;
  sock_init.id = 0;

  char* dest_mac = DESTINATION_MAC;
  char* dest_ip = DESTINATION_IP;

  sock_init.dest_mac = &dest_mac[0];
  sock_init.dest_ip = &dest_ip[0];
  sock_init.dest_port = DESTINATION_PORT;
  sock_init.source_port = SOURCE_PORT;

  // Setup socket
  socket_setup(&sock_init);

#if 1

  // Open socket
  if (!socket_open(0))
    return 0;

  int status = -1;

  while (status != SOCKET_STATUS_UDP)
    if (!socket_get_status(0, &status))
      return 0;

  // Set remote ip and port
  if (!wiz811_socket_dest_ip(0, sock_init.dest_ip))
    return 0;

  if (!wiz811_socket_dest_port(0, sock_init.dest_port))
    return 0;

  if (!wiz811_socket_dest_mac(0, sock_init.dest_mac))
    return 0;

  // RECONFIGURE SOURCE IP AND MAC OR ELSE...
#if 1
  if (!wiz811_write_ip(WIZ_SIPR, IP_ADDRESS))
    return 0;

  if (!wiz811_write_mac(WIZ_SHAR, MAC_ADDRESS))
    return 0;
#endif

  u8 data[] = {6, 91, 10, 53, 63};

  if (!socket_write(0, data, 5))
    return 0;

  if (!socket_sendmac(0))
    return 0;

  u8 ints;

  while (1)
  {
    if (!socket_get_interrupts(0, &ints))
      return 0;

    if (ints & SOCKET_INT_SENDOK)
      break;

    if (ints & SOCKET_INT_TIMEOUT)
      return 0;
  }

  u16 txrr, txwr;
  socket_get_txwr(0, &txwr);
  socket_get_txrr(0, &txrr);

  if (txwr != txrr)
    return 0;

  if (!socket_close(0))
    return 0;

  while (status != SOCKET_STATUS_CLOSED)
    if (!socket_get_status(0, &status))
      return 0;

#endif

  return 1;
}

#endif
