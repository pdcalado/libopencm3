#ifndef CONNECTION_H
#define CONNECTION_H

#include "serial.h"
#include "wiz811.h"
#include "udp.h"

#define CONN_INTS WIZ_IMR_IR7 | WIZ_IMR_IR6 | WIZ_IMR_IR0

#define IP_ADDRESS "192.168.100.5"
#define GATEWAY_ADDRESS "192.168.100.2"
#define SUBNET_MASK "255.255.255.0"
#define MAC_ADDRESS "02.00.00.15.AC.68"

#define RETRY_TIME 300000 // in useconds
#define RETRY_COUNT 3

#define DESTINATION_MAC "00:26:18:14:ae:17"
#define DESTINATION_IP "192.168.100.2"
#define DESTINATION_PORT 6009
#define SOURCE_PORT 6009

#define RX_MEM WIZ_MEM_2K_EACH
#define TX_MEM WIZ_MEM_2K_EACH

#define SOCKET_MODE_0 WIZ_SNMR_UDP
#define SOCKET_MODE_1 WIZ_SNMR_CLOSED
#define SOCKET_MODE_2 WIZ_SNMR_CLOSED
#define SOCKET_MODE_3 WIZ_SNMR_CLOSED

struct udp_socket udp;

u8 connection_setup(void)
{
  // Initialize wiz811
  wiz811_setup();

  u32 i;
  for (i = 0; i < 120000000; i++)
    __asm__("nop");

  printf("wiz setup done\r\n");

  // Configure
  struct wiz_init winit;
  winit.mr = 0x00;
  winit.imr = CONN_INTS;
  winit.rtr = RETRY_TIME;
  winit.rcr = RETRY_COUNT;

  if (!wiz811_basic_init(&winit))
    return 0;

  printf("wiz basic init done\r\n");

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

  printf("network configured\r\n");

  // Configure sockets
  if (!wiz811_set_socket_mode(0, SOCKET_MODE_0) || !wiz811_set_socket_mode(1, SOCKET_MODE_1) ||
      !wiz811_set_socket_mode(2, SOCKET_MODE_2) || !wiz811_set_socket_mode(3, SOCKET_MODE_3))
    return 0;

  struct udp_socket_init u_init;
  u_init.sock.id = 0;
  u_init.sock.dest_mac = DESTINATION_MAC;
  u_init.sock.dest_ip = DESTINATION_IP;
  u_init.sock.dest_port = DESTINATION_PORT;
  u_init.sock.source_port = SOURCE_PORT;

  if (!udp_socket_setup(&udp, &u_init))
    return 0;

  if (!udp_open(&udp))
    return 0;

  return 1;
}

// Connection update
u8 connection_run(u8* data, u8 size)
{
  if (!udp_update(&udp))
    return 0;

  if (!udp.to_send)
  {
    if (!udp_write(&udp, data, size))
      return 0;

    return 2;
  }

  return 1;
}

// Connection close
u8 connection_close(void)
{
  return udp_close(&udp);
}


#endif
