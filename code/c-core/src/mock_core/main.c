// This code was written totally by AI. It is not production code, and is for
// quick testing only

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// We include the header ONLY to get the IPCMessage struct definition.
// We DO NOT define SOCK_IPC_IMPLEMENTATION here because we aren't using
// the client functions. We are building the raw server side here.
#include "../../include/sockclient.h"

#define SOCK_PATH "/tmp/test_mqtt.sock"

int main() {
  int server_fd, client_fd;
  struct sockaddr_un addr;

  // 1. Create the server socket (SOCK_SEQPACKET guarantees message boundaries)
  server_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (server_fd == -1) {
    perror("Failed to create socket");
    return 1;
  }

  // Clean up any leftover socket file from previous crashes
  unlink(SOCK_PATH);

  // 2. Configure the address structure
  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

  // 3. Bind and Listen (This creates the actual .sock file on disk)
  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) ==
      -1) {
    perror("Failed to bind");
    return 1;
  }

  if (listen(server_fd, 5) == -1) {
    perror("Failed to listen");
    return 1;
  }

  printf("Fake Core is listening on %s...\n", SOCK_PATH);
  printf("Waiting for the MQTT daemon to connect...\n");

  // 4. Accept the connection (This blocks until your MQTT client turns on)
  client_fd = accept(server_fd, NULL, NULL);
  if (client_fd == -1) {
    perror("Failed to accept");
    return 1;
  }
  printf("MQTT Client connected successfully!\n");

  // Give the client a brief moment to fully initialize its Paho connection
  sleep(1);

  // 5. Build the IPC Message perfectly
  IPCMessage msg;
  memset(&msg, 0, sizeof(IPCMessage)); // Crucial: Prevent memory garbage

  msg.origin = MOD_CORE;
  msg.msgtype = MSG_CMD_MQTT_PUB;
  msg.payload_len = sizeof(PayloadMQTTPubCMD);

  // Fill the payload union
  const char *target_topic = "cyberdeck/teste";
  const char *secret_data = "HACK THE PLANET";

  strncpy(msg.payload.mqtt_pub_cmd.topic, target_topic,
          sizeof(msg.payload.mqtt_pub_cmd.topic) - 1);
  memcpy(msg.payload.mqtt_pub_cmd.data, secret_data, strlen(secret_data));
  msg.payload.mqtt_pub_cmd.data_len = strlen(secret_data);

  // 6. Send the raw struct bytes over the Unix Domain Socket
  printf("Sending MSG_CMD_MQTT_PUB:\n  Topic: '%s'\n  Data: '%s'\n",
         target_topic, secret_data);
  ssize_t bytes_sent = send(client_fd, &msg, sizeof(IPCMessage), 0);

  if (bytes_sent == -1) {
    perror("Failed to send message");
  } else {
    printf("Successfully injected %zd bytes into the socket.\n", bytes_sent);
  }

  printf("Entering listen loop. Blocking until message arrives from MQTT "
         "client...\n");

  IPCMessage recv_msg;
  // ALWAYS zero initialize your structs and buffers!
  memset(&recv_msg, 0, sizeof(recv_msg));

  char buffer[257]; // 256 bytes of data + 1 byte for the guaranteed '\0'
  memset(buffer, 0, sizeof(buffer));

  while (1) {
    // This will freeze here until the MQTT daemon sends something
    ssize_t bytes_read = recv(client_fd, &recv_msg, sizeof(recv_msg), 0);

    if (bytes_read > 0) {
      if (recv_msg.msgtype == MSG_EVT_MQTT_SUB_MSG) {
        printf("\n--- Received SUB Event from Client ---\n");
        printf("Topic: %s\n", recv_msg.payload.mqtt_sub_evt.topic);

        uint16_t cpylen = recv_msg.payload.mqtt_sub_evt.data_len;
        if (cpylen > 256)
          cpylen = 256; // Bounds check

        memcpy(buffer, recv_msg.payload.mqtt_sub_evt.data, cpylen);
        buffer[cpylen] = '\0'; // Guarantee string termination!

        printf("Payload: %s\n", buffer);
        printf("--------------------------------------\n");
        // break; // Exit after catching the first message
      } else {
        printf("Received a message, but it wasn't a SUB event (Type: %d)\n",
               recv_msg.msgtype);
      }
    } else if (bytes_read == 0) {
      printf("MQTT Client closed the connection.\n");
      // break;
    } else {
      perror("Socket recv error");
      // break;
    }
  }

  // 7. Clean teardown
  sleep(1); // Give it a moment to transmit before tearing down the socket
  close(client_fd);
  close(server_fd);
  unlink(SOCK_PATH);

  printf("Fake Core shutting down.\n");
  return 0;
}
