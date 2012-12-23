#ifndef CONNECTION_H
#define CONNECTIONH

#include "serial.h"
#include "wiz811.h"
#include "socket.h"

#define CONN_INTS WIZ_IMR_IR7 | WIZ_IMR_IR6 | WIZ_IMR_IR0

#define IP_ADDRESS "192.168.100.5"
#define GATEWAY_ADDRESS "192.168.100.2"
#define SUBNET_MASK "255.255.255.0"
#define MAC_ADDRESS "A0.A0.A0.A0.A0.A0"

#define RETRY_TIME 300000 // in useconds
#define RETRY_COUNT 3

#define DESTINATION_MAC "00.00.00.00.00.00"
#define DESTINATION_IP "192.168.100.2"
#define DESTINATION_PORT 10
#define SOURCE_PORT 10

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

  serial_send("wiz setup done\r\n");

  // Configure
  struct wiz_init winit;
  winit.mr = 0x00;
  winit.imr = CONN_INTS;
  winit.rtr = RETRY_TIME;
  winit.rcr = RETRY_COUNT;

  if (!wiz811_basic_init(&winit))
    return 0;

  serial_send("wiz basic init done\r\n");

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

  serial_send("network configured\r\n");

  // Configure sockets
  if (!wiz811_set_socket_mode(0, WIZ_SNMR_UDP) || !wiz811_set_socket_mode(1, WIZ_SNMR_CLOSED) ||
      !wiz811_set_socket_mode(2, WIZ_SNMR_CLOSED) || !wiz811_set_socket_mode(3, WIZ_SNMR_CLOSED))
    return 0;

  return 1;
}

// UDP transmission attempt
u8 udp_transmission(void)
{
  struct socket_init sock_init;
  sock_init.id = 0;
  sock_init.dest_mac = DESTINATION_MAC;
  sock_init.dest_ip = DESTINATION_IP;
  sock_init.dest_port = DESTINATION_PORT;
  sock_init.source_port = SOURCE_PORT;

  // Setup socket
  socket_setup(&sock_init);

  // Open socket
  if (!socket_open(0))
    return 0;

  int status = -1;

  while (status != SOCKET_STATUS_UDP)
    if (!socket_get_status(0, &status))
      return 0;

  u16 size;

  if (!socket_get_free_size(0, &size))
    return 0;

  u16 w_addr;
  u16 r_addr;

  if (!socket_get_txwr(0, &w_addr))
    return 0;

  if (!socket_get_txrr(0, &r_addr))
    return 0;

  if (w_addr != r_addr)
    return 0;



  if (!socket_close(0))
    return 0;

  while (status != SOCKET_STATUS_CLOSED)
    if (!socket_get_status(0, &status))
      return 0;

  return 1;
}

#endif
