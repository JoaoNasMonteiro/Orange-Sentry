#include "fifo-ipc.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#define MODULE_NAME "fifo_ipc "
#include "logging.h"

// TODO implement pubbing capabilities

int ipc_open_channel(IPC_Channel *channel, const char *path) {
  channel->path = path;
  if (mkfifo(path, 0666) == -1) {

    // throw error if there is an actual issue
    if (errno != EEXIST) {
      LOG_ERROR("Failed to create FIFO named pipe at %s", path);
      return -1;
    }

    LOG_INFO("File Already Exists. Trying to open");
    chmod(path, 0666);
  }

  channel->fd = open(path, O_RDWR | O_NONBLOCK);
  if (channel->fd == -1) {
    LOG_ERROR("Fatal error opening FIFO %s. Quitting", path);
    return -1;
  }

  LOG_INFO("Channel opened successfully at %s", path);
  return 0;
}

size_t ipc_read_nonblocking(IPC_Channel *channel, char *buffer,
                            size_t max_size) {
  ssize_t bytes = read(channel->fd, buffer, max_size - 1);

  if (bytes > 0) {
    buffer[bytes] = '\0';
    return bytes;
  }

  if (bytes == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return 0;
    }

    LOG_ERROR("Failed to read bytes from stream");
    return 1;
  }

  return 0;
}

size_t ipc_write_nonblocking(IPC_Channel *channel, char *message,
                             size_t messageLen) {
  ssize_t bytes = write(channel->fd, message, messageLen);
  if (bytes == -1) {
    LOG_ERROR("Could not write to pipe %s", channel->path);
    return 1;
  }
  return bytes;
}

void ipc_close_channel(IPC_Channel *channel) {
  if (channel->fd != -1) {
    close(channel->fd);
  }
}
