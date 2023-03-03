/**
 *
 * Microvisor Config Handler
 * Version 1.0.0
 * Copyright © 2022, Twilio
 * Licence: Apache 2.0
 *
 */


#ifndef CONFIG_HANDLER_H
#define CONFIG_HANDLER_H


#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/*
 * MACROS
 */
#define STRING_ITEM(name, str) \
    . name = { .data = (uint8_t*)str, . length = strlen(str) }


/*
 * DEFINES
 */
#define TAG_CHANNEL_CONFIG 100

#define BUF_CLIENT_SIZE 34
#define BUF_BROKER_HOST 128
#define BUF_ROOT_CA 1024
#define BUF_LOSANT_CLIENT_ID 128
#define BUF_LOSANT_ACCESS_KEY 128
#define BUF_LOSANT_ACCESS_SECRET 128

#define BUF_READ_BUFFER 3*1024


#ifdef __cplusplus
extern "C" {
#endif


/*
 * PROTOTYPES
 */
void start_configuration_fetch();
void receive_configuration_items();
void finish_configuration_fetch();


/*
 * GLOBALS
 */
extern uint8_t  client[BUF_CLIENT_SIZE];
extern size_t   client_len;

extern uint8_t  broker_host[BUF_BROKER_HOST];
extern size_t   broker_host_len;
extern uint16_t broker_port;
extern uint8_t  root_ca[BUF_ROOT_CA];
extern size_t   root_ca_len;
extern uint8_t  losant_client_id[BUF_LOSANT_CLIENT_ID];
extern size_t   losant_client_id_len;
extern uint8_t  losant_access_key[BUF_LOSANT_ACCESS_KEY];
extern size_t   losant_access_key_len;
extern uint8_t  losant_access_secret[BUF_LOSANT_ACCESS_SECRET];
extern size_t   losant_access_secret_len;


#ifdef __cplusplus
}
#endif


#endif /* CONFIG_HANDLER_H */
