/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef _WEBCFG_MQTT_H_
#define _WEBCFG_MQTT_H_

#include <stdio.h>
#include <stdbool.h>
#include <mosquitto.h>
#include <openssl/ssl.h>
#include "webcfg.h"
#include "webcfg_log.h"

#define TOPIC_FILE_LOCATION      "/nvram/topic.txt"
#define HOST_FILE_LOCATION   "/nvram/hostname.txt"
#define SUBSCRIBE_FILE_LOCATION   "/nvram/subscribe.txt"
#define CA_FILE_PATH "/nvram/cafile.txt"
#define CERT_FILE_PATH "/nvram/certfile.txt"
#define KEY_FILE_PATH "/nvram/keyfile.txt"

#define HOST_NAME_MAX_SZ       256
#define MOSQWEV_CID_SZ       64
#define MOSQWB_TLS_VERSION  "tlsv1.2"
#define STATS_MQTT_PORT         8883
#define STATS_MQTT_QOS          0
#define MOSQWB_KEEPALIVE   180  
static const char mosqwev_ciphers[] = TLS1_TXT_DHE_DSS_WITH_AES_128_SHA256
                                  ":"TLS1_TXT_DHE_RSA_WITH_AES_128_SHA256
                                  ":"TLS1_TXT_DHE_RSA_WITH_AES_256_SHA256
                                  ":"TLS1_TXT_DHE_RSA_WITH_AES_128_GCM_SHA256
                                  ":"TLS1_TXT_DHE_DSS_WITH_AES_128_GCM_SHA256
                                  ":"TLS1_TXT_ECDHE_ECDSA_WITH_AES_128_SHA256
                                  ":"TLS1_TXT_ECDHE_ECDSA_WITH_AES_256_SHA384
                                  ":"TLS1_TXT_ECDHE_RSA_WITH_AES_128_SHA256
                                  ":"TLS1_TXT_ECDHE_RSA_WITH_AES_256_SHA384
                                  ":"TLS1_TXT_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256
                                  ":"TLS1_TXT_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384
                                  ":"TLS1_TXT_ECDHE_RSA_WITH_AES_128_GCM_SHA256
                                  ":"TLS1_TXT_ECDHE_RSA_WITH_AES_256_GCM_SHA384;


bool WebcfgMqttInit(void);
void WebcfgMqttUninit(void);
void get_from_file(char *key, char **val);

typedef struct mosqwev mosqwev_t;

typedef void mosqwev_log_cbk_t(mosqwev_t *self, void *data, int level, const char *str);
typedef void mosqwev_cbk_t(mosqwev_t *self, void *data, int result);
typedef void mosqwev_message_cbk_t(mosqwev_t *self, void *data, const char *topic, void *msg, size_t msglen);
typedef void mosqwev_subscribe_cbk_t(mosqwev_t *self, void *data, int mid, int qos_n, const int *qos_v);
typedef void mosqwev_pwd_cbk_t(char *buf, int size, int rwflags, void *data);

struct mosqwev
{
    char                me_cid[MOSQWEV_CID_SZ];  /* Client ID */
    char                me_host[HOST_NAME_MAX]; /* Broker hostname */
    int                 me_port;                /* Broker port */
    struct mosquitto   *me_mosq;                /* Mosquitto library instance */
    bool                me_connected;           /* Connected state - safe to listen for write requests */
    bool                me_connecting;          /* mosquitto_connect() called but CONNACK yet to be received */
    /* Callbacks */
    void               *me_data;                /* Callback context data */
    mosqwev_cbk_t       *me_connect_cbk;         /* Connect handler */
    mosqwev_cbk_t       *me_disconnect_cbk;      /* Disconnect handler */
    mosqwev_cbk_t       *me_publish_cbk;         /* Publish handler */
    mosqwev_message_cbk_t *me_message_cbk;         /* Message handler */
    mosqwev_subscribe_cbk_t *me_subscribe_cbk;       /* Subscribe handler */
    mosqwev_cbk_t       *me_unsubscribe_cbk;     /* Unsubscribe handler */
    /* Memoized settings for reinit */
    char              *me_cafile;
    char              *me_capath;
    char              *me_certfile;
    char              *me_keyfile;
    char              *me_tls_version;
    char              *me_ciphers;
    int                me_cert_reqs;
    mosqwev_pwd_cbk_t  *me_pw_callback;
};

void webcfg_mqtt_connect();
bool webcfg_mqtt_is_connected();
void webcfg_mqtt_set(const char *broker, const char *port, const char *topic, const char *subscribe, const char *qos, int compress);

bool mosqwev_connect(mosqwev_t *self, char *host, char *subscribe, int port);
void mosqwev_subscribe_topic(mosqwev_t *self,char *subscribe);
bool mosqwev_disconnect(mosqwev_t *self);
bool mosqwev_reconnect(mosqwev_t *self);
bool mosqwev_is_connected(mosqwev_t *self);
bool mosqwev_tls_set(mosqwev_t *self, const char *cafile, const char *capath, const char *certfile, const char *keyfile, mosqwev_pwd_cbk_t *pw_callback);
bool mosqwev_tls_opts_set(mosqwev_t *self, int cert_reqs, const char *tls_version, const char *ciphers);
bool mosqwev_publish(mosqwev_t *self, int *mid, const char *topic, size_t msglen, void *msg, int qos, bool retain);
void mosqwev_connect_callback_internal( mosqwev_t *self, void *data, int result);
void mosqwev_subscribe_message_callback(mosqwev_t *self, void *data, const char *topic, void *msg, size_t msglen);
void mosqwev_subscribe_callback_internal( mosqwev_t *self, void *data, int mid, int qos_count, const int *granted_qos) ;

void mosqwev_connect_cbk_set(mosqwev_t *self, mosqwev_cbk_t *cbk);
void mosqwev_disconnect_cbk_set(mosqwev_t *self, mosqwev_cbk_t *cbk);
void mosqwev_publish_cbk_set(mosqwev_t *self, mosqwev_cbk_t *cbk);
void mosqwev_message_cbk_set(mosqwev_t *self, mosqwev_message_cbk_t *cbk);
void mosqwev_subscribe_cbk_set(mosqwev_t *self, mosqwev_subscribe_cbk_t *cbk);
void mosqwev_unsubscribe_cbk_set(mosqwev_t *self, mosqwev_cbk_t *cbk);
bool mosqwev_init(mosqwev_t *self, const char *cid, void *data);
#endif
