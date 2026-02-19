// standard includes
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

// shared includes
#include "../../include/arena.h"
#include "../../include/fifo-ipc.h"

// local includes
#include "mqtt.h"

// Boilerplate stuff
#define ARENA_SIZE (64 * 1024)
#define BUFFER_SIZE 1024
#define LOG_FIFO_NAME "/tmp/test_fifo"
static uint8_t client_memory[ARENA_SIZE];

#define MODULE_NAME "MQTT_CLIENT"
#include "../../include/logging.h"

// MQTT Stuff
// TODO make the program configurable via config file
#define ADDRESS "tcp://192.168.0.180:1883"
#define CLIENTID "TestClient"
#define TOPIC "/test"
#define PAYLOAD "Hello World!"
#define QOS 1
#define TIMEOUT 10000

volatile int keepRunning = 1;
void intHandler(int dummy) { keepRunning = 0; }

// Function prototypes
int initialize_log_fifo(IPC_Channel *channel);

/* Fluxo de funcionamento:
 * inicializar arena -> inicializar fifo pipes -> inicializar e preencher MQTT->
 * fazer handling de erros MQTT -> Entra loop de funcionamento: lê do FIFO ->
 * tem mensagem? se ssim envia para o destino via MQTT. Se não faz nada (ou
 * envia keepalive, ainda tenho que ver isso) mensagem chega -> envia para o
 * controlador via MQTT Sai do loop principal dar free nas coisas do MQTT, dar
 * free nas arenas, retornar a memória ao OS
 *
 * Este será um daemon que vai rodar no background
 * */

int main() {
  signal(SIGINT, intHandler);
  signal(SIGTERM, intHandler);

  // init arena
  Arena arena;
  arena_init(&arena, client_memory, ARENA_SIZE);

  char *charbuffer = arena_alloc(&arena, BUFFER_SIZE);
  if (charbuffer == NULL) {
    LOG_ERROR("Failed to allocate memory from arena");
    return -1;
  }
  memset(charbuffer, 0, BUFFER_SIZE);

  char *payload = arena_alloc(&arena, BUFFER_SIZE);
  if (payload == NULL) {
    LOG_ERROR("Failed to allocate memory from arena");
    return -1;
  }
  strncpy(payload, PAYLOAD, BUFFER_SIZE - 1);

  // initialize log fifo
  IPC_Channel log_channel = {0};
  log_channel.fd = -1;
  if (initialize_log_fifo(&log_channel) != 0) {
    LOG_ERROR("Failed to initialize log FIFO pipe");
    return -1;
  }

  LOG_DEBUG("Starting to read from log FIFO pipe at %s", log_channel.path);
  ipc_read_nonblocking(&log_channel, charbuffer, BUFFER_SIZE);
  LOG_INFO("Log FIFO message: %s", charbuffer);

  LOG_DEBUG("Starting MQTT Client");

  mqttContext *ctx = mqtt_init(ADDRESS, CLIENTID, 20);

  if (ctx == NULL) {
    LOG_ERROR("Failed to initialize MQTT client");
    return -1;
  }

  ipc_read_nonblocking(&log_channel, NULL,
                       BUFFER_SIZE); // Clear any existing messages

  while (keepRunning) {
    // Try to resolve the message from the FIFO pipe
    int bytesRead =
        ipc_read_nonblocking(&log_channel, charbuffer, BUFFER_SIZE - 1);
    if ((bytesRead > 0) && (charbuffer[0] != '\0')) {
      charbuffer[bytesRead] = '\0'; // Null-terminate the string
      LOG_INFO("Read message from FIFO: %s", charbuffer);

      strncpy(payload, charbuffer, BUFFER_SIZE - 1);
      payload[bytesRead - 1] = '\0'; // Remove newline character

      LOG_INFO("Updated payload to: %s", payload);
      // change later to get a better handling of FIFO pipes
      memset(charbuffer, 0, BUFFER_SIZE);

    } else {
      LOG_INFO("Nothing to publish. Publishing Hello, World");
      strncpy(payload, PAYLOAD, BUFFER_SIZE - 1);
    }

    // actually try to publish the message
    if (mqtt_pub_message(ctx, TOPIC, payload) != 0) {
      LOG_ERROR("Failed to publish message");
    } else {
      LOG_INFO("Successfully published message to topic %s: %s", TOPIC,
               payload);
    }
    sleep(5);
  }

  LOG_DEBUG("Shutting down MQTT Client");
  mqtt_disconnect_and_free(ctx);
  arena_reset(&arena);
  ipc_close_channel(&log_channel);

  LOG_DEBUG("MQTT Client stopped");
  return 0;
}

int initialize_log_fifo(IPC_Channel *channel) {
  if (ipc_open_channel(channel, LOG_FIFO_NAME) != 0) {
    LOG_ERROR("Failed to open log FIFO at %s", LOG_FIFO_NAME);
    return -1;
  }
  return 0;
}

#include "mqtt.c"
