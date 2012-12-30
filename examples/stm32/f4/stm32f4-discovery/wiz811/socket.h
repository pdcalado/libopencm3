#ifndef SOCKET_H
#define SOCKET_H

#include "wiz811.h"

#include <stdio.h>
#include <string.h>

// Socket interrupt events
#define SOCKET_INT_SENDOK WIZ_SNIR_SENDOK
#define SOCKET_INT_TIMEOUT WIZ_SNIR_TIMEOUT
#define SOCKET_INT_RECV WIZ_SNIR_RECV
#define SOCKET_INT_DISCON WIZ_SNIR_DISCON
#define SOCKET_INT_CON WIZ_SNIR_CON

// Socket status
#define SOCKET_STATUS_CLOSED WIZ_SNSR_CLOSED
#define SOCKET_STATUS_INIT WIZ_SNSR_INIT
#define SOCKET_STATUS_LISTEN WIZ_SNSR_LISTEN
#define SOCKET_STATUS_ESTAB WIZ_SNSR_ESTAB
#define SOCKET_STATUS_CLOSEWAIT WIZ_SNSR_CLOSEWAIT
#define SOCKET_STATUS_UDP WIZ_SNSR_UDP
#define SOCKET_STATUS_IPRAW WIZ_SNSR_IPRAW
#define SOCKET_STATUS_MACRAW WIZ_SNSR_MACRAW
#define SOCKET_STATUS_PPPOE WIZ_SNSR_PPPOE
#define SOCKET_STATUS_SYNSENT WIZ_SNSR_SYNSENT
#define SOCKET_STATUS_SYNRECV WIZ_SNSR_SYNRECV
#define SOCKET_STATUS_FINWAIT WIZ_SNSR_FINWAIT
#define SOCKET_STATUS_CLOSING WIZ_SNSR_CLOSING
#define SOCKET_STATUS_TIMEWAIT WIZ_SNSR_TIMEWAIT
#define SOCKET_STATUS_LASTACK WIZ_SNSR_LASTACK
#define SOCKET_STATUS_ARP WIZ_SNSR_ARP

// Socket mem size
#define S_MEM_SIZE(_socket, _tmsr)		\
  (1024 << (((u16)_tmsr >> (_socket * 2)) & 0x0003))

struct socket_init
{
  u8 id;
  char* dest_mac;
  char* dest_ip;
  u16 dest_port;
  u16 source_port;
};

typedef struct socket_init* socket_init_t;

struct udp_header
{
  // Destinated ip address
  u8 ip[4];
  // Destinated port
  u16 port;
  // Data size
  u16 size;
};

typedef struct udp_header* udp_header_t;

inline u8 socket_open(u8 socket)
{
  return wiz811_socket_command(socket, WIZ_SNCR_OPEN);
}

inline u8 socket_close(u8 socket)
{
  return wiz811_socket_command(socket, WIZ_SNCR_CLOSE);
}

inline u8 socket_listen(u8 socket)
{
  return wiz811_socket_command(socket, WIZ_SNCR_LISTEN);
}

inline u8 socket_connect(u8 socket)
{
  return wiz811_socket_command(socket, WIZ_SNCR_CONNECT);
}

inline u8 socket_discon(u8 socket)
{
  return wiz811_socket_command(socket, WIZ_SNCR_DISCON);
}

inline u8 socket_send(u8 socket)
{
  return wiz811_socket_command(socket, WIZ_SNCR_SEND);
}

inline u8 socket_sendmac(u8 socket)
{
  return wiz811_socket_command(socket, WIZ_SNCR_SENDMAC);
}

inline u8 socket_sendkeep(u8 socket)
{
  return wiz811_socket_command(socket, WIZ_SNCR_SENDKEEP);
}

inline u8 socket_recv(u8 socket)
{
  return wiz811_socket_command(socket, WIZ_SNCR_RECV);
}

u8 socket_get_status(u8 socket, int* status)
{
  u8 u_status;

  if (!wiz811_socket_status(socket, &u_status))
    return 0;

  *status = (int)u_status;

  return 1;
}

u8 socket_get_free_size(u8 socket, u16* size)
{
  u8 temp;

  if (!wiz811_read_reg((WIZ_SNTXFSR_0 | (socket << 8)), &temp))
    return 0;

  *size = (u16)temp << 8;

  if (!wiz811_read_reg((WIZ_SNTXFSR_0 | (socket << 8)) + 1, &temp))
    return 0;

  *size |= temp;

  return 1;
}

u8 socket_get_txwr(u8 socket, u16* addr)
{
  u8 temp;

  if (!wiz811_read_reg((WIZ_SNTXWR_0 | (socket << 8)), &temp))
    return 0;

  *addr = (u16)temp << 8;

  if (!wiz811_read_reg((WIZ_SNTXWR_0 | (socket << 8)) + 1, &temp))
    return 0;

  *addr |= temp;

  return 1;
}

u8 socket_get_txrr(u8 socket, u16* addr)
{
  u8 temp;

  if (!wiz811_read_reg(WIZ_SNTXRR_0 + (socket << 8), &temp))
    return 0;

  *addr = (u16)temp << 8;

  if (!wiz811_read_reg(WIZ_SNTXRR_0 + (socket << 8) + 1, &temp))
    return 0;

  *addr |= temp;

  return 1;
}

u8 socket_set_txwr(u8 socket, u16 addr)
{
  if (!wiz811_write_reg(WIZ_SNTXWR_0 + (socket << 8), addr >> 8))
    return 0;

  if (!wiz811_write_reg(WIZ_SNTXWR_0 + (socket << 8) + 1, addr & 0xFF))
    return 0;

  return 1;
}

u8 socket_get_interrupts(u8 socket, u8* ints)
{
  return wiz811_read_reg(WIZ_SNIR_0 + (socket << 8), ints);
}

u8 socket_clear_interrupt(u8 socket, u8 ints)
{
  return wiz811_write_reg(WIZ_SNIR_0 + (socket << 8), ints);
}

u8 socket_setup(socket_init_t sinit)
{
  if (!wiz811_socket_dest_mac(sinit->id, sinit->dest_mac))
    return 0;

  if (!wiz811_socket_dest_ip(sinit->id, sinit->dest_ip))
    return 0;

  if (!wiz811_socket_dest_port(sinit->id, sinit->dest_port))
    return 0;

  if (!wiz811_socket_source_port(sinit->id, sinit->source_port))
    return 0;

  return 1;
}

u16 socket_compute_tx_base_address(u8 socket, u8 msr)
{
  u16 base = WIZ_S0_TX_BASE;
  u8 i;

  for (i = 0; i < socket; ++i)
    base += S_MEM_SIZE(socket, msr);

  return base;
}

u16 socket_compute_rx_base_address(u8 socket, u8 msr)
{
  u16 base = WIZ_S0_RX_BASE;
  u8 i;

  for (i = 0; i < socket; ++i)
    base += S_MEM_SIZE(socket, msr);

  return base;
}

u8 socket_write(u8 socket, u8* data, u16 size)
{
  u8 tmsr;

  if (!wiz811_read_reg(WIZ_TMSR, &tmsr))
    return 0;

  u16 mask = S_MEM_SIZE(socket, tmsr) - 1;
  u16 base = socket_compute_tx_base_address(socket, tmsr);

  u16 free_size;
  if (!socket_get_free_size(socket, &free_size))
    return 0;

  if (free_size < size)
    return 0;

  u16 txwr;
  if (!socket_get_txwr(socket, &txwr))
    return 0;

  u16 offset = txwr & mask;
  u16 start_address = base + offset;

  if (offset + size > mask + 1)
  {
    u16 upper_size = mask + 1 - offset;

    if (!wiz811_write_multiple_reg(start_address, data, upper_size))
      return 0;

    u16 size_left = size - upper_size;

    if (!wiz811_write_multiple_reg(base, data + upper_size, size_left))
      return 0;
  }
  else
  {
    if (!wiz811_write_multiple_reg(start_address, data, size))
      return 0;
  }

  if (!socket_set_txwr(socket, txwr + size))
    return 0;

  return 1;
}

u8 socket_recv_size(u8 socket, u16* size)
{
  return wiz811_socket_recv_size(socket, size);
}

u8 socket_get_rxrd(u8 socket, u16* rxrd)
{
  u8 temp;

  if (!wiz811_read_reg(WIZ_SNRXRD_0 + (socket << 8), &temp))
    return 0;

  *rxrd = (u16)temp << 8;

  if (!wiz811_read_reg(WIZ_SNRXRD_0 + (socket << 8) + 1, &temp))
    return 0;

  *rxrd |= temp;

  return 1;
}

u8 socket_set_rxrd(u8 socket, u16 addr)
{
  if (!wiz811_write_reg(WIZ_SNRXRD_0 + (socket << 8), addr >> 8))
    return 0;

  if (!wiz811_write_reg(WIZ_SNRXRD_0 + (socket << 8) + 1, addr & 0xFF))
    return 0;

  return 1;
}

int socket_read(u8 socket, udp_header_t header, u8** data)
{
  u8 rmsr;

  if (!wiz811_read_reg(WIZ_RMSR, &rmsr))
    return 0;

  u16 recv_size;
  if (!socket_recv_size(socket, &recv_size))
    return 0;

  u16 mask = S_MEM_SIZE(socket, rmsr) - 1;
  u16 base = socket_compute_rx_base_address(socket, rmsr);

  u16 rxrd;
  if (!socket_get_rxrd(socket, &rxrd))
    return 0;

  // compute offset address
  u16 offset = rxrd & mask;

  u16 start_address = base + offset;

  u8 header_size = 8;
  u8 header_addr[8];

  // if overflow
  if ((offset + header_size) > mask + 1)
  {
    u16 upper_size = mask + 1 - offset;

    if (!wiz811_read_multiple_reg(start_address, &header_addr[0], upper_size))
      return 0;

    u16 left_size = header_size - upper_size;
    if (!wiz811_read_multiple_reg(base, &header_addr[upper_size], left_size))
      return 0;

    offset = left_size;
  }
  else
  {
    if (!wiz811_read_multiple_reg(start_address, header_addr, header_size))
      return 0;

    offset += header_size;
  }

  // update start address
  start_address = base + offset;

  memcpy(header->ip, header_addr, 4);
  header->port = ((u16)header_addr[4] << 8) | header_addr[5];
  header->size = ((u16)header_addr[6] << 8) | header_addr[7];

  if (!header->size)
  {
    if (!socket_set_rxrd(socket, rxrd + header_size))
      return 0;

    return -1;
  }
  else if (header->size + header_size > recv_size)
  {
    if (!socket_set_rxrd(socket, rxrd + recv_size))
      return 0;

    return -1;
  }

  *data = malloc(sizeof(u8) * header->size);

  if (offset + header->size > mask + 1)
  {
    u16 upper_size = mask + 1 - offset;

    if (!wiz811_read_multiple_reg(start_address, *data, upper_size))
      return 0;

    u16 left_size = header->size - upper_size;
    if (!wiz811_read_multiple_reg(base, *data + upper_size, left_size))
      return 0;
  }
  else
  {
    if (!wiz811_read_multiple_reg(start_address, *data, header->size))
      return 0;
  }

  if (!socket_set_rxrd(socket, rxrd + (u16)header_size + header->size))
    return 0;

  return 1;
}

#endif
