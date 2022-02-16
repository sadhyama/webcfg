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
#ifndef __WEBCFG_H__
#define __WEBCFG_H__

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
#define MAX_BUF_SIZE	           256
#define MAX_PARAMETERNAME_LENGTH       512
#define BACKOFF_SLEEP_DELAY_SEC 	    10

#ifdef BUILD_YOCTO
#define DEVICE_PROPS_FILE       "/etc/device.properties"
#else
#define DEVICE_PROPS_FILE       "/tmp/device.properties"
#endif

#define WEBCFG_FREE(__x__) if(__x__ != NULL) { free((void*)(__x__)); __x__ = NULL;} else {printf("Trying to free null pointer\n");}
#ifndef TEST
#define FOREVER()   1
#else
extern int numLoops;
#define FOREVER()   numLoops--
#endif

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
typedef enum
{
    WEBCFG_SUCCESS = 0,
    WEBCFG_FAILURE
} WEBCFG_STATUS;

typedef enum
{
	DECODE_ROOT_FAILURE,
	INCORRECT_BLOB_TYPE,
	BLOB_PARAM_VALIDATION_FAILURE,
	WEBCONFIG_DATA_EMPTY,
	MULTIPART_BOUNDARY_NULL,
	INVALID_CONTENT_TYPE,
	ADD_TO_CACHE_LIST_FAILURE,
	FAILED_TO_SET_BLOB,
	MULTIPART_CACHE_NULL,
	AKER_SUBDOC_PROCESSING_FAILED,
	AKER_RESPONSE_PARSE_FAILURE,
	INVALID_AKER_RESPONSE,
	LIBPARODUS_RECEIVE_FAILURE,
	COMPONENT_EVENT_PARSE_FAILURE,
	SUBDOC_RETRY_FAILED
} WEBCFG_ERROR_CODE;
/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
#ifdef MULTIPART_UTILITY
int testUtility();
int get_g_testfile();
void set_g_testfile(int value);
#endif

bool get_global_shutdown();
void set_global_shutdown(bool shutdown);
bool get_bootSync();
void set_bootSync(bool bootsync);
bool get_maintenanceSync();
void set_maintenanceSync(bool maintenancesync);
pthread_cond_t *get_global_sync_condition(void);
pthread_mutex_t *get_global_sync_mutex(void);
pthread_t *get_global_mpThreadId(void);
int get_global_supplementarySync();
void set_global_supplementarySync(int value);

void initWebConfigMultipartTask(unsigned long status);
void processWebconfgSync(int Status, char* docname);
WEBCFG_STATUS webcfg_http_request(char **configData, int r_count, int status, long *code, char **transaction_id,char* contentType, size_t* dataSize, char* docname);

void webcfgStrncpy(char *destStr, const char *srcStr, size_t destSize);

char* get_global_auth_token();
void getCurrent_Time(struct timespec *timer);
long timeVal_Diff(struct timespec *starttime, struct timespec *finishtime);
void initWebConfigClient();
pthread_t get_global_client_threadid();
void JoinThread (pthread_t threadId);
#endif
