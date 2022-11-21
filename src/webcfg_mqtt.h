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
#include <uuid/uuid.h>
#include <mosquitto.h>
#include <openssl/ssl.h>
#include <time.h>
#include <math.h>
#include "webcfg.h"
#include "webcfg_log.h"
#include "webcfg_multipart.h"
#include "webcfg_metadata.h"
#include "webcfg_auth.h"
#include "webcfg_db.h"
#include "webcfg_generic.h"

#define HOST_FILE_LOCATION   "/nvram/hostname.txt"
#define MOSQ_TLS_VERSION     "tlsv1.2"
#define KEEPALIVE       180
#define MQTT_PORT            443

bool webcfg_mqtt_init(int status, char *systemreadytime);
void get_from_file(char *key, char **val);
void publish_notify_mqtt(char *pub_topic, void *payload, ssize_t len, char * dest);
char * createMqttPubHeader(char * payload, char * dest, ssize_t * payload_len);
int get_global_mqtt_connected();
void reset_global_mqttConnected();
void set_global_mqttConnected();
int createMqttHeader(char **header_list);
int triggerBootupSync();
#endif
