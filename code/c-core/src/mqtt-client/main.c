//  #include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../../include/arena.h"
#include "../../include/fifo-ipc.h"
#include "../../vendor/paho.mqtt.c/src/MQTTClient.h"

//Boilerplate stuff
#define ARENA_SIZE (64 * 1024)
#define LOG_FIFO_NAME "/tmp/test_fifo"

static uint8_t client_memory[ARENA_SIZE];

//MQTT Stuff
// TODO move the MQTT stuff into it's own function
// TODO also need to make it work by sending the messages it recieved from the FIFO pipes
#define ADDRESS "tcp://localhost:1883"
#define CLIENTID "ExampleClientPub"
#define TOPIC "MQTT Examples"
#define PAYLOAD "Hello World!"
#define QOS 1
#define TIMEOUT 10000L


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

  // do mqtt stuff
  // First we declare and initialize the needed variables
  // client, connection options, message, delivery token, return code
  MQTTClient client;
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  MQTTClient_deliveryToken token;

  //then we create the client based on those variables
  int rc;
  MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE,NULL);
  //set keepalive interval and clean session parameters. those are important
  conn_opts.keepAliveInterval = 20;
  conn_opts.cleansession = 1;

  //connect to the broker
  if (rc = MQTTClient_connect);

  if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
    perror(!"[Error] Failed to connect to MQTT broker");
    exit(EXIT_FAILURE);
  }

  //prepare the message to be sent
  pubmsg.payload = PAYLOAD;
  pubmsg.payloadlen = strlen(PAYLOAD);
  pubmsg.qos = QOS;
  pubmsg.retained = 0;

  //publish the message
  MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
  printf("Waiting for up to %d seconds for publication of %s\non topic %s for client with ClientID: %s\n", (int)(TIMEOUT / 1000), PAYLOAD, TOPIC, CLIENTID);
  rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
  
  //wait for the message to be delivered
  printf("Message with delivery token %d delivered\n", token);
  
  //disconnect from the broker and destroy the client
  MQTTClient_disconnect(client, 10000);
  MQTTClient_destroy(&client);

  // end MQTT stuff

  // close down named pipe and reset arena
  arena_reset(&arena);
  ipc_close_channel(&mqtt_log_channel);
  return 0;
}
