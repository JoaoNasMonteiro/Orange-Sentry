#include <stdio.h>
#include <stdlib.h>

#include "../../include/arena.h"
#include "../../vendor/paho.mqtt.c/src/MQTTClient.h"
#include "mqtt.h"

#include "../../include/fifo-ipc.h"
#include "../../include/logging.h"

mqttContext *mqtt_create(const char *address, const char *clientID,
                         int keepAliveInterval, Arena *a, IPC_Channel *subC,
                         IPC_Channel *pubC) {
  int rc;

  mqttContext *ctx = (mqttContext *)arena_alloc(a, sizeof(mqttContext));
  if (ctx == NULL) {
    LOG_ERROR("Failed to allocate memory for mqttContext");
    return NULL;
  }

  ctx->status = MQTT_DISCONNECTED;

  rc = MQTTClient_create(&ctx->client, address, clientID,
                         MQTTCLIENT_PERSISTENCE_NONE, NULL);
  if (rc != MQTTCLIENT_SUCCESS) {
    LOG_ERROR("Failed to create MQTT client. RC: %d", rc);
    free(ctx);
    return NULL;
  }

  ctx->subChannel = subC;
  ctx->pubChannel = pubC;

  // Handle callbacks
  rc = MQTTClient_setCallbacks(ctx->client, NULL, mqtt_on_connection_lost,
                               mqtt_on_message_arrived, NULL);
  if (rc != MQTTCLIENT_SUCCESS) {
    LOG_ERROR("Failed to set callbakcs");
    free(ctx);
    return NULL;
  }

  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  conn_opts.keepAliveInterval = 30;
  conn_opts.cleansession = 1;

  rc = MQTTClient_connect(ctx->client, &conn_opts);
  if (rc != MQTTCLIENT_SUCCESS) {
    LOG_ERROR("Failed to connect to MQTT broker. RC: %d", rc);
    MQTTClient_destroy(&ctx->client);
    free(ctx);
    return NULL;
  }

  LOG_INFO("MQTT client connected successfully to %s", address);
  ctx->status = MQTT_CONNECTED;
  return ctx;
}

int mqtt_pub_message(mqttContext *ctx, const char *topic, const char *payload) {
  if (ctx == NULL || ctx->status != MQTT_CONNECTED) {
    LOG_ERROR("MQTT client is not connected");
    return -1;
  }

  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  MQTTClient_deliveryToken token = 0;

  pubmsg.payload = (char *)payload;
  pubmsg.payloadlen = strlen(payload);
  pubmsg.qos = 1;
  pubmsg.retained = 0;

  int rc;
  rc = MQTTClient_publishMessage(ctx->client, topic, &pubmsg, &token);
  if (rc != MQTTCLIENT_SUCCESS) {
    LOG_ERROR("Failed to publish message. (token %d) RC: %d", token, rc);
    return rc;
  }

  rc = MQTTClient_waitForCompletion(ctx->client, token, 10000);
  if (rc != MQTTCLIENT_SUCCESS) {
    LOG_ERROR("Failed to publish message. (token %d) RC: %d", token, rc);
    return rc;
  }

  LOG_INFO("Message with delivery token %d delivered", token);
  return 0;
}

void mqtt_disconnect_and_free(mqttContext *ctx) {
  if (ctx == NULL) {
    return;
  }

  if (ctx->status == MQTT_CONNECTED) {
    MQTTClient_disconnect(ctx->client, 10000);
    LOG_INFO("MQTT client disconnected successfully");
  }

  MQTTClient_destroy(&ctx->client);
  free(ctx);
}

volatile MQTTClient_deliveryToken deliveredtoken;

void mqtt_message_delivered(void *context, MQTTClient_deliveryToken dt) {
  LOG_INFO("Message with token value %d delivery confirmed", dt);
  deliveredtoken = dt;
}

int mqtt_on_message_arrived(void *context, char *topic, int topicLen,
                            MQTTClient_message *msg) {
  mqttContext *ctx = (mqttContext *)context;

  ssize_t bytes_written =
      ipc_write_nonblocking(ctx->subChannel, msg->payload, msg->payloadlen);

  if (bytes_written < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      LOG_WARN("Command buffer full. Discarding message at topic %s", topic);
    } else {
      LOG_ERROR("Fatal error writing to Pipe: %s", strerror(errno));
      return 1;
    }
  } else {
    LOG_DEBUG("Message successfully written to pipe %s (%d bytes)",
              ctx->subChannel->fd, (int)bytes_written);
  }

  MQTTClient_freeMessage(&msg);
  MQTTClient_free(topic);

  return 0;
}

void mqtt_on_connection_lost(void *context, char *cause) {
  LOG_WARN("Connection lost. Cause: %s", cause);
  mqttContext *ctx = (mqttContext *)context;
  ctx->status = MQTT_DISCONNECTED;
}
