#ifndef MQTT_WRAPPER_H
#define MQTT_WRAPPER_H

#include "../../include/arena.h"
#include "../../vendor/paho.mqtt.c/src/MQTTClient.h"
#include <stdint.h>

enum MqttStatus {
  MQTT_DISCONNECTED = 0,
  MQTT_CONNECTED = 1,
  MQTT_ATTEMPT_CONNECT = 2
};

/* *
 * Structure holding the Paho MQTT client instance,
 * connection status, and the IPC socket file descriptor.
 */
typedef struct mqttContext {
  MQTTClient client;
  uint8_t status;
  int ipc_socket_fd;
} mqttContext;

/* *
 * Initializes the MQTT client, allocates memory, and connects to the broker.
 * * Returns:
 * Pointer to the new mqttContext if successful.
 * NULL if memory allocation fails or connection is refused.
 */
mqttContext *mqtt_create_context(const char *address, const char *clientID,
                                 int keepAliveInterval, Arena *a, int sock_fd);

/* *
 * Publishes a message to a topic with QoS 1.
 * This function blocks until the message is delivered or a timeout occurs.
 * * Returns:
 * 0 if the message was delivered successfully.
 * Non-zero error code if the publication failed.
 */
int mqtt_pub_message(mqttContext *ctx, const char *topic, const char *payload);

/* *
 * Disconnects the client (if connected) and frees all allocated memory.
 * It is safe to pass NULL to this function.
 */
void mqtt_disconnect_and_free(mqttContext *ctx);

/* *
 * Callback triggered when a published message delivery is confirmed by the
 * broker.
 */
void mqtt_message_delivered(void *context, MQTTClient_deliveryToken dt);

/* *
 * Callback triggered when a message arrives from a subscribed topic.
 * Packages the payload into an IPCMessage and sends it to the Main Controller.
 * * Returns:
 * 1 (True) to tell the Paho library the message was successfully processed.
 * 0 (False) to indicate failure, causing Paho to re-deliver QoS 1 or 2
 * messages.
 */
int mqtt_on_message_arrived(void *context, char *topic, int topicLen,
                            MQTTClient_message *msg);

/* *
 * Callback triggered when the connection to the MQTT broker is lost.
 */
void mqtt_on_connection_lost(void *context, char *cause);

/* *
 * Subscribes the client to a specific topic.
 * Must be called AFTER a successful connection.
 * * Returns:
 * 0 on success.
 * Non-zero error code if the subscription failed.
 */
int mqtt_subscribe(mqttContext *ctx, const char *topic, int qos);

#endif // MQTT_WRAPPER_H
