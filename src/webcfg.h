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

int numLoops;
/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
typedef enum
{
    WEBCFG_SUCCESS = 0,
    WEBCFG_FAILURE
} WEBCFG_STATUS;
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
pthread_cond_t *get_global_sync_condition(void);
pthread_mutex_t *get_global_sync_mutex(void);

void initWebConfigMultipartTask(unsigned long status);
void processWebconfgSync(int Status);
WEBCFG_STATUS webcfg_http_request(char **configData, int r_count, int status, long *code, char **transaction_id,char* contentType, size_t* dataSize);

void webcfgStrncpy(char *destStr, const char *srcStr, size_t destSize);

char* get_global_auth_token();
void getCurrent_Time(struct timespec *timer);
long timeVal_Diff(struct timespec *starttime, struct timespec *finishtime);
void initWebConfigClient();
pthread_t get_global_client_threadid();
void JoinThread (pthread_t threadId);
void initRandomTimer();
int checkRandomTimer();
bool check_rand_time();
long long get_global_rand_time();
int secondarySyncSeconds();
void set_global_rand_time(long long value);
bool get_global_secondary_docs();
void set_global_secondary_docs(bool secondary_docs);
#endif
