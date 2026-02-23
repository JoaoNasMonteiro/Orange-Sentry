// Global defines
#define MODULE_NAME "MQTT_CLIENT"

// standard includes
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

// shared includes
#include "../../include/arena.h"

// local includes
#include "mqtt.h"

// Boilerplate stuff
#define ARENA_SIZE (64 * 1024)
#define BUFFER_SIZE 512
static uint8_t client_memory[ARENA_SIZE];

#include "../../include/logging.h"

#define SOCK_IPC_IMPLEMENTATION
#include "../../include/sockclient.h"

// MQTT Stuff
// TODO make the program configurable via config file
#define ADDRESS "tcp://192.168.0.180:1883"
#define ADDRESS_DBG "tcp://127.0.0.1:1883"
#define CLIENTID "TestClient"
#define TOPIC "/test"
#define PAYLOAD "Hello World!"
#define QOS 1
#define TIMEOUT 10000

#define SOCK_PATH "/tmp/test_mqtt.sock"

volatile int keepRunning = 1;
void intHandler(int dummy) { keepRunning = 0; }

// Function prototypes
int get_payload_from_ipc_message(IPCMessage *msg, char *buffer,
                                 size_t maxBufferSize);

/* Fluxo de funcionamento:
 * inicializar arena -> inicializar fifo pipes -> inicializar e preencher
 * MQTT-> fazer handling de erros MQTT -> Entra loop de funcionamento: lê do
 * FIFO -> tem mensagem? se ssim envia para o destino via MQTT. Se não faz
 * nada (ou envia keepalive, ainda tenho que ver isso) mensagem chega ->
 * envia para o controlador via MQTT Sai do loop principal dar free nas
 * coisas do MQTT, dar free nas arenas, retornar a memória ao OS
 *
 * Este será um daemon que vai rodar no background
 * */

int main() {
  signal(SIGINT, intHandler);
  signal(SIGTERM, intHandler);

  // init arena
  Arena arena;
  arena_init(&arena, client_memory, ARENA_SIZE);

  // Attmept to connect to controller socket
  int sock_fd = ipc_client_connect(SOCK_PATH);
  if (sock_fd < 1) {
    LOG_INFO("Could not connect to socket. Quitting.");
    return -1;
  }

  // MQTT client initialization
  LOG_DEBUG("Starting MQTT Client");
  mqttContext *ctx =
      mqtt_create_context(ADDRESS_DBG, CLIENTID, 20, &arena, sock_fd);
  if (ctx == NULL) {
    LOG_ERROR("Failed to create MQTT client");
    return -1;
  }

  mqtt_subscribe(ctx, TOPIC, QOS);

  // Buffer allocation for reading from FIFO pipes
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
  memset(payload, 0, BUFFER_SIZE);

  IPCMessage rcv_msg;
  memset(&rcv_msg, 0, sizeof(IPCMessage));

  LOG_INFO("Entering main mqttd loop");
  while (keepRunning) {
    int rcv_status = ipc_client_receive(sock_fd, &rcv_msg);

    if (rcv_status > 0) {
      // success case, start treating the message

      if (rcv_msg.msgtype == MSG_CMD_MQTT_PUB) {
        memset(payload, 0, BUFFER_SIZE);
        if (get_payload_from_ipc_message(&rcv_msg, payload, BUFFER_SIZE) == 0) {
          if (mqtt_pub_message(ctx, TOPIC, payload) != 0) {
            LOG_ERROR("Failed to publish message");
          }
        } else {
          LOG_ERROR("Could not get payload from message");
        }
      }
    }

    else if (rcv_status < 0) {
      LOG_ERROR("IPC connection lost. Exiting loop");
      break;
    }

    safe_usleep(10000);
  }

  LOG_DEBUG("Shutting down MQTT Client");
  mqtt_disconnect_and_free(ctx);
  arena_reset(&arena);
  ipc_client_disconnect(&sock_fd);

  LOG_INFO("MQTT Client stopped");
  return 0;
}

int get_payload_from_ipc_message(IPCMessage *msg, char *buffer,
                                 size_t maxBufferSize) {
  if (msg == NULL) {
    LOG_ERROR("Invalid pointer to message");
    return -1;
  }

  if (msg->msgtype != MSG_CMD_MQTT_PUB) {
    LOG_ERROR("Message of wrong type. Is: %d; Should be: %d", msg->msgtype,
              MSG_CMD_MQTT_PUB);
    return -1;
  }

  size_t len_to_copy = msg->payload.mqtt_pub_cmd.data_len;

  if (len_to_copy >= maxBufferSize) {
    len_to_copy = maxBufferSize - 1;
  }

  memcpy(buffer, msg->payload.mqtt_pub_cmd.data, len_to_copy);
  buffer[len_to_copy] = '\0';

  return 0;
}
