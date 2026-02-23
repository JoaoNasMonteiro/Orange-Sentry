#include <bits/types/struct_timeval.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "../../include/arena.h"
#include "../../vendor/paho.mqtt.c/src/MQTTClient.h"
#include "mqtt.h"

#include "../../include/logging.h"
#include "../../include/sockclient.h"

mqttContext *mqtt_create_context(const char *address, const char *clientID,
                                 int keepAliveInterval, Arena *a, int sock_fd) {
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

  ctx->ipc_socket_fd = sock_fd;

  // Handle callbacks
  rc = MQTTClient_setCallbacks(ctx->client, ctx, mqtt_on_connection_lost,
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

  LOG_INFO("MQTT client created successfully and connected to %s", address);
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
}

int mqtt_subscribe(mqttContext *ctx, const char *topic, int qos) {
  if (ctx == NULL || ctx->status != MQTT_CONNECTED) {
    LOG_ERROR("Cannot subscribe: MQTT client is not connected");
    return -1;
  }

  int rc = MQTTClient_subscribe(ctx->client, topic, qos);

  if (rc != MQTTCLIENT_SUCCESS) {
    LOG_ERROR("Failed to subscribe to topic %s. RC: %d", topic, rc);
    return rc;
  }

  LOG_INFO("Successfully subscribed to topic: %s (QoS %d)", topic, qos);
  return 0;
}

volatile MQTTClient_deliveryToken deliveredtoken;

void mqtt_message_delivered(void *context, MQTTClient_deliveryToken dt) {
  LOG_INFO("Message with token value %d delivery confirmed", dt);
  deliveredtoken = dt;
}

int mqtt_on_message_arrived(void *context, char *topic, int topicLen,
                            MQTTClient_message *msg) {
  mqttContext *ctx = (mqttContext *)context;

  IPCMessage ipc_msg;
  memset(&ipc_msg, 0, sizeof(IPCMessage));

  ipc_msg.origin = MOD_MQTT;
  ipc_msg.msgtype = MSG_EVT_MQTT_SUB_MSG;

  struct timeval tv;
  gettimeofday(&tv, NULL);
  ipc_msg.timestamp_ms =
      (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;

  size_t max_topic_len = sizeof(ipc_msg.payload.mqtt_sub_evt.topic) - 1;
  size_t copy_topic_len = (topicLen > max_topic_len) ? max_topic_len : topicLen;

  strncpy(ipc_msg.payload.mqtt_sub_evt.topic, topic, copy_topic_len);
  ipc_msg.payload.mqtt_sub_evt.topic[copy_topic_len] = '\0'; // Garante o nulo

  uint16_t cpylen = (msg->payloadlen > 256) ? 256 : msg->payloadlen;
  memcpy(ipc_msg.payload.mqtt_sub_evt.data, msg->payload, cpylen);

  ipc_msg.payload.mqtt_sub_evt.data_len = cpylen;

  ipc_msg.payload_len = sizeof(ipc_msg.payload.mqtt_sub_evt);

  int bytes_written = ipc_client_send(ctx->ipc_socket_fd, &ipc_msg);

  if (bytes_written < 0) {
    LOG_ERROR("Failed to send message to controller");
    return 0;
  } else {
    LOG_INFO("Message successfully sent to controller %d (%d bytes)",
             ctx->ipc_socket_fd, bytes_written);
  }

  MQTTClient_freeMessage(&msg);
  MQTTClient_free(topic);

  return 1;
}

void mqtt_on_connection_lost(void *context, char *cause) {
  LOG_WARN("Connection lost. Cause: %s", cause);
  mqttContext *ctx = (mqttContext *)context;
  ctx->status = MQTT_DISCONNECTED;
}
