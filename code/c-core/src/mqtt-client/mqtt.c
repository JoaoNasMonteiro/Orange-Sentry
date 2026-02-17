#include <stdio.h>
#include <stdlib.h>

#include "../../include/arena.h"
#include "../../vendor/paho.mqtt.c/src/MQTTClient.h"
#include "mqtt.h"

#include "../../include/fifo-ipc.h"
#include "../../include/logging.h"

// TODO estabelecer estrutura de areanas para alocar as memórias desta
// biblioteca -> usar arenas pequenas (lifetime do programa? alocar a arena
// aqui? não, importar arena.h e somente usar as funções para pedir arenas para
// a main.c antes disso vou ter que impleemtar alocadores slab nela, umabacking
// arena grande que se divide em vários slabs)

// Para isso tenho que mudar minha estratégia de build. Em ves de compilar para
// inário e linkar, posso simplemente dar include deste arquivo dentro do main
// para evitar dependency hell, então eu poderei usar todos os recursos da
// backing arena definida dentro da main. Para isso eu ainda vou ter que mudar a
// lógica do arena.h substancialmente para incluir a logica de subarenas/slabs.

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

// sub stuff

volatile MQTTClient_deliveryToken deliveredtoken;

void mqtt_message_delivered(void *context, MQTTClient_deliveryToken dt) {
  LOG_INFO("Message with token value %d delivery confirmed", dt);
  deliveredtoken = dt;
}

int sentry_on_message(void *context, char *topic, int topicLen,
                      MQTTClient_message *msg) {
  // 1. Recupera o contexto (onde guardamos o descritor do arquivo)
  mqttContext *ctx = (mqttContext *)context;

  // 2. Protocolo Interno Simplificado:
  // Podemos enviar [Tamanho (int)][Payload (bytes)] para facilitar a leitura lá
  // na frente. Mas para este exemplo, vamos assumir escrita direta dos dados.

  // Tenta escrever no Pipe de Comandos
  
  //this pointer to a ponter to a struct inside a struct stuff is so cursed
  ssize_t bytes_written =
      ipc_write_nonblocking(ctx->subChannel->fd, msg->payload, msg->payloadlen);

  if (bytes_written < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // Cenario Crítico: O Main Controller está lento e o Pipe encheu.
      // DECISÃO DE PROJETO: Descartamos a mensagem para salvar a conexão MQTT.
      LOG_WARN("Buffer de comandos CHEIO! Descartando mensagem do tópico: %s",
               topic);
    } else {
      LOG_ERROR("Erro fatal ao escrever no Pipe: %s", strerror(errno));
    }
  } else {
    LOG_DEBUG("Mensagem repassada para o Main Controller (%d bytes)",
              (int)bytes_written);
  }

  // 3. Limpeza Obrigatória (Conforme Paho Docs)
  MQTTClient_freeMessage(&msg);
  MQTTClient_free(topic);

  return 1; // 1 = Sucesso (Consome a mensagem da fila do QoS)
}

int mqtt_on_message_arrived(void *context, char *topic, int topiclen,
                            MQTTClient_message *message) {
  // For this operation to be non-blocking I will just pass it hhot-potato style
  // to the main controlelr, and do any kind of validation, parsing and stuff
  // there.

  char *buffer = arena_alloc(Arena * a, size_t size);

  int max_payload_len = 16;
  char payload_buffer[max_payload_len];
  strncpy(payload_buffer, message->payload, max_payload_len - 2);
  payload_buffer[max_payload_len - 1] = '\0';
  printf("Message ariived at topic %s:\n %s message", topic, payload_buffer);
  MQTTClient_freeMessage(&message);
  MQTTClient_free(topic);
  return 0;
  // this is an unsafe implementation. I can also just implement everything from
  // FIFO here to simplify stuff. I will need to figure out a way to copy the
  // user input safely.
}

void mqtt_on_connection_lost(void *context, char *cause) {
  LOG_WARN("Connection lost. Cause: %s", cause);
}
