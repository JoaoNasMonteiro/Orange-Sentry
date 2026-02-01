#ifndef MQTT_WRAPPER_H
#define MQTT_WRAPPER_H

#include "../../vendor/paho.mqtt.c/src/MQTTClient.h"
#include <stdint.h>

/* * Opaque structure to hide Paho MQTT implementation details.
 * The actual definition is inside mqtt.c.
 */

typedef struct mqttContext {
  MQTTClient client;
  uint8_t status;
} mqttContext;

/* * Initializes the MQTT client, allocates memory, and connects to the broker.
 * * Returns:
 * Pointer to the new mqttContext if successful.
 * NULL if memory allocation fails or connection is refused.
 * * Example:
 * mqttContext *ctx = mqtt_init("tcp://localhost:1883", "Sensor_01", 20);
 */
mqttContext *mqtt_init(const char *address, const char *clientID,
                       int keepAliveInterval);

/* * Publishes a message to a topic with QoS 1.
 * This function blocks until the message is delivered or a timeout occurs.
 * * Returns:
 * 0 if the message was delivered successfully.
 * Non-zero error code if the publication failed.
 * * Example:
 * if (mqtt_pub_message(ctx, "sensors/temp", "25.5") != 0) {
 * // Handle error
 * }
 */
int mqtt_pub_message(mqttContext *ctx, const char *topic, const char *payload);

/* * Disconnects the client (if connected) and frees all allocated memory.
 * It is safe to pass NULL to this function.
 * * Example:
 * mqtt_disconnect_and_free(ctx);
 */
void mqtt_disconnect_and_free(mqttContext *ctx);

#endif // MQTT_WRAPPER_H
