//  #include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include "../../include/arena.h"
#include "../../include/fifo-ipc.h"
#include "../../vendor/paho.mqtt.c/src/MQTTClient.h"

#define ARENA_SIZE (64 * 1024)
#define LOG_FIFO_NAME "/tmp/test_fifo"

static uint8_t client_memory[ARENA_SIZE];

int main() {

  // initialize memory arena and named pipe
  Arena arena;
  arena_init(&arena, client_memory, ARENA_SIZE);

  IPC_Channel mqtt_log_channel;
  if (ipc_open_channel(&mqtt_log_channel, LOG_FIFO_NAME) == 0) {
    perror("[Error] Failed to initialize FIFO");
    ipc_close_channel(&mqtt_log_channel);
    return 1;
  }

  MQTTClient client;

  // do mqtt stuff

  // close down named pipe and reset arena
  arena_reset(&arena);
  ipc_close_channel(&mqtt_log_channel);
  return 0;
}
