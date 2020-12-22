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
#include "webcfg_multipart.h"
#include "webcfg_notify.h"
#include "webcfg_generic.h"
#include "webcfg_param.h"
#include "webcfg_auth.h"
#include <msgpack.h>
#include <wdmp-c.h>
#include <base64.h>
#include "webcfg_db.h"
#include "webcfg_aker.h"
#include "webcfg_metadata.h"
#include "webcfg_event.h"
/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/


#ifdef MULTIPART_UTILITY
#ifdef BUILD_YOCTO
#define TEST_FILE_LOCATION		"/nvram/multipart.bin"
#else
#define TEST_FILE_LOCATION		"/tmp/multipart.bin"
#endif
#endif
#define TELEMETRY_POKE_STR 		"telemetry"
/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
pthread_mutex_t sync_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sync_condition=PTHREAD_COND_INITIALIZER;
bool g_shutdown  = false;
#ifdef MULTIPART_UTILITY
static int g_testfile = 0;
#endif
static int g_supplementarySync = 0;
/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
void *WebConfigMultipartTask(void *status);
int handlehttpResponse(long response_code, char *webConfigData, int retry_count, char* transaction_uuid, char* ct, size_t dataSize);
/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

void initWebConfigMultipartTask(unsigned long status)
{
	int err = 0;
	pthread_t threadId;

	err = pthread_create(&threadId, NULL, WebConfigMultipartTask, (void *) status);
	if (err != 0) 
	{
		WebcfgError("Error creating WebConfigMultipartTask thread :[%s]\n", strerror(err));
	}
	else
	{
		WebcfgInfo("WebConfigMultipartTask Thread created Successfully.\n");
	}
}

void *WebConfigMultipartTask(void *status)
{
	pthread_detach(pthread_self());
	int rv=0;
	int forced_sync=0;
        int Status = 0;
	int retry_flag = 0;
	char* docname[64] = {0};
	char *syncDoc = NULL;
	int i = 0;
	struct timespec ts;
	Status = (unsigned long)status;

	initWebcfgProperties(WEBCFG_PROPERTIES_FILE);

	//start webconfig notification thread.
	initWebConfigNotifyTask();
	initWebConfigClient();
	WebcfgInfo("initDB %s\n", WEBCFG_DB_FILE);

	initDB(WEBCFG_DB_FILE);

	//For Primary sync set flag to 0
	set_global_supplementarySync(0);
	processWebconfgSync((int)Status, NULL);

	//For supplementary sync set flag to 1
	set_global_supplementarySync(1);

	*docname = getSupplementaryUrls();
	int docs_size = sizeof(docname)/sizeof(*docname);

	for(i=0; i<docs_size; i++)
	{
		if(docname[i] != NULL)
		{
			WebcfgInfo("Supplementary sync for %s\n",docname[i]);
			processWebconfgSync((int)Status, docname[i]);
		}
		else
		{
			break;
		}
	}

	//Resetting the supplementary sync
	set_global_supplementarySync(0);

	while(1)
	{
		if(forced_sync)
		{
			WebcfgInfo("Triggered Forced sync\n");
			processWebconfgSync((int)Status, syncDoc);
			WebcfgDebug("reset forced_sync after sync\n");
			forced_sync = 0;
			if(syncDoc !=NULL)
			{
				WebcfgInfo("free syncDoc\n");
				WEBCFG_FREE(syncDoc);
				syncDoc = NULL;
			}
			setForceSync("", "", 0);
			WebcfgInfo("reset global_supplementarySync\n");
			set_global_supplementarySync(0);
		}

		pthread_mutex_lock (&sync_mutex);

		if (g_shutdown)
		{
			WebcfgInfo("g_shutdown is %d, proceeding to kill webconfig thread\n", g_shutdown);
			pthread_mutex_unlock (&sync_mutex);
			break;
		}

		retry_flag = get_doc_fail();
		WebcfgDebug("The retry flag value is %d\n", retry_flag);
		if (retry_flag == 1)
		{
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += 900;

			WebcfgDebug("B4 sync_condition pthread_cond_timedwait\n");
			rv = pthread_cond_timedwait(&sync_condition, &sync_mutex, &ts);
			WebcfgInfo("The retry flag value is %d\n", get_doc_fail());
			WebcfgInfo("The value of rv %d\n", rv);
		}
		else 
		{
			rv = pthread_cond_wait(&sync_condition, &sync_mutex);
		}

		if(rv == ETIMEDOUT && get_doc_fail() == 1)
		{
			WebcfgDebug("Inside the timedout condition\n");
			set_doc_fail(0);
			failedDocsRetry();
			WebcfgDebug("After the failedDocsRetry\n");
		}
		else if(!rv && !g_shutdown)
		{
			char *ForceSyncDoc = NULL;
			char* ForceSyncTransID = NULL;

			// Identify ForceSync based on docname
			getForceSync(&ForceSyncDoc, &ForceSyncTransID);
			WebcfgDebug("ForceSyncDoc %s ForceSyncTransID. %s\n", ForceSyncDoc, ForceSyncTransID);
			if(ForceSyncTransID !=NULL)
			{
				if((ForceSyncDoc != NULL) && strlen(ForceSyncDoc)>0)
				{
					forced_sync = 1;
					WebcfgDebug("Received signal interrupt to Force Sync\n");

					//To check telemetry poke string received and perform secondary doc sync.
					if(strcmp(ForceSyncDoc, TELEMETRY_POKE_STR) == 0) //TODO: IsSupplementaryDoc() check is required here and avoid hardcoding for telemetry string.
					{
						WebcfgInfo("Received supplementary poke request for %s\n", TELEMETRY_POKE_STR);
						set_global_supplementarySync(1);
						ForceSyncDoc[0] = toupper(ForceSyncDoc[0]);
						syncDoc = strdup(ForceSyncDoc);
						WebcfgInfo("syncDoc in upper case is %s\n", syncDoc);
					}
					WEBCFG_FREE(ForceSyncDoc);
					WEBCFG_FREE(ForceSyncTransID);
				}
				else
				{
					WebcfgError("ForceSyncDoc is NULL\n");
					WEBCFG_FREE(ForceSyncTransID);
				}
			}

			WebcfgDebug("forced_sync is %d\n", forced_sync);
		}
		else if(g_shutdown)
		{
			WebcfgInfo("Received signal interrupt to RFC disable. g_shutdown is %d, proceeding to kill webconfig thread\n", g_shutdown);
			pthread_mutex_unlock (&sync_mutex);
			break;
		}
		
		pthread_mutex_unlock(&sync_mutex);

	}

	/* release all active threads before shutdown */
	pthread_mutex_lock (get_global_client_mut());
	pthread_cond_signal (get_global_client_con());
	pthread_mutex_unlock (get_global_client_mut());

	pthread_mutex_lock (get_global_notify_mut());
	pthread_cond_signal (get_global_notify_con());
	pthread_mutex_unlock (get_global_notify_mut());


	if(get_global_eventFlag())
	{
		pthread_mutex_lock (get_global_event_mut());
		pthread_cond_signal (get_global_event_con());
		pthread_mutex_unlock (get_global_event_mut());

		WebcfgDebug("event process thread: pthread_join\n");
		JoinThread (get_global_process_threadid());

		WebcfgDebug("event thread: pthread_join\n");
		JoinThread (get_global_event_threadid());
	}

	WebcfgDebug("notify thread: pthread_join\n");
	JoinThread (get_global_notify_threadid());

	WebcfgDebug("client thread: pthread_join\n");
	JoinThread (get_global_client_threadid());

	reset_global_eventFlag();
	set_doc_fail( 0);
	reset_successDocCount();

	//delete tmp, db, and mp cache lists.
	delete_tmp_doc_list();

	WebcfgDebug("webcfgdb_destroy\n");
	webcfgdb_destroy (get_global_db_node() );

	WebcfgDebug("multipart_destroy\n");
	multipart_destroy(get_global_mp());

	WebcfgInfo("B4 pthread_exit\n");
	pthread_exit(0);
	WebcfgDebug("After pthread_exit\n");
	return NULL;
}

pthread_cond_t *get_global_sync_condition(void)
{
    return &sync_condition;
}

pthread_mutex_t *get_global_sync_mutex(void)
{
    return &sync_mutex;
}

bool get_global_shutdown()
{
    return g_shutdown;
}

void set_global_shutdown(bool shutdown)
{
    g_shutdown = shutdown;
}

#ifdef MULTIPART_UTILITY
int get_g_testfile()
{
    return g_testfile;
}

void set_g_testfile(int value)
{
    g_testfile = value;
}
#endif

void set_global_supplementarySync(int value)
{
    g_supplementarySync = value;
}

int get_global_supplementarySync()
{
    return g_supplementarySync;
}

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
void processWebconfgSync(int status, char* docname)
{
	int retry_count=0;
	int r_count=0;
	int configRet = -1;
	char *webConfigData = NULL;
	long res_code;
	int rv=0;
	char *transaction_uuid =NULL;
	char ct[256] = {0};
	size_t dataSize=0;

	WebcfgDebug("========= Start of processWebconfgSync =============\n");
	while(1)
	{
		#ifdef MULTIPART_UTILITY
		if(testUtility()==1)
		{
			break;
		}
		#endif

		if(retry_count >3)
		{
			WebcfgInfo("Webcfg curl retry to server has reached max limit. Exiting.\n");
			retry_count=0;
			break;
		}
		configRet = webcfg_http_request(&webConfigData, r_count, status, &res_code, &transaction_uuid, ct, &dataSize, docname);
		if(configRet == 0)
		{
			rv = handlehttpResponse(res_code, webConfigData, retry_count, transaction_uuid, ct, dataSize);
			if(rv ==1)
			{
				WebcfgDebug("No curl retries are required. Exiting..\n");
				break;
			}
		}
		else
		{
			WebcfgError("Failed to get webConfigData from cloud\n");
			//WEBCFG_FREE(transaction_uuid);
		}
		WebcfgInfo("webcfg_http_request BACKOFF_SLEEP_DELAY_SEC is %d seconds\n", BACKOFF_SLEEP_DELAY_SEC);
		sleep(BACKOFF_SLEEP_DELAY_SEC);
		retry_count++;
		if(retry_count <= 3)
		{
			WebcfgInfo("Webconfig curl retry_count to server is %d\n", retry_count);
		}
	}
	WebcfgDebug("========= End of processWebconfgSync =============\n");
	return;
}

int handlehttpResponse(long response_code, char *webConfigData, int retry_count, char* transaction_uuid, char *ct, size_t dataSize)
{
	int first_digit=0;
	int msgpack_status=0;
	int err = 0;
	char version[512]={'\0'};
	uint32_t db_root_version = 0;
	char *db_root_string = NULL;
	int subdocList = 0;
	char *contentLength = NULL;

	if(response_code == 304)
	{
		WebcfgInfo("webConfig is in sync with cloud. response_code:%ld\n", response_code);
		getRootDocVersionFromDBCache(&db_root_version, &db_root_string, &subdocList);
		addWebConfgNotifyMsg(NULL, db_root_version, NULL, NULL, transaction_uuid, 0, "status", 0, db_root_string, response_code);
		if(db_root_string !=NULL)
		{
			WEBCFG_FREE(db_root_string);
		}
		WEBCFG_FREE(transaction_uuid);
		return 1;
	}
	else if(response_code == 200)
	{
		WebcfgInfo("webConfig is not in sync with cloud. response_code:%ld\n", response_code);

		if(webConfigData !=NULL && (strlen(webConfigData)>0))
		{
			WebcfgDebug("webConfigData fetched successfully\n");
			WebcfgDebug("parseMultipartDocument\n");
			msgpack_status = parseMultipartDocument(webConfigData, ct, dataSize, transaction_uuid);

			if(msgpack_status == WEBCFG_SUCCESS)
			{
				WebcfgInfo("webConfigData applied successfully\n");
				return 1;
			}
			else
			{
				WebcfgDebug("root webConfigData processed, check apply status events\n");
				return 1;
			}
		}
		else
		{
			WebcfgInfo("webConfigData is empty\n");
			//After factory reset when server sends 200 with empty config, set POST-NONE root version
			contentLength = get_global_contentLen();
			if((contentLength !=NULL) && (strcmp(contentLength, "0") == 0))
			{
				WebcfgInfo("webConfigData content length is 0\n");
				refreshConfigVersionList(version, response_code);
				WEBCFG_FREE(contentLength);
				WEBCFG_FREE(transaction_uuid);
				WEBCFG_FREE(webConfigData);
				return 1;
			}
		}
	}
	else if(response_code == 204)
	{
		WebcfgInfo("No configuration available for this device. response_code:%ld\n", response_code);
		getRootDocVersionFromDBCache(&db_root_version, &db_root_string, &subdocList);
		addWebConfgNotifyMsg(NULL, db_root_version, NULL, NULL, transaction_uuid, 0, "status", 0, db_root_string, response_code);
		if(db_root_string !=NULL)
		{
			WEBCFG_FREE(db_root_string);
		}
		WEBCFG_FREE(transaction_uuid);
		return 1;
	}
	else if(response_code == 403)
	{
		WebcfgError("Token is expired, fetch new token. response_code:%ld\n", response_code);
		createNewAuthToken(get_global_auth_token(), TOKEN_SIZE, get_deviceMAC(), get_global_serialNum() );
		WebcfgDebug("createNewAuthToken done in 403 case\n");
		err = 1;
	}
	else if(response_code == 429)
	{
		WebcfgInfo("No action required from client. response_code:%ld\n", response_code);
		getRootDocVersionFromDBCache(&db_root_version, &db_root_string, &subdocList);
		addWebConfgNotifyMsg(NULL, db_root_version, NULL, NULL, transaction_uuid, 0, "status", 0, db_root_string, response_code);
		if(db_root_string !=NULL)
		{
			WEBCFG_FREE(db_root_string);
		}
		WEBCFG_FREE(transaction_uuid);
		return 1;
	}
	first_digit = (int)(response_code / pow(10, (int)log10(response_code)));
	if((response_code !=403) && (first_digit == 4)) //4xx
	{
		WebcfgInfo("Action not supported. response_code:%ld\n", response_code);
		if (response_code == 404)
		{
			//To set POST-NONE root version when 404
			refreshConfigVersionList(version, response_code);
		}
		getRootDocVersionFromDBCache(&db_root_version, &db_root_string, &subdocList);
		addWebConfgNotifyMsg(NULL, db_root_version, NULL, NULL, transaction_uuid, 0, "status", 0, db_root_string, response_code);
		if(db_root_string !=NULL)
		{
			WEBCFG_FREE(db_root_string);
		}
		WEBCFG_FREE(transaction_uuid);
		return 1;
	}
	else //5xx & all other errors
	{
		WebcfgError("Error code returned, need to do curl retry to server. response_code:%ld\n", response_code);
		if(retry_count == 3 && !err)
		{
			WebcfgDebug("3 curl retry attempts\n");
			getRootDocVersionFromDBCache(&db_root_version, &db_root_string, &subdocList);
			addWebConfgNotifyMsg(NULL, db_root_version, NULL, NULL, transaction_uuid, 0, "status", 0, db_root_string, response_code);
			if(db_root_string !=NULL)
			{
				WEBCFG_FREE(db_root_string);
			}
			WEBCFG_FREE(transaction_uuid);
			return 0;
		}
		WEBCFG_FREE(transaction_uuid);
	}
	return 0;
}

void webcfgStrncpy(char *destStr, const char *srcStr, size_t destSize)
{
    strncpy(destStr, srcStr, destSize-1);
    destStr[destSize-1] = '\0';
}

void getCurrent_Time(struct timespec *timer)
{
	clock_gettime(CLOCK_REALTIME, timer);
}

long timeVal_Diff(struct timespec *starttime, struct timespec *finishtime)
{
	long msec;
	msec=(finishtime->tv_sec-starttime->tv_sec)*1000;
	msec+=(finishtime->tv_nsec-starttime->tv_nsec)/1000000;
	return msec;
}

#ifdef MULTIPART_UTILITY
int testUtility()
{
	char *data = NULL;
	char command[30] = {0};
	int test_dataSize = 0;
	int test_file_status = 0;

	char *transaction_uuid =NULL;
	char ct[256] = {0};

	if(readFromFile(TEST_FILE_LOCATION, &data, &test_dataSize) == 1)
	{
		set_g_testfile(1);
		WebcfgInfo("Using Test file \n");

		char * data_body = malloc(sizeof(char) * test_dataSize+1);
		memset(data_body, 0, sizeof(char) * test_dataSize+1);
		data_body = memcpy(data_body, data, test_dataSize+1);
		data_body[test_dataSize] = '\0';
		char *ptr_count = data_body;
		char *ptr1_count = data_body;
		char *temp = NULL;

		while((ptr_count - data_body) < test_dataSize )
		{
			ptr_count = memchr(ptr_count, 'C', test_dataSize - (ptr_count - data_body));
			if(0 == memcmp(ptr_count, "Content-type:", strlen("Content-type:")))
			{
				ptr1_count = memchr(ptr_count+1, '\r', test_dataSize - (ptr_count - data_body));
				temp = strndup(ptr_count, (ptr1_count-ptr_count));
				strncpy(ct,temp,(sizeof(ct)-1));
				break;
			}

			ptr_count++;
		}
		free(data_body);
		free(temp);
		transaction_uuid = strdup(generate_trans_uuid());
		if(data !=NULL)
		{
			WebcfgInfo("webConfigData fetched successfully\n");
			WebcfgInfo("parseMultipartDocument\n");
			test_file_status = parseMultipartDocument(data, ct, (size_t)test_dataSize, transaction_uuid);

			if(test_file_status == WEBCFG_SUCCESS)
			{
				WebcfgInfo("Test webConfigData applied successfully\n");
			}
			else
			{
				WebcfgError("Failed to apply Test root webConfigData received from server\n");
			}
		}
		else
		{
			WEBCFG_FREE(transaction_uuid);
			WebcfgError("webConfigData is empty, need to retry\n");
		}
		sprintf(command,"rm -rf %s",TEST_FILE_LOCATION);
		if(-1 != system(command))
		{
			WebcfgInfo("The %s file is removed successfully\n",TEST_FILE_LOCATION);
		}
		else
		{
			WebcfgError("Error in removing %s file\n",TEST_FILE_LOCATION);
		}
		return 1;
	}
	else
	{
		set_g_testfile(0);
		return 0;
	}
}
#endif

void JoinThread (pthread_t threadId)
{
	int ret = 0;
	ret = pthread_join (threadId, NULL);
	if(ret ==0)
	{
		WebcfgInfo("pthread_join returns success\n");
	}
	else
	{
		WebcfgError("Error joining thread threadId\n");
	}
}
