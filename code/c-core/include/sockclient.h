#ifndef SOCK_IPC_H
#define SOCK_IPC_H
// header
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "logging.h"

// enums
typedef enum { MOD_CORE = 0, MOD_MQTT, MOD_DISPLAY, MOD_HWINPUT } ModuleID;

typedef enum {
  // system / lifecycle
  MSG_SYS_ACK = 0,
  MSG_SYS_PING,
  MSG_SYS_PONG,

  // commands (controller -> module)
  // gen
  MSG_CMD_START,
  MSG_CMD_STOP,
  MSG_CMD_REQ_DATA,
  // mqtt
  MSG_CMD_MQTT_PUB,

  // event (module -> controller)
  // gen
  MSG_EVT_LOG,
  // mqtt
  MSG_EVT_MQTT_SUB_MSG,

  // errors
  MSG_ERR
} MSGType;

// structs
typedef struct {
  char topic[64];
  uint8_t qos;
  uint16_t data_len;
  uint8_t data[256];
} PayloadMQTTPubCMD;

typedef struct {
  char topic[64];
  uint16_t data_len;
  uint8_t data[256];
} PayloadMQTTSubEVT;

typedef struct {
  int32_t system_errno; // if 0 it's not a system error
  int32_t module_errno; // if 0 it's not a module error
  char message[256];
} PayloadError;

// master union for messages

typedef struct {
  ModuleID origin;
  MSGType msgtype;
  uint64_t timestamp_ms;
  size_t payload_len;
  union {
    PayloadMQTTPubCMD mqtt_pub_cmd;
    PayloadMQTTSubEVT mqtt_sub_evt;
    PayloadError rror;
    // add more payload types here
  } payload;
} IPCMessage;

// prototypes

int ipc_client_connect(const char *socket_path);
int ipc_client_send(int fd, const IPCMessage *msg);
int ipc_client_receive(int fd, IPCMessage *msg);
int ipc_client_disconnect(int *pfd);

#endif

// implementation (compile only once per program)
#ifdef SOCK_IPC_IMPLEMENTATION
// definitions
int ipc_client_connect(const char *socket_path) {
  int client_fd;
  struct sockaddr_un addr;

  client_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (client_fd == -1) {
    LOG_ERROR("Failed to create client socket fd: %s", strerror(errno));
    return -1;
  }

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

  LOG_INFO("Waiting for server to be available at %s...", socket_path);

  int retries = 50;
  while (retries > 0) {
    if (connect(client_fd, (struct sockaddr *)&addr,
                sizeof(struct sockaddr_un)) == 0) {
      LOG_INFO("Connected successfully to server at %s. fd: %d", socket_path,
               client_fd);
      return client_fd;
    }

    retries--;
    usleep(100000); // 100ms delay
  }

  LOG_ERROR("Failed to connect to server at %s after multiple attempts: %s",
            socket_path, strerror(errno));
  close(client_fd);
  return -1;
}

int ipc_client_send(int fd, const IPCMessage *msg) {
  if (fd < 0 || msg == NULL) {
    LOG_ERROR("Invalid arguments to ipc_client_send");
    return -1;
  }

  ssize_t bytes_sent = send(fd, msg, sizeof(IPCMessage), MSG_NOSIGNAL);
  if (bytes_sent == -1) {
    if (errno == EPIPE || errno == ECONNRESET) {
      LOG_ERROR("Connection to server lost while sending message: %s",
                strerror(errno));
    } else {
      LOG_ERROR("Failed to send message to server: %s", strerror(errno));
    }
    return -1;
  }

  LOG_DEBUG("Successfully sent message to server. Origin: %d, Type: %d",
            msg->origin, msg->msgtype);
  return (int)bytes_sent;
}

int ipc_client_receive(int fd, IPCMessage *msg) {
  if (fd < 0 || msg == NULL) {
    LOG_ERROR("Invalid arguments to ipc_client_receive");
    return -1;
  }

  ssize_t bytes_read = recv(fd, msg, sizeof(IPCMessage), MSG_DONTWAIT);
  if (bytes_read == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // No data available, not an error
      return 0;
    } else {
      LOG_ERROR("Failed to receive message from server: %s", strerror(errno));
      return -1;
    }
  } else if (bytes_read == 0) {
    LOG_ERROR("Server closed the connection");
    return -1;
  }

  LOG_DEBUG("Successfully received message from server. Origin: %d, Type: %d",
            msg->origin, msg->msgtype);
  return (int)bytes_read;
}

int ipc_client_disconnect(int *pfd) {
  if (pfd == NULL) {
    LOG_ERROR("Invalid argument to ipc_client_disconnect");
    return -1;
  }

  if (*pfd >= 0) {
    if (close(*pfd) == -1) {
      LOG_ERROR("Failed to close client socket fd: %s", strerror(errno));
      return -1;
    } else {
      LOG_INFO("Client socket fd %d closed successfully", *pfd);
    }
    *pfd = -1;
  } else {
    LOG_ERROR("Client fd is already invalid in ipc_client_disconnect");
    return -1;
  }
  return 0;
}

#endif
