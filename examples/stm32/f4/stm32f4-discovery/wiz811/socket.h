#ifndef SOCKET_H
#define SOCKET_H

#include "wiz811.h"

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

struct socket_init
{
  u8 id;
  char* dest_mac;
  char* dest_ip;
  u16 dest_port;
  u16 source_port;
};

typedef struct socket_init* socket_init_t;

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

  if (!wiz811_read_reg(WIZ_SNTXFSR_0 + (socket << 8), &temp))
    return 0;

  *size = (u16)temp << 8;

  if (!wiz811_read_reg(WIZ_SNTXFSR_0 + (socket << 8) + 1, &temp))
    return 0;

  *size += temp;

  return 1;
}

u8 socket_get_txwr(u8 socket, u16* addr)
{
  u8 temp;

  if (!wiz811_read_reg(WIZ_SNTXWR_0 + (socket << 8), &temp))
    return 0;

  *addr = (u16)temp << 8;

  if (!wiz811_read_reg(WIZ_SNTXWR_0 + (socket << 8) + 1, &temp))
    return 0;

  *addr += temp;

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

  *addr += temp;

  return 1;
}

u8 socket_set_txwr(u8 socket, u16 addr)
{
  if (!wiz811_write_reg(WIZ_SNTXWR_0 + (socket << 8), addr << 8))
    return 0;

  if (!wiz811_write_reg(WIZ_SNTXWR_0 + (socket << 8) + 1, addr))
    return 0;

  return 1;
}

u8 socket_set_txrr(u8 socket, u16 addr)
{
  if (!wiz811_write_reg(WIZ_SNTXRR_0 + (socket << 8), addr << 8))
    return 0;

  if (!wiz811_write_reg(WIZ_SNTXRR_0 + (socket << 8) + 1, addr))
    return 0;

  return 1;
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

u8 socket_write(u8 socket, u8* data, u16 size)
{
  u16 free_size;
  if (!socket_get_free_size(socket, &free_size))
    return 0;

  if (free_size < size)
    return 0;

  u16 txwr;
  if (!socket_get_txwr(socket, &txwr))
    return 0;

  u16 offset = txwr & 0x07FF;
  u16 start_address = 0x4000 + offset;

  if (offset + size > 0x07FF + 1)
  {
    u16 upper_size = 0x07FF + 1 - offset;
    if (!wiz811_write_multiple_reg(start_address, data, upper_size))
      return 0;

    u16 size_left = size - upper_size;
    if (!wiz811_write_multiple_reg(start_address, data + upper_size, size_left))
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

#endif
