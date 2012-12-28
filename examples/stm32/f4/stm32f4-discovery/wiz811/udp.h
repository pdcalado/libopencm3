#ifndef UDP_H
#define UDP_H

#include "socket.h"

typedef enum
{
  // Init state
  UDP_IDLE,
  // Transmitting
  UDP_TX
} udp_state_t;

// UDP socket
struct udp_socket
{
  // Socket id
  u8 id;
  // UDP socket state
  udp_state_t state;
  // Got data to send
  u8 to_send;
  // Size of data to receive
  u16 size_read;
  // Header of received message
  struct udp_header header;
};

typedef struct udp_socket* udp_socket_t;

// UDP socket initialization
struct udp_socket_init
{
  // Socket id
  u8 id;
  // Socket init struct
  socket_init_t sock;
};

typedef struct udp_socket_init* udp_socket_init_t;

// Setup udp socket
u8 udp_socket_setup(udp_socket_t udp, udp_socket_init_t uinit)
{
  udp->id = uinit->id;
  udp->state = UDP_IDLE;
  udp->to_send = 0;
  udp->size_read = 0;

  socket_init_t sinit;
  sinit = uinit->sock;

  // Setup socket
  return socket_setup(sinit);
}

// Open Socket (blocking)
u8 udp_open(udp_socket_t udp)
{
  if (!socket_open(udp->id))
    return 0;

  int status = -1;

  while (status != SOCKET_STATUS_UDP)
    if (!socket_get_status(udp->id, &status))
      return 0;

  return 1;
}

// Close socket (blocking)
u8 udp_close(udp_socket_t udp)
{
  if (!socket_close(udp->id))
    return 0;

  int status = -1;

  while (status != SOCKET_STATUS_CLOSED)
    if (!socket_get_status(0, &status))
      return 0;

  return 1;
}

int udp_write(udp_socket_t udp, u8* data, u16 size)
{
  if (udp->to_send)
    return -1;

  if (!socket_write(udp->id, data, size))
    return 0;

  udp->to_send = 1;

  return 1;
}

int udp_read(udp_socket_t udp, u8* data)
{
  if (!udp->size_read)
    return -1;

  // read procedure
  if (!socket_read(udp->id, &udp->header, data))
    return 0;

  if (!socket_recv(udp->id))
    return 0;

  udp->size_read -= 8 + udp->header.size;

  return 1;
}

int udp_update_tx(udp_socket_t udp)
{
  u8 ints;

  if (!socket_get_interrupts(0, &ints))
    return 0;

  if (ints & SOCKET_INT_SENDOK)
  {
    udp->state = UDP_IDLE;
    udp->to_send = 0;
    return 2;
  }

  if (ints & SOCKET_INT_TIMEOUT)
  {
    udp->state = UDP_IDLE;
    udp->to_send = 0;
    return -1;
  }

  return 1;
}

int udp_update(udp_socket_t udp)
{
  switch (udp->state)
  {
    case UDP_IDLE:
      if (!udp->size_read)
      {
	// check if we have received data
	u16 recv_size;
	if (!socket_recv_size(udp->id, &recv_size))
	  return 0;

	if (recv_size)
	  udp->size_read = recv_size;
      }

      // if not then goto tx if there's data to be sent
      if (udp->to_send)
      {
	udp->state = UDP_TX;

	if (!socket_send(0))
	  return 0;

	udp_update(udp);
      }
      break;
    case UDP_TX:
      return udp_update_tx(udp);
    default:
      break;
  }

  return 1;
}

#endif
