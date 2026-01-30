#include "fifo-ipc.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#define MODULE_NAME "fifo_ipc "
#include "logging.h"

int ipc_open_channel(IPC_Channel *channel, const char *path) {
  channel->path = path;
  if (mkfifo(path, 0666) == -1) {

    // throw error if there is an actual issue
    if (errno != EEXIST) {
      LOG_ERROR("Failed to create FIFO named pipe at %s", path);
      return -1;
    }

    // if it gets here then the file already existed, so we just update the
    chmod(path, 0666);
  }

  channel->fd = open(path, O_RDWR | O_NONBLOCK);
  if (channel->fd == -1) {
    LOG_ERROR("Failed to open FIFO named pipe at %s", path);
    return -1;
  }

  // TODO flush old pipe data
  LOG_INFO("Channel opened successfully at %s", path);
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

    LOG_ERROR("Failed to read bytes from stream");
    return 0;
  }

  return 0;
}

void ipc_close_channel(IPC_Channel *channel) {
  if (channel->fd != -1) {
    close(channel->fd);
  }
}
