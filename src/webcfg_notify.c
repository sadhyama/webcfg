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

#include "webcfg_multipart.h"
#include "webcfg_notify.h"
#include "webcfg_generic.h"
#include "webcfg.h"
#include <pthread.h>
#include <cJSON.h>
#include <unistd.h>
/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
static pthread_t NotificationThreadId=0;
pthread_mutex_t notify_mut=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t notify_con=PTHREAD_COND_INITIALIZER;
notify_params_t *notifyMsgQ = NULL;
/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
void* processWebConfgNotification();
void free_notify_params_struct(notify_params_t *param);
/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
pthread_t get_global_notify_threadid()
{
    return NotificationThreadId;
}

//To handle webconfig notification tasks
void initWebConfigNotifyTask()
{
	int err = 0;
	WebConfigLog("Inside initWebConfigNotifyTask\n");
	err = pthread_create(&NotificationThreadId, NULL, processWebConfgNotification, NULL);
	if (err != 0)
	{
		WebConfigLog("Error creating Webconfig Notification thread :[%s]\n", strerror(err));
	}
	else
	{
		WebConfigLog("Webconfig Notification thread created Successfully\n");
	}

}

void addWebConfgNotifyMsg(char *docname, uint32_t version, char *status, char *error_details, char *transaction_uuid)
{
	notify_params_t *args = NULL;

	args = (notify_params_t *)malloc(sizeof(notify_params_t));
	char versionStr[32] = {'\0'};

	if(args)
	{
		memset(args, 0, sizeof(notify_params_t));
		if(docname != NULL)
		{
			args->name = strdup(docname);
		}

		if(status != NULL)
		{
			args->application_status = strdup(status);
		}

		if(error_details != NULL)
		{
			args->error_details = strdup(error_details);
		}

		snprintf(versionStr, sizeof(versionStr), "%lu", (long)version);

		if(strlen(versionStr) > 0)
		{
			args->version = strdup(versionStr);
		}

		if(transaction_uuid != NULL)
		{
			args->transaction_uuid = strdup(transaction_uuid);
		}

		WebConfigLog("args->name:%s,args->application_status:%s,args->error_details:%s,args->version:%s,args->transaction_uuid:%s\n",args->name,args->application_status, args->error_details, args->version, args->transaction_uuid );

		args->next=NULL;

		pthread_mutex_lock (&notify_mut);
		//Producer adds the notifyMsg into queue
		if(notifyMsgQ == NULL)
		{
			notifyMsgQ = args;
			WebConfigLog("Producer added notify message\n");
			pthread_cond_signal(&notify_con);
			pthread_mutex_unlock (&notify_mut);
			WebConfigLog("mutex unlock in notify producer thread\n");
		}
		else
		{
			notify_params_t *temp = notifyMsgQ;
			while(temp->next)
			{
			    temp = temp->next;
			}
			temp->next = args;
			pthread_mutex_unlock (&notify_mut);
		}
	}
	else
	{
	    WebConfigLog("failure in allocation for notify message\n");
	}
}

//Notify thread function waiting for notify msgs
void* processWebConfgNotification()
{
	pthread_detach(pthread_self());
	char device_id[32] = { '\0' };
	cJSON *notifyPayload = NULL;
	char  * stringifiedNotifyPayload = NULL;
	char dest[512] = {'\0'};
	char *source = NULL;

	while(1)
	{
		pthread_mutex_lock (&notify_mut);
		WebConfigLog("mutex lock in notify consumer thread\n");
		if(notifyMsgQ != NULL)
		{
			notify_params_t *msg = notifyMsgQ;
			notifyMsgQ = notifyMsgQ->next;
			pthread_mutex_unlock (&notify_mut);
			WebConfigLog("mutex unlock in notify consumer thread\n");

			if((get_deviceMAC() !=NULL) && (strlen(get_deviceMAC()) == 0))
			{
				WebConfigLog("deviceMAC is NULL, failed to send Webconfig Notification\n");
			}
			else
			{
				snprintf(device_id, sizeof(device_id), "mac:%s", get_deviceMAC());
				WebConfigLog("webconfig Device_id %s\n", device_id);

				notifyPayload = cJSON_CreateObject();

				if(notifyPayload != NULL)
				{
					cJSON_AddStringToObject(notifyPayload,"device_id", device_id);

					if(msg)
					{
						cJSON_AddStringToObject(notifyPayload,"namespace", (NULL != msg->name && (strlen(msg->name)!=0)) ? msg->name : "unknown");
						cJSON_AddStringToObject(notifyPayload,"application_status", (NULL != msg->application_status) ? msg->application_status : "unknown");
						if((msg->error_details !=NULL) && (strcmp(msg->error_details, "none")!=0))
						{
							cJSON_AddStringToObject(notifyPayload,"error_details", (NULL != msg->error_details) ? msg->error_details : "unknown");
						}
						cJSON_AddStringToObject(notifyPayload,"transaction_uuid", (NULL != msg->transaction_uuid && (strlen(msg->transaction_uuid)!=0)) ? msg->transaction_uuid : "unknown");
						cJSON_AddStringToObject(notifyPayload,"version", (NULL != msg->version && (strlen(msg->version)!=0)) ? msg->version : "NONE");
					
					}
					stringifiedNotifyPayload = cJSON_PrintUnformatted(notifyPayload);
					cJSON_Delete(notifyPayload);
				}

				snprintf(dest,sizeof(dest),"event:subdoc-report/%s/%s/status",msg->name,device_id);
				WebConfigLog("dest is %s\n", dest);

				if (stringifiedNotifyPayload != NULL && strlen(device_id) != 0)
				{
					source = (char*) malloc(sizeof(char) * sizeof(device_id));
					strncpy(source, device_id, sizeof(device_id));
					WebConfigLog("source is %s\n", source);
					WebConfigLog("stringifiedNotifyPayload is %s\n", stringifiedNotifyPayload);
					//stringifiedNotifyPayload, source to be freed by sendNotification
					sendNotification(stringifiedNotifyPayload, source, dest);
				}
				if(msg != NULL)
				{
					free_notify_params_struct(msg);
					msg = NULL;
				}
			}
			pthread_mutex_unlock (&notify_mut);
		}
		else
		{
			WebConfigLog("Before pthread cond wait in notify thread\n");
			pthread_cond_wait(&notify_con, &notify_mut);
			pthread_mutex_unlock (&notify_mut);
			WebConfigLog("mutex unlock in notify thread after cond wait\n");
		}
	}
	return NULL;
}

void free_notify_params_struct(notify_params_t *param)
{
    if(param != NULL)
    {
        if(param->name != NULL)
        {
            WEBCFG_FREE(param->name);
        }
        if(param->application_status != NULL)
        {
            WEBCFG_FREE(param->application_status);
        }
	if(param->error_details != NULL)
        {
            WEBCFG_FREE(param->error_details);
        }
        if(param->version != NULL)
        {
            WEBCFG_FREE(param->version);
        }
	if(param->transaction_uuid != NULL)
        {
	    WEBCFG_FREE(param->transaction_uuid);
        }
        WEBCFG_FREE(param);
    }
}

