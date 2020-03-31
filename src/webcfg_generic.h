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
#ifndef __WEBCFGGENERIC_H__
#define __WEBCFGGENERIC_H__

#include <stdint.h>
#include <wdmp-c.h>
#include <msgpack.h>
/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/

#define WEBCFG_BLOB_PATH                      "/tmp/webcfg_blob.bin"
/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
/* return logger file name */
const char *fetch_generic_file(void);
int setForceSync(char* pString, char *transactionId,int *session_status);
int getForceSync(char** pString, char **transactionId);

char * get_DB_BLOB_base64(size_t *len);
char *get_global_systemReadyTime();
char* getDeviceBootTime();
char * getSerialNumber();
char * getProductClass();
char * getModelName();
char * getFirmwareVersion();
void getDeviceMac();
char* get_global_deviceMAC();

void initWebConfigMultipartTask();
void sendNotification(char *payload, char *source, char *destination);

int Get_Webconfig_Blob( char *pString);
int Get_Webconfig_URL( char **pString);
int Set_Webconfig_URL( char *pString);

/*For Blob Test purpose*/
typedef struct{
        char * name;
        uint32_t version;
        char * status;        
}blob_data_t;

typedef struct{
        blob_data_t *entries;
        size_t entries_count;
}blob_struct_t;

int readBlobFromFile(char * blob_file_path);
void webcfgdbblob_destroy( blob_struct_t *bd );
const char* webcfgdbblob_strerror( int errnum );
int process_webcfgdbblob( blob_struct_t *bd, msgpack_object *obj );
int process_webcfgdbblobparams( blob_data_t *e, msgpack_object_map *map );
blob_struct_t* decodeBlobData(const void * buf, size_t len);
int writeBlobToFile(char *blob_file_path, char *data);

/**
 * @brief setValues interface sets the parameter value.
 *
 * @param[in] paramVal List of Parameter name/value pairs.
 * @param[in] paramCount Number of parameters.
 * @param[in] setType enum to specify the type of set operation. 
 * WEBPA_SET = 0 for set values, 1 for atomic set, 2 for XPC atomic set.
 * @param[out] timeSpan timing_values for each component. 
 * @param[out] retStatus Returns status
 * @param[out] ccspStatus Returns ccsp set status
 */
void setValues(const param_t paramVal[], const unsigned int paramCount, const int setType, char *transactionId, money_trace_spans *timeSpan, WDMP_STATUS *retStatus, int *ccspStatus);

#endif
