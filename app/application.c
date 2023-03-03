#include "application.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "stm32u5xx_hal.h"
#include "mv_syscalls.h"
#include "cmsis_os.h"

#include "work.h"
#include "log_helper.h"

#if defined(APPLICATION_TEMPERATURE)
#  include "i2c_helper.h"
#endif

#if defined(APPLICATION_SWITCH)
#  include "switch_helper.h"
#endif
/*
 *  FORWARD DECLARATIONS
 */
static void application_init();
static void application_poll();
static void application_process_message(const uint8_t* topic, size_t topic_len,
                                        const uint8_t* payload, size_t payload_len);
/*
 *  GENERIC DATA
 */
char application_message_payload[128];

static bool mqtt_connected = false;
static bool message_in_flight = false;
osMessageQueueId_t applicationMessageQueue;

/*
 *  APPLICATION_SPECIFIC DATA
 */
#if defined(APPLICATION_DUMMY)
static uint64_t last_send_microsec = 0;
static double latitude = 0.0;
static double longitude = 0.0;
static bool application_running = true;
#elif defined(APPLICATION_TEMPERATURE)
static uint64_t last_send_microsec = 0;
static bool application_running = true;
static bool i2c_initialised = true;

#define TH02_ADDR 0x80

#define TH02_STATUS_ADDR 0
#define TH02_DATAH_ADDR 1
#define TH02_CONFIG_ADDR 3

#define TH02_CONFIG_START 0x01
#define TH02_CONFIG_TEMP 0x10

#define TH02_STATUS_RDY 0x01
#endif

/**
 * @brief Push message into application queue.
 *
 * @param  type: ApplicationMessageType enumeration value
 */
void pushApplicationMessage(enum ApplicationMessageType type) {
    osStatus_t status;
    if ((status = osMessageQueuePut(applicationMessageQueue, &type, 0U, 0U)) != osOK) {
        server_error("failed to post application message: %ld", status);
    }
}

/**
 * @brief Function implementing the Application task thread.
 *
 * @param  argument: Not used.
 */
void start_application_task(void *argument) {
    applicationMessageQueue = osMessageQueueNew(16, sizeof(enum ApplicationMessageType), NULL);
    if (applicationMessageQueue == NULL) {
        server_error("failed to create queue");
        return;
    }

    application_init();

    enum ApplicationMessageType messageType;

    // The task's main loop
    while (1) {
        if (osMessageQueueGet(applicationMessageQueue, &messageType, NULL, 100U ) == osOK) {
            // server_log("application event loop received a message: 0x%02x", messageType);
            switch (messageType) {
                case OnMqttConnected:
                    mqtt_connected = true;
                    break;
                case OnMqttDisconnected:
                    mqtt_connected = false;
                    break;
                case OnIncomingMqttMessage:
                    application_process_message(incoming_message_topic,
                                                incoming_message_topic_len,
                                                incoming_message_payload,
                                                incoming_message_payload_len);
                    pushWorkMessage(OnApplicationConsumedMessage);
                    break;
                case OnMqttMessageSent:
                    message_in_flight = false;
                    break;
            }
        }

        application_poll();
    }
}

void application_init() {
    last_send_microsec = 0;
    latitude = 51.5081;
    longitude = -0.1248;
    application_running = true;
}

void application_poll() {
    uint64_t current_microsec = 0;
    mvGetMicroseconds(&current_microsec);

    if (application_running && mqtt_connected && !message_in_flight && ((current_microsec - last_send_microsec) > 60*1000*1000)) { // trigger approx every 60 seconds, depending on how chatty other messages are (relying on 100ms timeout for osMessageQueueGet above)
       last_send_microsec = current_microsec;
       message_in_flight = true;

        double random_bearing = (double)rand()/(double)(RAND_MAX/6.28318530718);
        latitude += 0.005 * sin(random_bearing);
        longitude += 0.005 * cos(random_bearing);

       sprintf(application_message_payload, "{\"data\":{\"location\":[%.8f,%.8f]}}", latitude, longitude);
       server_log("sending: %.*s", strlen(application_message_payload), application_message_payload);
       pushWorkMessage(OnApplicationProducedMessage);
    }
}

void application_process_message(const uint8_t* topic, size_t topic_len,
                                 const uint8_t* payload, size_t payload_len) {
    // server_log("Got a message on topic '%.*s' with payload '%.*s",
    //            (int) topic_len, topic,
    //            (int) payload_len, payload);
    if (strncmp("stop", (char*) payload, payload_len) == 0) {
        application_running = false;
    }

    if (strncmp("restart", (char*) payload, payload_len) == 0) {
        application_running = true;
    }
}
