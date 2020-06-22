/*
 * Copyright 2020 Comcast Cable Communications Management, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include "webcfg.h"
#include "webcfg_log.h"
#include "webcfg_client.h"
#include <wrp-c.h>
#include <wdmp-c.h>
#include <libparodus.h>
/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
#define CONTENT_TYPE_JSON       "application/json"
#define PARODUS_URL_DEFAULT      "tcp://127.0.0.1:6666"
#define CLIENT_URL_DEFAULT       "tcp://127.0.0.1:6659"
/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
libpd_instance_t current_instance;
/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
static void webConfigClientReceive();
static void connect_parodus();
static void webConfigClientReceive();
static void *parodus_receive();
static void get_parodus_url(char **parodus_url, char **client_url);
/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
void initWebConfigClient()
{
	connect_parodus();
	webConfigClientReceive();
}

int send_aker_blob(char *blob, char *transId)
{
	WebcfgInfo("Aker blob is %s\n",blob);
	wrp_msg_t *msg = NULL;
	int ret = WDMP_FAILURE;
	msg = (wrp_msg_t *)malloc(sizeof(wrp_msg_t));
	if(msg != NULL)
	{
		memset(msg, 0, sizeof(wrp_msg_t));
		msg->msg_type = WRP_MSG_TYPE__UPDATE;
		msg->u.crud.payload = strdup(blob);
		msg->u.crud.payload_size = strlen(msg->u.crud.payload);
		msg->u.crud.source = strdup("mac:14cfe2xxxxxx/webcfg");
		msg->u.crud.dest = strdup("mac:14cfe2xxxxxx/aker/schedule");
		msg->u.crud.transaction_uuid = strdup(transId);
		msg->u.crud.content_type = strdup(CONTENT_TYPE_JSON);
		int sendStatus = libparodus_send(current_instance, msg);
		WebcfgDebug("sendStatus is %d\n",sendStatus);
		if(sendStatus == 0)
		{
			WebcfgInfo("Sent message successfully to parodus\n");
			ret = WDMP_SUCCESS;
		}
		else
		{
			WebcfgError("Failed to send message: '%s'\n",libparodus_strerror(sendStatus));
		}
		wrp_free_struct (msg);
	}
	return ret;
}
/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
static void webConfigClientReceive()
{
	int err = 0;
	pthread_t threadId;

	err = pthread_create(&threadId, NULL, parodus_receive, NULL);
	if (err != 0) 
	{
		WebcfgError("Error creating webConfigClientReceive thread :[%s]\n", strerror(err));
	}
	else
	{
		WebcfgInfo("webConfigClientReceive Thread created Successfully.\n");
	}
}

static void connect_parodus()
{
	int backoffRetryTime = 0;
	int backoff_max_time = 5;
	int max_retry_sleep;
	//Retry Backoff count shall start at c=2 & calculate 2^c - 1.
	int c =2;
	int retval=-1;
	char *parodus_url = NULL;
	char *client_url = NULL;

	max_retry_sleep = (int) pow(2, backoff_max_time) -1;
	WebcfgInfo("max_retry_sleep is %d\n", max_retry_sleep );

	get_parodus_url(&parodus_url, &client_url);

	if(parodus_url != NULL && client_url != NULL)
	{	
		libpd_cfg_t cfg1 = {.service_name = "webcfg",
							.receive = true, .keepalive_timeout_secs = 64,
							.parodus_url = parodus_url,
							.client_url = client_url
						   };

		WebcfgDebug("libparodus_init with parodus url %s and client url %s\n",cfg1.parodus_url,cfg1.client_url);

		while(1)
		{
			if(backoffRetryTime < max_retry_sleep)
			{
				backoffRetryTime = (int) pow(2, c) -1;
			}
			WebcfgDebug("New backoffRetryTime value calculated as %d seconds\n", backoffRetryTime);
			int ret =libparodus_init (&current_instance, &cfg1);
			WebcfgDebug("ret is %d\n",ret);
			if(ret ==0)
			{
				WebcfgInfo("Init for parodus Success..!!\n");
				break;
			}
			else
			{
				WebcfgError("Init for parodus failed: '%s'\n",libparodus_strerror(ret));
				sleep(backoffRetryTime);
				c++;

				if(backoffRetryTime == max_retry_sleep)
				{
					c = 2;
					backoffRetryTime = 0;
					WebcfgDebug("backoffRetryTime reached max value, reseting to initial value\n");
				}
			}
			retval = libparodus_shutdown(&current_instance);
			WebcfgDebug("libparodus_shutdown retval %d\n", retval);
		}
	}
}
	
void* parodus_receive()
{
	int rtn;
	wrp_msg_t *wrp_msg;
	wrp_msg_t *res_wrp_msg ;
	char *contentType = NULL;
	char *sourceService, *sourceApplication =NULL;

	pthread_detach(pthread_self());
	while(1)
	{
		rtn = libparodus_receive (current_instance, &wrp_msg, 2000);
		if (rtn == 1)
		{
			continue;
		}

		if (rtn != 0)
		{
			WebcfgError ("Libparodus failed to recieve message: '%s'\n",libparodus_strerror(rtn));
			sleep(5);
			continue;
		}

		if(wrp_msg != NULL)
		{
			printf("WebConfig: Message received with type %d\n",wrp_msg->msg_type);
			if (wrp_msg->msg_type == WRP_MSG_TYPE__REQ)
			{
				res_wrp_msg = (wrp_msg_t *)malloc(sizeof(wrp_msg_t));
				if(res_wrp_msg != NULL)
				{
					memset(res_wrp_msg, 0, sizeof(wrp_msg_t));
					if(res_wrp_msg->u.req.payload !=NULL)
					{   
						WebcfgDebug("Response payload is %s\n",(char *)(res_wrp_msg->u.req.payload));
						res_wrp_msg->u.req.payload_size = strlen(res_wrp_msg->u.req.payload);
					}
					res_wrp_msg->msg_type = wrp_msg->msg_type;
					if(wrp_msg->u.req.dest != NULL)
					{
						res_wrp_msg->u.req.source = strdup(wrp_msg->u.req.dest);
					}
					if(wrp_msg->u.req.source != NULL)
					{
						res_wrp_msg->u.req.dest = strdup(wrp_msg->u.req.source);
					}
					if(wrp_msg->u.req.transaction_uuid != NULL)
					{
						res_wrp_msg->u.req.transaction_uuid = strdup(wrp_msg->u.req.transaction_uuid);
					}
					contentType = strdup(CONTENT_TYPE_JSON);
					if(contentType != NULL)
					{
						res_wrp_msg->u.req.content_type = contentType;
					}
					int sendStatus = libparodus_send(current_instance, res_wrp_msg);
					WebcfgDebug("sendStatus is %d\n",sendStatus);
					if(sendStatus == 0)
					{
						WebcfgInfo("Sent message successfully to parodus\n");
					}
					else
					{
						WebcfgError("Failed to send message: '%s'\n",libparodus_strerror(sendStatus));
					}
					wrp_free_struct (res_wrp_msg);
				}
				wrp_free_struct (wrp_msg);
			}
			//handle cloud-status retrieve response received from parodus
			else if (wrp_msg->msg_type == WRP_MSG_TYPE__UPDATE)
			{
				sourceService = wrp_get_msg_element(WRP_ID_ELEMENT__SERVICE, wrp_msg, SOURCE);
				WebcfgInfo("sourceService: %s\n",sourceService);
				sourceApplication = wrp_get_msg_element(WRP_ID_ELEMENT__APPLICATION, wrp_msg, SOURCE);
				WebcfgInfo("sourceApplication: %s\n",sourceApplication);
				//if(sourceService != NULL && sourceApplication != NULL && strcmp(sourceService,"parodus")== 0 && strcmp(sourceApplication,"cloud-status")== 0)
				{
					WebcfgInfo("cloud-status Retrieve response received from parodus : %s len %lu transaction_uuid %s\n",(char *)wrp_msg->u.crud.payload, strlen(wrp_msg->u.crud.payload), wrp_msg->u.crud.transaction_uuid );
				}
				wrp_free_struct (wrp_msg);
			}
		}
	}
	libparodus_shutdown(&current_instance);
	return 0;
}

static void get_parodus_url(char **parodus_url, char **client_url)
{
	FILE *fp = fopen(DEVICE_PROPS_FILE, "r");
	if (NULL != fp)
	{
		char str[255] = {'\0'};
		while(fscanf(fp,"%s", str) != EOF)
		{
			char *value = NULL;
			if(NULL != (value = strstr(str, "PARODUS_URL=")))
			{
				value = value + strlen("PARODUS_URL=");
				*parodus_url = strdup(value);
			}

			if(NULL != (value = strstr(str, "WEBCFG_CLIENT_URL=")))
			{
				value = value + strlen("WEBCFG_CLIENT_URL=");
				*client_url = strdup(value);
			}
		}
		fclose(fp);
	}
	else
	{
		WebcfgError("Failed to open device.properties file:%s\n", DEVICE_PROPS_FILE);
		WebcfgInfo("Adding default values for parodus_url and client_url\n");
		*parodus_url = strdup(PARODUS_URL_DEFAULT);
		*client_url = strdup(CLIENT_URL_DEFAULT);
	}

	if (NULL == *parodus_url)
	{
		WebcfgError("parodus_url is not present in device.properties, adding default parodus_url\n");
		*parodus_url = strdup(PARODUS_URL_DEFAULT);
	}
	else
	{
		WebcfgDebug("parodus_url formed is %s\n", *parodus_url);
	}

	if (NULL == *client_url)
	{
		WebcfgError("client_url is not present in device.properties, adding default client_url\n");
		*client_url = strdup(CLIENT_URL_DEFAULT);
	}
	else
	{
		WebcfgDebug("client_url formed is %s\n", *client_url);
	}
}

