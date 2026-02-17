#ifndef IPC_H
#define IPC_H

#include <stddef.h> // For size_t

// Opaque struct for the channel, hiding the fd
typedef struct {
  int fd;
  const char *path;
} IPC_Channel;

/**
 * Initializes the IPC Channel. Creates the FIFO if it didnt exist already.
 * Returns 0 on success, -1 on error.
 */
int ipc_open_channel(IPC_Channel *channel, const char *path);

/**
 * Tries to read a message from the channel without blocking.
 * Returns:
 * > 0: Quantity of bytes read (on success).
 *   0: No message available at the moment (not and error).
 *  -1: Real read error.
 */
size_t ipc_read_nonblocking(IPC_Channel *channel, char *buffer,
                            size_t max_size);

/**
 * Closes the channel.
 */
void ipc_close_channel(IPC_Channel *channel);

/**
 * Writes non-blocking a message on the channel.
 * returns:
 * >0: amount of bytes written
 * -1: Writing error
 */
size_t ipc_write_nonblocking(IPC_Channel *channel, char *message,
                             size_t messageLen);

#endif
