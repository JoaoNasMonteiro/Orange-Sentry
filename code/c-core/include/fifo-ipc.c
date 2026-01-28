#include "fifo-ipc.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int ipc_open_channel(IPC_Channel *channel, const char *path) {
  channel->path = path;
  if (mkfifo(path, 0666) == -1) {
    perror("[Error] Failed to create FIFO Named Pipe");
    return -1;
  }

  channel->fd = open(path, O_RDWR | O_NONBLOCK);
  if (channel->fd == -1) {
    perror("[Error] Failed to open FIFO named pipe");
    return -1;
  }

  return 0;
}

int ipc_read_nonblocking(IPC_Channel *channel, char *buffer, size_t max_size) {
  ssize_t bytes = read(channel->fd, buffer, max_size - 1);

  if (bytes > 0) {
    buffer[bytes] = '\n';
    return (int)bytes;
  }

  if (bytes == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return 0;
    }

    perror("[Error] Failed to read bytes from stream");
    return 0;
  }

  return 0;
}

void ipc_close_channel(IPC_Channel *channel) {
  if (channel->fd != -1) {
    close(channel->fd);
  }
}
