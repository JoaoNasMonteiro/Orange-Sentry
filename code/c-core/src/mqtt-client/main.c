//  #include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../../include/arena.h"
#include "../../include/fifo-ipc.h"
#include "../../vendor/paho.mqtt.c/src/MQTTClient.h"

// Boilerplate stuff
#define ARENA_SIZE (64 * 1024)
#define LOG_FIFO_NAME "/tmp/test_fifo"
#define MODULE_NAME "MQTT_CLIENT"
#include "../../include/logging.h"

static uint8_t client_memory[ARENA_SIZE];

// MQTT Stuff
//  TODO also need to make it work by sending the messages it recieved from the
//  FIFO pipes
#define ADDRESS "tcp://localhost:1883"
#define CLIENTID "TestClient"
#define TOPIC "/test"
#define PAYLOAD "Hello World!"
#define QOS 1
#define TIMEOUT 10000L

// Function prototypes
int initialize_log_fifo(IPC_Channel *channel);
int mqtt_initialize(MQTTClient *client, const char *address,
                    const char *clientID);
int mqtt_pub_message(MQTTClient client, const char *topic, const char *payload);
int mqtt_close(MQTTClient client);

int main() {

  // initialize memory arena and named pipe
  Arena arena;
  arena_init(&arena, client_memory, ARENA_SIZE);

  IPC_Channel mqtt_log_channel;
  initialize_log_fifo(&mqtt_log_channel);

  // do mqtt stuff
  // client, connection options, message, delivery token, return code
  MQTTClient client;
  // fail fast
  if (mqtt_initialize(&client, ADDRESS, CLIENTID) != MQTTCLIENT_SUCCESS) {
    LOG_ERROR("MQTT initialization failed, exiting.");
    return -1;
  }
  if (mqtt_pub_message(client, TOPIC, PAYLOAD) != MQTTCLIENT_SUCCESS) {
    LOG_ERROR("MQTT publish message failed, exiting.");
    return -1;
  }
  if (mqtt_close(client) != 0) {
    LOG_ERROR("MQTT client close failed, exiting.");
    return -1;
  }

  // cleanup
  arena_reset(&arena);
  ipc_close_channel(&mqtt_log_channel);
  return 0;
}

int initialize_log_fifo(IPC_Channel *channel) {
  if (ipc_open_channel(channel, LOG_FIFO_NAME) != 0) {
    LOG_ERROR("Failed to open log FIFO at %s", LOG_FIFO_NAME);
    return -1;
  }
  return 0;
}

// need to create the client on main, then call this with the params
int mqtt_initialize(MQTTClient *client, const char *address,
                    const char *clientID) {
  int rc;
  rc = MQTTClient_create(client, address, clientID, MQTTCLIENT_PERSISTENCE_NONE,
                         NULL);
  if (rc != MQTTCLIENT_SUCCESS) {
    LOG_ERROR("Failed to create MQTT client. RC: %d", rc);
    return rc;
  }

  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  conn_opts.keepAliveInterval = 20;
  conn_opts.cleansession = 1;

  rc = MQTTClient_connect(*client, &conn_opts);
  if (rc != MQTTCLIENT_SUCCESS) {
    LOG_ERROR("Failed to connect to MQTT broker. RC: %d", rc);
    return rc;
  }

  return 0;
}

// need to call this with the client, topic, payload, qos, timeout
int mqtt_pub_message(MQTTClient client, const char *topic,
                     const char *payload) {

  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  MQTTClient_deliveryToken token = 0;

  pubmsg.payload = (char *)payload;
  pubmsg.payloadlen = strlen(payload);
  pubmsg.qos = QOS;
  pubmsg.retained = 0;

  int rc;
  rc = MQTTClient_publishMessage(client, topic, &pubmsg, &token);
  if (rc != MQTTCLIENT_SUCCESS) {
    LOG_ERROR("Failed to publish message. (token %d)RC: %d", token, rc);
    return rc;
  }
  printf("Waiting for up to %d seconds for publication of %s\non topic %s\n",
         (int)(TIMEOUT / 1000), payload, topic);

  rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
  if (rc != MQTTCLIENT_SUCCESS) {
    LOG_ERROR("Failed to publish message. (token %d)RC: %d", token, rc);
    return rc;
  }
  printf("Message with delivery token %d delivered\n", token);

  return 0;
}

int mqtt_close(MQTTClient client) {
  MQTTClient_disconnect(client, 10000);
  MQTTClient_destroy(&client);
  return 0;
}
