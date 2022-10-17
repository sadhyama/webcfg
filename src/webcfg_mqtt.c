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

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include "webcfg_mqtt.h"

//global MQTT instance
mosqwev_t         webcfg_mqtt;

char             webcfg_mqtt_broker[HOST_NAME_MAX_SZ];
char             webcfg_mqtt_topic[HOST_NAME_MAX_SZ];
char             webcfg_mqtt_subscribe[HOST_NAME_MAX_SZ];
int              webcfg_mqtt_port = STATS_MQTT_PORT;
int              webcfg_mqtt_qos = STATS_MQTT_QOS;
uint8_t          webcfg_mqtt_compress = 0;
int              webcfg_mosquitto_init = false;

	
/*
 * Initialize MQTT library
 */
bool WebcfgMqttInit(void)
{
	WebcfgInfo("Initializing MQTT library\n");
	webcfg_mosquitto_init = true;

	mosquitto_lib_init();

	char cID[64];

	char *cid = NULL;

	get_from_file("cid=", &cid);
	if(cid !=NULL)
	{
		strcpy(cID, cid);
		WebcfgInfo("cID is %s\n", cID);
	}

	if (!mosqwev_init(&webcfg_mqtt, cID, NULL))
	{
		WebcfgError("Error initializing MQTT library\n");
		return false;
	}

	//file read and then assign broker ,topic, port
	char * topic, *hostname, *subscribe = NULL;
	get_from_file("TOPIC=", &topic);
	get_from_file("HOSTNAME=", &hostname);
	get_from_file("SUBSCRIBE=", &subscribe);
	if(topic != NULL)
	{
		WebcfgInfo("The topic is %s\n", topic);
	}

	if(hostname != NULL)
	{
		WebcfgInfo("The hostname is %s\n", hostname);
	}
	if(subscribe != NULL)
	{
		WebcfgInfo("subscribe to %s\n", subscribe);
	}
	const char* mqtt_broker = hostname;
	const char* mqtt_topic = topic;
	const char* mqtt_subscribe = subscribe;

	webcfg_mqtt_set(mqtt_broker, "443", mqtt_topic, mqtt_subscribe, 0, 1);
	WebcfgInfo("WebcfgMqttInit done\n");
	return true;
}

bool mosqwev_init(mosqwev_t *self, const char *cid, void *data)
{
    memset(self, 0, sizeof(*self));

    strcpy(self->me_cid, cid);
    self->me_data = data;

    strcpy(self->me_host, "unkown");
    self->me_port = -1;

    /*
     * Instantiate a new Mosquitto structure --
     * The 2nd paramater is the "clean_session" parameter, needs to be true if cid is NULL
     */
    self->me_mosq = mosquitto_new(cid, true, self);
    if (self->me_mosq == NULL)
    {
        WebcfgError("Error initializing Mosquitto instance: CID: %s", cid);
        return false;
    }

    //mosqwev_init_cbk(self);

    return true;
}

/**
 * Set MQTT settings
 */
void webcfg_mqtt_set(const char *broker, const char *port, const char *topic, const char *subscribe, const char *qos, int compress)
{
    const char *new_broker;
    const char *new_subscribe;

    int new_port;
    bool broker_changed = false;
    bool subscribe_changed = false;


    webcfg_mqtt_compress = compress;

    // broker address
    new_broker = broker ? broker : "";
    if (strcmp(webcfg_mqtt_broker, new_broker)) broker_changed = true;
    strcpy(webcfg_mqtt_broker, new_broker);

    // broker port
    new_port = port ? atoi(port) : STATS_MQTT_PORT;
    if (webcfg_mqtt_port != new_port) broker_changed = true;
    webcfg_mqtt_port = new_port;

    // broker subscribe
    new_subscribe = subscribe ? subscribe : "";
    if (strcmp(webcfg_mqtt_subscribe, new_subscribe)) subscribe_changed = true;
    strcpy(webcfg_mqtt_subscribe, new_subscribe);

    // qos
    if (qos)
    {
        webcfg_mqtt_qos = atoi(qos);
    }
    else
    {
        webcfg_mqtt_qos = STATS_MQTT_QOS;
    }

    // topic
    if (topic != NULL)
    {
        strcpy(webcfg_mqtt_topic, topic);
    }
    else
    {
        webcfg_mqtt_topic[0] = '\0';
    }

    WebcfgInfo("webcfg_mqtt_broker %s webcfg_mqtt_topic %s webcfg_mqtt_qos %s webcfg_mqtt_subscribe %s webcfg_mqtt_port %d\n", webcfg_mqtt_broker, webcfg_mqtt_topic, webcfg_mqtt_qos , webcfg_mqtt_subscribe, webcfg_mqtt_port);

    /* Initialize TLS options */
    if (!mosqwev_tls_opts_set(&webcfg_mqtt, SSL_VERIFY_PEER, MOSQWB_TLS_VERSION, mosqwev_ciphers))
    {
        WebcfgError("Error setting TLS options.\n");
    }

	WebcfgInfo("Fetch tls cert from file\n");
    	char * tls_cacert_file, *tls_cert_file , *tls_privkey_file = NULL;
	get_from_file("CA_FILE_PATH=", &tls_cacert_file);
        get_from_file("CERT_FILE_PATH=", &tls_cert_file);
	get_from_file("KEY_FILE_PATH=", &tls_privkey_file);
	WebcfgInfo("MQTT: cafile %s, certfile %s, keyfile %s.\n", tls_cacert_file, tls_cert_file, tls_privkey_file);

    if (!mosqwev_tls_set(&webcfg_mqtt,tls_cacert_file, NULL,tls_cert_file,tls_privkey_file,NULL))
    {
        WebcfgError("Error setting TLS certificates\n");
    }

    WebcfgInfo("MQTT broker: '%s' port: %d topic: '%s' qos: %d compress: %d\n",
            webcfg_mqtt_broker, webcfg_mqtt_port, webcfg_mqtt_topic, webcfg_mqtt_qos, webcfg_mqtt_compress);

    // reconnect if broker changed
    if (broker_changed) {
        WebcfgDebug("MQTT broker changed - reconnecting...");
        if (webcfg_mqtt_is_connected()) {
            // if already connected, disconnect first.
            mosqwev_disconnect(&webcfg_mqtt);
        }
	WebcfgInfo("B4 mqtt_connect\n");
        webcfg_mqtt_connect();
	WebcfgInfo("mqtt_connect done\n");
    }
    else if(subscribe_changed) {
        WebcfgInfo("MQTT subscription changed, while broker is still connected");
        mosqwev_subscribe_topic(&webcfg_mqtt, webcfg_mqtt_subscribe);
    }

	WebcfgInfo("webcfg_mqtt_set end\n");
    return;
}

void webcfg_mqtt_connect()
{
	mosqwev_t *mqtt = &webcfg_mqtt;
	if (!mosqwev_is_connected(mqtt))
	{
		WebcfgInfo("Connecting to %s ...\n", webcfg_mqtt_broker);
		if (!mosqwev_connect(&webcfg_mqtt, webcfg_mqtt_broker, !strlen(webcfg_mqtt_subscribe) ? NULL : webcfg_mqtt_subscribe, webcfg_mqtt_port))
		{
		    WebcfgError("Error in Connecting.\n");
		    return;
		}
		else	
		{
			WebcfgInfo("Mqtt broker connect success\n");
		}
	}
	WebcfgInfo("End webcfg_mqtt_connect\n");
}

/*
 * Connect to a mosquitto broker -- this is a BLOCKING call
 */
bool mosqwev_connect(mosqwev_t *self, char *host, char *subscribe, int port)
{
    int rc;

    strcpy(self->me_host, host);
    self->me_port = port;

    WebcfgInfo("Connecting to: %s:%d:%s", host, port, self->me_cid);

    res_init();

    if (self->me_connecting)
        WebcfgInfo("Previous session was still connecting");

    if (self->me_connected)
        WebcfgInfo("Previous session was still connected");

    /*rc = mosqwev_reinit(self);
    if (rc) {
        WebcfgError("Connection failed due to reinit: %s", mosquitto_strerror(rc));
        return rc;
    }*/

    self->me_connecting = false;
    self->me_connected = false;
 
    self->me_connect_cbk = mosqwev_connect_callback_internal;
    self->me_subscribe_cbk = mosqwev_subscribe_callback_internal; 
    self->me_message_cbk = mosqwev_subscribe_message_callback;

    rc = mosquitto_connect(self->me_mosq, self->me_host, self->me_port, MOSQWB_KEEPALIVE);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        WebcfgError("Connection failed: %s:%d:%s: Error %s",
                host, port, self->me_cid, mosquitto_strerror(rc));

        return false;
    }

    self->me_connecting = true;

    return true;
}

void mosqwev_connect_callback_internal( mosqwev_t *self, void *data, int result)
{
    int ret;

    if(!result)
    {
	self->me_connected = true;
        WebcfgInfo("MQTT Connect Success\n");
        if(strlen(webcfg_mqtt_subscribe))
        {
            WebcfgInfo("MQTT Subscription to %s", webcfg_mqtt_subscribe);
            ret = mosquitto_subscribe(self->me_mosq, NULL, webcfg_mqtt_subscribe, 0);
            WebcfgInfo("MQTT Subscription, ret: %d", ret);
        }
    }
    else
    {
        WebcfgError("MQTT Connect failed\n");
    }
}

void mosqwev_subscribe_callback_internal( mosqwev_t *self, void *data, int mid, int qos_count, const int *granted_qos) {
    WebcfgInfo("MQTT Poke Subscribed Successfully-------");
}

void mosqwev_subscribe_message_callback(mosqwev_t *self, void *data, const char *topic, void *msg, size_t msglen) {
    char msgDecode[2048] = {'\0'};
    WebcfgInfo("MQTT Poke Message Received on topic: %s", topic);
    memcpy(msgDecode, msg, msglen);
    WebcfgInfo("MQTT Poke Message Content: %s ", msgDecode);
}

/*
 * Enable TLS mode; must be called before mosqwev_connect() -- just a wrapper around mosquitto_tls_set()
 */
bool mosqwev_tls_set(mosqwev_t *self,const char *cafile,const char *capath,const char *certfile,const char *keyfile,mosqwev_pwd_cbk_t *pw_callback)
{
    int rc;

    WebcfgInfo( "CAFILE:%s CAPATH:%s CERTFILE:%s KEYFILE:%s PWCBK:%p", cafile, capath, certfile, keyfile, pw_callback);
    rc = mosquitto_tls_set(self->me_mosq, cafile, capath,
            certfile, keyfile, (void *)pw_callback);

#define SETSTR(l, r) do { if (l) free(l); l = NULL; if (r) l = strdup(r); } while (0)
    SETSTR(self->me_cafile, cafile);
    SETSTR(self->me_capath, capath);
    SETSTR(self->me_certfile, certfile);
    SETSTR(self->me_keyfile, keyfile);
    self->me_pw_callback = pw_callback;
#undef SETSTR

    if (rc != MOSQ_ERR_SUCCESS)
    {
        WebcfgError("Error setting TLS: %s", mosquitto_strerror(rc));
        return false;
    }
	WebcfgInfo("mosqwev_tls_set success\n");
    return true;
}

/*
 * Set TLS options; must be called before mosqwev_connect() -- just a wrapper around mosquitto_tls_opts_set()
 */
bool mosqwev_tls_opts_set(mosqwev_t   *self,
                         int         cert_reqs,
                         const char *tls_version,
                         const char *ciphers)
{
    int rc;

    rc = mosquitto_tls_opts_set(self->me_mosq, cert_reqs,
            tls_version, ciphers);

#define SETSTR(l, r) do { if (l) free(l); l = NULL; if (r) l = strdup(r); } while (0)
    self->me_cert_reqs = cert_reqs;
    SETSTR(self->me_tls_version, tls_version);
    SETSTR(self->me_ciphers, ciphers);
#undef SETSTR

    if (rc != MOSQ_ERR_SUCCESS)
    {
        WebcfgError( "Error setting TLS options: %s", mosquitto_strerror(rc));
        return false;
    }
	WebcfgInfo("mosqwev_tls_opts_set success\n");
    return true;
}

void mosqwev_subscribe_topic( mosqwev_t *self, char *subscribe)
{
   int ret;
   WebcfgInfo("MQTT Subscribing to: %s", subscribe);
   ret = mosquitto_subscribe(self->me_mosq, NULL, subscribe, 0);
   WebcfgInfo("mosqwev_subscribe_topic MQTT subscription ret = %d", ret);
   return;
}

/*
 * Reconnect
 */
bool mosqwev_reconnect(mosqwev_t *self)
{
    int rc;

    WebcfgInfo("Reconnecting to: %s:%d:%s", self->me_host, self->me_port, self->me_cid);

    rc = mosquitto_reconnect(self->me_mosq);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        WebcfgError("Reconnection failed: %s:%d:%s: Error %s", self->me_host, self->me_port, self->me_cid,
                 mosquitto_strerror(rc));
        return false;
    }

    return true;
}

/*
 * Disconnect
 */
bool mosqwev_disconnect(mosqwev_t *self)
{
    int rc;

    WebcfgInfo("Disconnecting from: %s:%d:%s", self->me_host, self->me_port, self->me_cid);

    rc = mosquitto_disconnect(self->me_mosq);
    if (rc != MOSQ_ERR_SUCCESS)
    {
       WebcfgError("Re-connection failed: %s:%d:%s: Error %s", self->me_host, self->me_port, self->me_cid,
                 mosquitto_strerror(rc));
        return false;
    }

    return true;
}

/*
 * Returns true whether we have an active connection to the MQTT broker.
 */
bool mosqwev_is_connected(mosqwev_t *self)
{
    return self->me_connected;
}

void WebcfgMqttUninit(void)
{
    if (webcfg_mosquitto_init)
    {
    	mosquitto_lib_cleanup();
    }
    WebcfgInfo("Closing MQTT connection");
}

bool webcfg_mqtt_is_connected()
{
    return mosqwev_is_connected(&webcfg_mqtt);
}
/*
 * Publish a message
 */
bool mosqwev_publish(mosqwev_t *self,
                    int *mid,
                    const char *topic,
                    size_t msglen,
                    void *msg,
                    int qos,
                    bool retain)
{
    int rc;

    rc = mosquitto_publish(self->me_mosq, mid, topic, msglen, msg, qos, retain);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        WebcfgError("Message publish failed: Topic: %s", topic);
        return false;
    }

    mosquitto_loop(self->me_mosq, 0, 1);

    return true;
}

/*
 * Set the connection callback
 */

/*
 * Set the connection callback
 */
void mosqwev_connect_cbk_set(mosqwev_t *self, mosqwev_cbk_t *cbk)
{
    self->me_connect_cbk = cbk;
}

/*
 * Set the disconnect callback
 */
void mosqwev_disconnect_cbk_set(mosqwev_t *self, mosqwev_cbk_t *cbk)
{
    self->me_disconnect_cbk = cbk;
}

/*
 * Set the publish callback
 */
void mosqwev_publish_cbk_set(mosqwev_t *self, mosqwev_cbk_t *cbk)
{
    self->me_publish_cbk = cbk;
}

/*
 * Set the message callback
 */
void mosqwev_message_cbk_set(mosqwev_t *self, mosqwev_message_cbk_t *cbk)
{
    self->me_message_cbk = cbk;
}

/*
 * Set the subscribe callback
 */
void mosqwev_subscribe_cbk_set(mosqwev_t *self, mosqwev_subscribe_cbk_t *cbk)
{
    self->me_subscribe_cbk = cbk;
}

/*
 * Set the subscribe callback
 */
void mosqwev_cbk_unsubscribe_set(mosqwev_t *self, mosqwev_cbk_t *cbk)
{
    self->me_unsubscribe_cbk = cbk;
}

/*int fileread(char *filename, char **data, int *len)
{
   	FILE *fp;
	size_t sz;
	int ch_count = 0;
	fp = fopen(filename, "r+");
	if (fp == NULL)
	{
		WebcfgError("Failed to open file %s\n", filename);
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	ch_count = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	*data = (char *) malloc(sizeof(char) * (ch_count));
	sz = fread(*data, 1, ch_count,fp);
	if (!sz) 
	{	
		fclose(fp);
		WebcfgError("fread failed.\n");
		WEBCFG_FREE(*data);
		return WEBCFG_FAILURE;
	}
	*len = ch_count;
	fclose(fp);
	return 1;

}*/
/*int fileread(char *filename, char **data, int *len)
{
	FILE *fp;
	int ch_count = 0;
	fp = fopen(filename, "r+");
	if (fp == NULL)
	{
		WebcfgError("Failed to open file %s\n", filename);
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	ch_count = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	*data = (char *) malloc(sizeof(char) * (ch_count + 1));
	fread(*data, 1, ch_count-1,fp);
	*len = ch_count;
	WebcfgInfo("ch_count is %d\n", ch_count);
	WebcfgInfo("len %d data len is %ld\n", *len, strlen(*data));
	(*data)[ch_count] ='\0';
	fclose(fp);
	return 1;
}*/
void get_from_file(char *key, char **val)
{
        FILE *fp = fopen(HOST_FILE_LOCATION, "r");

        if (NULL != fp)
        {
                char str[255] = {'\0'};
                while (fgets(str, sizeof(str), fp) != NULL)
                {
                    char *value = NULL;

                    if(NULL != (value = strstr(str, key)))
                    {
                        value = value + strlen(key);
                        value[strlen(value)-1] = '\0';
                        *val = strdup(value);
                        break;
                    }

                }
                fclose(fp);
        }

        if (NULL == *val)
        {
                WebcfgError("WebConfig val is not present in file\n");

        }
        else
        {
                WebcfgInfo("val fetched is %s\n", *val);
        }
}
