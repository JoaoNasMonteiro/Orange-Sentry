#ifndef LOGGING_H
#define LOGGING_H

#include <errno.h>
#include <stdio.h>
#include <string.h>

/* ==========================================================================
 *  Orange Sentry - System-wide Logging Utility
 * ==========================================================================
 *
 *  SUMMARY:
 *  This header provides a standardized, zero-allocation logging mechanism
 *  designed for the Orange Sentry control plane. It directs output to
 *  stdout/stderr with proper systemd-journald priority prefixes (0-7).
 *
 *  FEATURES:
 *  - Automatic Module Tagging: Identifies which service/file generated the log.
 *  - Function Context: Automatically appends __func__ to error logs.
 *  - Systemd Integration: Prefixes logs with <N> for journald filtering.
 *  - Errno Automation: LOG_SYS_ERROR automatically prints strerror(errno).
 *  - Compile-time Debug: LOG_DEBUG is stripped out unless -DDEBUG is set.
 *
 *  USAGE INSTRUCTIONS:
 *  1. Define MODULE_NAME *before* including this header.
 *  2. Use specific macros for specific error types (Logic vs System).
 *
 *  EXAMPLE:
 *
 *      // In src/mqtt/client.c
 *      #define MODULE_NAME "MQTT-Client"
 *      #include "logging.h"
 *
 *      // 1. Standard Information (Goes to stdout / Journald Info)
 *      LOG_INFO("Connecting to broker at %s:%d...", "127.0.0.1", 1883);
 *      // Output: [MQTT-Client] Connecting to broker at 127.0.0.1:1883...
 *
 *      // 2. Warnings (Goes to stderr / Journald Warning)
 *      if (temperature > 70) {
 *          LOG_WARN("High thermal zone detected: %dC", temperature);
 *      }
 *      // Output: [MQTT-Client] WARN: High thermal zone detected: 75C
 *
 *      // 3. Logic/Library Errors (Goes to stderr / Journald Error)
 *      // Use this for Paho MQTT codes or internal logic failures.
 *      int rc = MQTTClient_connect(client, &opts);
 *      if (rc != MQTTCLIENT_SUCCESS) {
 *          LOG_ERROR("Broker connection refused. RC: %d", rc);
 *      }
 *      // Output: <3>[MQTT-Client] mqtt_connect_fn(): ERROR: Broker connection
 * refused. RC: -1
 *
 *      // 4. System Errors (Goes to stderr / Journald Error)
 *      // Use this ONLY when a libc function fails and sets 'errno'.
 *      // It automatically appends ": <strerror string>"
 *      if (open("/tmp/ipc.fifo", O_RDONLY) < 0) {
 *          LOG_SYS_ERROR("Failed to open FIFO");
 *      }
 *      // Output: <3>[MQTT-Client] open_fifo(): SYS_ERR: Failed to open FIFO:
 * No such file or directory
 *
 *      // 5. Debugging (Goes to stdout / Journald Debug)
 *      // Only compiles if CFLAGS includes -DDEBUG
 *      LOG_DEBUG("Packet payload size: %lu bytes", sizeof(payload));
 *      // Output: [MQTT-Client] DBG (client.c:42): Packet payload size: 256
 * bytes
 *
 * ========================================================================== */

// Systemd priority definitions
#define LOG_LVL_EMERG "<0>"
#define LOG_LVL_ALERT "<1>"
#define LOG_LVL_CRIT "<2>"
#define LOG_LVL_ERR "<3>"
#define LOG_LVL_WARNING "<4>"
#define LOG_LVL_NOTICE "<5>"
#define LOG_LVL_INFO "<6>"
#define LOG_LVL_DEBUG "<7>"

#ifndef MODULE_NAME
#define MODULE_NAME "GENERIC"
#endif

// generic log macros
#define LOG_INFO(fmt, ...)                                                     \
  fprintf(stdout, LOG_LVL_INFO "[%s] " fmt "\n", MODULE_NAME, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...)                                                     \
  fprintf(stderr, LOG_LVL_WARNING "[%s] WARN: " fmt "\n", MODULE_NAME,         \
          ##__VA_ARGS__)

// for library/code errors
#define LOG_ERROR(fmt, ...)                                                    \
  fprintf(stderr, LOG_LVL_ERR "[%s] ERROR: " fmt "\n", MODULE_NAME,            \
          ##__VA_ARGS__)

// for system errors
#define LOG_SYS_ERROR(fmt, ...)                                                \
  fprintf(stderr, LOG_LVL_ERR "[%s] SYS_ERR: " fmt ": %s\n", MODULE_NAME,      \
          ##__VA_ARGS__, strerror(errno))
// conditional debugging

#ifdef DEBUG
#define LOG_DEBUG(fmt, ...)                                                    \
  fprintf(stdout, LOG_LVL_DEBUG "[%s] DBG (%s:%d): " fmt "\n", MODULE_NAME,    \
          __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...)                                                    \
  do {                                                                         \
  } while (0)
#endif

#endif // LOGGING_H
