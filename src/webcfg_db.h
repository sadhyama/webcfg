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
#include <base64.h>
#include "webcfg.h"
#include "webcfg_multipart.h"
/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
#ifdef BUILD_YOCTO
#define WEBCFG_DB_FILE 	    "/nvram/webconfig_db.bin"
#else
#define WEBCFG_DB_FILE 	    "/tmp/webconfig_db.bin"
#endif
#define WEBCFG_BASE64DB_FILE "/tmp/wifi_base64.bin"
/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/

typedef struct webconfig_tmp_data
{
        char * name;
        uint32_t version;
        char * status;
	char * error_details;
        struct webconfig_tmp_data *next;
} webconfig_tmp_data_t;

typedef struct webconfig_db_data{
	char * name;
	uint32_t version;
        struct webconfig_db_data *next;
}webconfig_db_data_t;

typedef struct blob{
	char *data;
	size_t len;
} blob_t; // will be formed from the struct webconfig_tmp_t and returned in Device.X_RDK_WebConfig.Data as base64 encode

/*For Blob Test purpose*/
typedef struct{
        char * name;
        uint32_t version;
        char * status;
	char * error_details;
}blob_data_t;

typedef struct{
        blob_data_t *entries;
        size_t entries_count;
}blob_struct_t;

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

void webcfgdbblob_destroy( blob_struct_t *bd );
void webcfgdb_destroy( webconfig_db_data_t *pm );
const char* webcfgdbblob_strerror( int errnum );
blob_struct_t* decodeBlobData(const void * buf, size_t len);

WEBCFG_STATUS initDB(char * db_file_path);

WEBCFG_STATUS addNewDocEntry(size_t count);

int writeToDBFile(char * db_file_path, char * data);

WEBCFG_STATUS generateBlob();

blob_t * get_DB_BLOB();

webconfig_db_data_t * get_global_db_node(void);

webconfig_tmp_data_t * get_global_tmp_node(void);

WEBCFG_STATUS addToTmpList( multipart_t *mp);

void addToDBList(webconfig_db_data_t *webcfgdb);

WEBCFG_STATUS updateTmpList(char *docname, uint32_t version, char *status, char *error_details);

WEBCFG_STATUS deleteFromTmpList(char* doc_name);

void delete_tmp_doc_list();

int get_numOfMpDocs();

int get_successDocCount();

char * get_DB_BLOB_base64();

void checkDBList(char *docname, uint32_t version);

WEBCFG_STATUS updateDBlist(char *docname, uint32_t version);

int writebase64ToDBFile(char *base64_file_path, char *data);

char * base64blobencoder(char * blob_data, size_t blob_size );

/**
 *  This function converts a msgpack buffer into an webconfig_db_t structure
 *  if possible.
 *
 *  @param buf the buffer to convert
 *  @param len the length of the buffer in bytes
 *
 *  @return NULL on error, success otherwise
 */
webconfig_db_data_t* decodeData(const void * data, size_t len);


/**
 *  This function returns a general reason why the conversion failed.
 *
 *  @param errnum the errno value to inspect
 *
 *  @return the constant string (do not alter or free) describing the error
 */
const char* webcfgdbparam_strerror( int errnum );

#endif
