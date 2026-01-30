#include "../../vendor/paho.mqtt.c/src/MQTTClient.h"

#define MODULE_NAME "MQTT_CLIENT"
#include "../../include/logging.h"

typedef struct mqttContext {
  MQTTClient client;
  uint8_t status;
} mqttContext;

enum MqttStatus { MQTT_DISCONNECTED = 0, MQTT_CONNECTED = 1, MQTT_ATTEMPT_CONNECT = 2 };

mqttContext* mqtt_init(const char* address, const char* clientID, int keepAliveInterval) {
    int rc;

    mqttContext* ctx = (mqttContext*)malloc(sizeof(mqttContext));
    if (ctx == NULL){
        LOG_ERROR("Failed to allocate memory for mqttContext");
        return NULL;
    }

    ctx->status = MQTT_DISCONNECTED;

    rc = MQTTClient_create(&ctx->client, address, clientID, MQTTCLIENT_PERSISTENCE_NONE, NULL); 
    if (rc != MQTTCLIENT_SUCCESS) {
        LOG_ERROR("Failed to create MQTT client. RC: %d", rc);
        free(ctx);
        return NULL;
    }

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.keepAliveInterval = keepAliveInterval;
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

int mqtt_pub_message(mqttContext* ctx, const char* topic, const char* payload){
    if (ctx == NULL || ctx->status != MQTT_CONNECTED) {
        LOG_ERROR("MQTT client is not connected");
        return -1;
    }

    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token = 0;

    pubmsg.payload = (char*)payload;
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

void mqtt_disconnect_and_free(mqttContext* ctx) {
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
#include "../../vendor/paho.mqtt.c/src/MQTTClient.h"

#define MODULE_NAME "MQTT_CLIENT"
#include "../../include/logging.h"

typedef struct mqttContext {
  MQTTClient client;
  uint8_t status;
} mqttContext;

enum MqttStatus { MQTT_DISCONNECTED = 0, MQTT_CONNECTED = 1, MQTT_ATTEMPT_CONNECT = 2 };

mqttContext* mqtt_init(const char* address, const char* clientID, int keepAliveInterval) {
    int rc;

    mqttContext* ctx = (mqttContext*)malloc(sizeof(mqttContext));
    if (ctx == NULL){
        LOG_ERROR("Failed to allocate memory for mqttContext");
        return NULL;
    }

    ctx->status = MQTT_DISCONNECTED;

    rc = MQTTClient_create(&ctx->client, address, clientID, MQTTCLIENT_PERSISTENCE_NONE, NULL); 
    if (rc != MQTTCLIENT_SUCCESS) {
        LOG_ERROR("Failed to create MQTT client. RC: %d", rc);
        free(ctx);
        return NULL;
    }

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.keepAliveInterval = keepAliveInterval;
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

int mqtt_pub_message(mqttContext* ctx, const char* topic, const char* payload){
    if (ctx == NULL || ctx->status != MQTT_CONNECTED) {
        LOG_ERROR("MQTT client is not connected");
        return -1;
    }

    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token = 0;

    pubmsg.payload = (char*)payload;
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

void mqtt_disconnect_and_free(mqttContext* ctx) {
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
