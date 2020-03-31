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
#ifndef __WEBCFG__DB_H__
#define __WEBCFG__DB_H__

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "webcfg_multipart.h"
/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
#define WEBCFG_DB_FILE 	    "/nvram/webconfig_db.bin"
/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/

typedef struct webconfig_tmp_data
{
        char * name;
        uint32_t version;
        char * status;
        struct webconfig_tmp_data *next;
} webconfig_tmp_data_t;

typedef struct {
	webconfig_tmp_data_t *entries;
	size_t      entries_count;
} webconfig_tmp_t; // For storing in memory processing docs "pending", "failed"
// root is also one entry with latest root version recived apply in progress

typedef struct webconfig_db_data{
	char * name;
	uint32_t version;
        struct webconfig_db_data *next;
}webconfig_db_data_t;


typedef struct {
	webconfig_db_data_t *entries;
	size_t      entries_count;
} webconfig_db_t; // For storing the successfully applied docs
// root is also one entry with root version applied

typedef struct blob{
	char *data;
	size_t len;
} blob_t; // will be formed from the struct webconfig_tmp_t and returned in Device.X_RDK_WebConfig.Data as base64 encode
/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

/**
 *  Initialize/Load data from DB file.
 *
 *  @param db_file_path full path name
 *
 *  @return 
 */
int initDB(char * db_file_path);

int addNewDocEntry(size_t count);

webconfig_db_t* getDocEntry(char *doc_name);

int updateDocEntry(char *doc_name, webconfig_db_t *subdoc);

int writeToDBFile(char * db_file_path, char * data);

int generateBlob();

blob_t * get_DB_BLOB();

webconfig_db_data_t * get_global_db_node(void);

webconfig_tmp_data_t * get_global_tmp_node(void);

int addToTmpList( multipart_t *mp);

void addToDBList(webconfig_db_data_t *webcfgdb);

int updateTmpList(char *docname, uint32_t version, char *status);

int deleteFromTmpList(char* doc_name);

void delete_tmp_doc_list();

int get_numOfMpDocs();

int get_successDocCount();

void b64_encoder(const void *buf,size_t len, char ** decodeMsg);

/**
 *  This function converts a msgpack buffer into an webconfig_db_t structure
 *  if possible.
 *
 *  @param buf the buffer to convert
 *  @param len the length of the buffer in bytes
 *
 *  @return NULL on error, success otherwise
 */
webconfig_db_t* decodeData(const void * data, size_t len);

/**
 *  This function destroys an webconfig_db_t object.
 *
 *  @param e the webcfgparam to destroy
 */
void webcfgdb_destroy( webconfig_db_t *d );

/**
 *  This function returns a general reason why the conversion failed.
 *
 *  @param errnum the errno value to inspect
 *
 *  @return the constant string (do not alter or free) describing the error
 */
const char* webcfgdbparam_strerror( int errnum );

extern webconfig_db_t *wd;
extern webconfig_db_data_t* webcfgdb_data;
#endif
