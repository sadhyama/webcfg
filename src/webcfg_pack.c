/*
 * Copyright 2019 Comcast Cable Communications Management, LLC
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
#include <errno.h>
#include <string.h>
#include <msgpack.h>

#include "webcfg.h"
#include "webcfg_helpers.h"
#include "webcfg_pack.h"
#include "webcfg_log.h"
/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
struct webcfg_token {
    const char *name;
    size_t length;
};
/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
static const struct webcfg_token PAM_PARAMETERS   = { .name = "PublicHotspotData", .length = sizeof( "PublicHotspotData" ) - 1 };
static const struct webcfg_token WEBCFG_DB_PARAMETERS   = { .name = "webcfgdb", .length = sizeof( "webcfgdb" ) - 1 };
static const struct webcfg_token WEBCFG_BLOB_PARAMETERS   = { .name = "webcfgblob", .length = sizeof( "webcfgblob" ) - 1 };
static void __msgpack_pack_string( msgpack_packer *pk, const void *string, size_t n );
static void __msgpack_pack_string_nvp( msgpack_packer *pk,
                                       const struct webcfg_token *token,
                                       const char *val );

static void __msgpack_pack_string( msgpack_packer *pk, const void *string, size_t n )
{
    msgpack_pack_str( pk, n );
    msgpack_pack_str_body( pk, string, n );
}

static void __msgpack_pack_string_nvp( msgpack_packer *pk,
                                       const struct webcfg_token *token,
                                       const char *val )
{
    if( ( NULL != token ) && ( NULL != val ) ) {
        __msgpack_pack_string( pk, token->name, token->length );
        __msgpack_pack_string( pk, val, strlen( val ) );
    }
}


ssize_t webcfgdb_blob_pack(webconfig_db_data_t *webcfgdb, webconfig_tmp_data_t * webcfgtemp, void **data)
{
    size_t rv = -1;
    int db_count = 0,tmp_count = 0;
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_sbuffer_init( &sbuf );
    msgpack_packer_init( &pk, &sbuf, msgpack_sbuffer_write );

    webconfig_db_data_t *db_data;
    webconfig_tmp_data_t *temp_data;

    db_data = webcfgdb;
    temp_data = webcfgtemp;

    while(db_data != NULL)
    {
        //WebcfgDebug("The db name is %s\n",db_data->name);
        db_data = db_data->next;
        //WebcfgDebug("The pack count of DB_count calc is %d\n",db_count );
        db_count++;
    }

    while(temp_data != NULL)
    {
        temp_data = temp_data->next;
        tmp_count++;
    }

    db_data = NULL;
    temp_data = NULL;

    db_data = webcfgdb;
    temp_data = webcfgtemp;

    if( db_data != NULL || temp_data != NULL ) {

	msgpack_pack_map( &pk, 1);

        __msgpack_pack_string( &pk, WEBCFG_BLOB_PARAMETERS.name, WEBCFG_BLOB_PARAMETERS.length );
	msgpack_pack_array( &pk, (db_count + tmp_count) );

        WebcfgDebug("The pack count of DB_count is %d\n",db_count );
        WebcfgDebug("The pack count of temp_count is %d\n",tmp_count);
	WebcfgDebug("The pack count of blob is %d\n",(db_count + tmp_count));

       if(db_data != NULL)
       {
	    while(db_data != NULL) //1 element
	    {
		if(db_data->root_string !=NULL)
		{
                msgpack_pack_map( &pk, 6); //name, version,status, error_details, root_string
		}
		else
		{
			msgpack_pack_map( &pk, 5);
		}

	        struct webcfg_token WEBCFG_MAP_BLOB_NAME;

                WEBCFG_MAP_BLOB_NAME.name = "name";
                WEBCFG_MAP_BLOB_NAME.length = strlen( "name" );
                __msgpack_pack_string_nvp( &pk, &WEBCFG_MAP_BLOB_NAME, db_data->name );

	        struct webcfg_token WEBCFG_MAP_BLOB_VERSION;

                WEBCFG_MAP_BLOB_VERSION.name = "version";
                WEBCFG_MAP_BLOB_VERSION.length = strlen( "version" );
	        __msgpack_pack_string( &pk, WEBCFG_MAP_BLOB_VERSION.name, WEBCFG_MAP_BLOB_VERSION.length);
                //WebcfgDebug("The db version is %ld\n",(long)db_data->version);
                msgpack_pack_uint64(&pk,(uint32_t) db_data->version);

                struct webcfg_token WEBCFG_MAP_BLOB_STATUS;

                WEBCFG_MAP_BLOB_STATUS.name = "status";
                WEBCFG_MAP_BLOB_STATUS.length = strlen( "status" );
                __msgpack_pack_string_nvp( &pk, &WEBCFG_MAP_BLOB_STATUS, "success" );

		struct webcfg_token WEBCFG_MAP_BLOB_ERROR_DETAILS;

                WEBCFG_MAP_BLOB_ERROR_DETAILS.name = "error_details";
                WEBCFG_MAP_BLOB_ERROR_DETAILS.length = strlen( "error_details" );
                __msgpack_pack_string_nvp( &pk, &WEBCFG_MAP_BLOB_ERROR_DETAILS, "none" );

	        struct webcfg_token WEBCFG_MAP_BLOB_ERROR_CODE;

                WEBCFG_MAP_BLOB_ERROR_CODE.name = "error_code";
                WEBCFG_MAP_BLOB_ERROR_CODE.length = strlen( "error_code" );
	        __msgpack_pack_string( &pk, WEBCFG_MAP_BLOB_ERROR_CODE.name, WEBCFG_MAP_BLOB_ERROR_CODE.length);
                msgpack_pack_uint64(&pk,0);

		if(db_data->root_string !=NULL)
		{
			struct webcfg_token WEBCFG_MAP_BLOB_ROOT_STRING;

		        WEBCFG_MAP_BLOB_ROOT_STRING.name = "root_string";
		        WEBCFG_MAP_BLOB_ROOT_STRING.length = strlen( "root_string" );
		        __msgpack_pack_string_nvp( &pk, &WEBCFG_MAP_BLOB_ROOT_STRING, db_data->root_string );
		}
                db_data = db_data->next;

	    }
       }

      if(temp_data != NULL)
       {
	    while(temp_data != NULL) //1 element
	    {
                msgpack_pack_map( &pk, 5); //name, version

	        struct webcfg_token WEBCFG_MAP_TEMP_NAME;

                WEBCFG_MAP_TEMP_NAME.name = "name";
                WEBCFG_MAP_TEMP_NAME.length = strlen( "name" );
                WebcfgDebug("The tmp name is %s\n",temp_data->name);
                __msgpack_pack_string_nvp( &pk, &WEBCFG_MAP_TEMP_NAME, temp_data->name );

	        struct webcfg_token WEBCFG_MAP_TEMP_VERSION;

                WEBCFG_MAP_TEMP_VERSION.name = "version";
                WEBCFG_MAP_TEMP_VERSION.length = strlen( "version" );
	        __msgpack_pack_string( &pk, WEBCFG_MAP_TEMP_VERSION.name, WEBCFG_MAP_TEMP_VERSION.length);
                //WebcfgDebug("The tmp version is %ld\n",(long)temp_data->version);
		msgpack_pack_uint64(&pk,(uint32_t) temp_data->version);

                struct webcfg_token WEBCFG_MAP_TEMP_STATUS;

                WEBCFG_MAP_TEMP_STATUS.name = "status";
                WEBCFG_MAP_TEMP_STATUS.length = strlen( "status" );
                WebcfgDebug("The tmp status is %s\n",temp_data->status);
                __msgpack_pack_string_nvp( &pk, &WEBCFG_MAP_TEMP_STATUS, temp_data->status );

		struct webcfg_token WEBCFG_MAP_TEMP_ERROR_DETAILS;

                WEBCFG_MAP_TEMP_ERROR_DETAILS.name = "error_details";
                WEBCFG_MAP_TEMP_ERROR_DETAILS.length = strlen( "error_details" );
                WebcfgDebug("The tmp error_details is %s\n",temp_data->error_details);
                __msgpack_pack_string_nvp( &pk, &WEBCFG_MAP_TEMP_ERROR_DETAILS, temp_data->error_details );

	        struct webcfg_token WEBCFG_MAP_TEMP_ERROR_CODE;

                WEBCFG_MAP_TEMP_ERROR_CODE.name = "error_code";
                WEBCFG_MAP_TEMP_ERROR_CODE.length = strlen( "error_code" );
	        __msgpack_pack_string( &pk, WEBCFG_MAP_TEMP_ERROR_CODE.name, WEBCFG_MAP_TEMP_ERROR_CODE.length);
                msgpack_pack_uint64(&pk,(uint16_t)temp_data->error_code);

                temp_data = temp_data->next;

	    }
       }
    } else {
        WebcfgError("parameters is NULL\n" );
        return rv;
    }

    if( sbuf.data ) {
        *data = ( char * ) malloc( sizeof( char ) * sbuf.size );

        if( NULL != *data ) {
            memcpy( *data, sbuf.data, sbuf.size );
	    WebcfgDebug("sbuf.data is %s sbuf.size %ld\n", sbuf.data, sbuf.size);
            rv = sbuf.size;
        }
    }

    msgpack_sbuffer_destroy( &sbuf );
    return rv;
}

ssize_t webcfgdb_pack( webconfig_db_data_t *packData, void **data, size_t count )
{
    size_t rv = -1;
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_sbuffer_init( &sbuf );
    msgpack_packer_init( &pk, &sbuf, msgpack_sbuffer_write );

    if( packData != NULL ) {

	msgpack_pack_map( &pk, 1);

        __msgpack_pack_string( &pk, WEBCFG_DB_PARAMETERS.name, WEBCFG_DB_PARAMETERS.length );
	msgpack_pack_array( &pk, count );
        
	WebcfgDebug("The pack count is %zu\n",count);
    
    webconfig_db_data_t *temp = packData;

	while(temp != NULL) //1 element
	{
	    if(temp->root_string !=NULL)
	    {
		msgpack_pack_map( &pk, 3); //name, version, root_string
	    }
	    else
	    {
		msgpack_pack_map( &pk, 2);
            }

	    struct webcfg_token WEBCFG_MAP_NAME;

            WEBCFG_MAP_NAME.name = "name";
            WEBCFG_MAP_NAME.length = strlen( "name" );
            __msgpack_pack_string_nvp( &pk, &WEBCFG_MAP_NAME, temp->name );

	    struct webcfg_token WEBCFG_MAP_VERSION;

            WEBCFG_MAP_VERSION.name = "version";
            WEBCFG_MAP_VERSION.length = strlen( "version" );
	    __msgpack_pack_string( &pk, WEBCFG_MAP_VERSION.name, WEBCFG_MAP_VERSION.length);
            //WebcfgDebug("The version is %ld\n",(long)temp->version);
            msgpack_pack_uint64(&pk,(uint32_t) temp->version);

	    if(temp->root_string !=NULL)
	    {
	    struct webcfg_token WEBCFG_MAP_ROOTSTRING;

            WEBCFG_MAP_ROOTSTRING.name = "root_string";
            WEBCFG_MAP_ROOTSTRING.length = strlen( "root_string" );
            __msgpack_pack_string_nvp( &pk, &WEBCFG_MAP_ROOTSTRING, temp->root_string );
            }
            temp = temp->next;

	}

    } else {
        WebcfgError("parameters is NULL\n" );
        return rv;
    }

    if( sbuf.data ) {
        *data = ( char * ) malloc( sizeof( char ) * sbuf.size );

        if( NULL != *data ) {
            memcpy( *data, sbuf.data, sbuf.size );
	    //WebcfgDebug("sbuf.data is %s sbuf.size %ld\n", sbuf.data, sbuf.size);
            rv = sbuf.size;
        }
    }

    msgpack_sbuffer_destroy( &sbuf );
    return rv;
}


ssize_t webcfg_pack_rootdoc(data1_t *packData, void **data )
{
    size_t rv = -1;
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_sbuffer_init( &sbuf );
    msgpack_packer_init( &pk, &sbuf, msgpack_sbuffer_write );
    int i =0;

    if( packData != NULL && packData->count != 0 ) {
	int count = packData->count;
	msgpack_pack_map( &pk, 1);
        __msgpack_pack_string( &pk, PAM_PARAMETERS.name, PAM_PARAMETERS.length );
	msgpack_pack_array( &pk, count );
        
	msgpack_pack_map( &pk, 3); //name, value, type

	for( i = 0; i < count; i++ ) //1 element
	{
	    struct webcfg_token PAM_MAP_NAME;

            PAM_MAP_NAME.name = "name";
            PAM_MAP_NAME.length = strlen( "name" );
            __msgpack_pack_string_nvp( &pk, &PAM_MAP_NAME, packData->data_items[count].name );
            
	    struct webcfg_token PAM_MAP_VALUE;

            PAM_MAP_VALUE.name = "value";
            PAM_MAP_VALUE.length = strlen( "value" );
	    __msgpack_pack_string_nvp( &pk, &PAM_MAP_VALUE, packData->data_items[count].value );

	    struct webcfg_token PAM_MAP_TYPE;

            PAM_MAP_TYPE.name = "dataType";
            PAM_MAP_TYPE.length = strlen( "dataType" );
             __msgpack_pack_string( &pk, PAM_MAP_TYPE.name, PAM_MAP_TYPE.length );
	    msgpack_pack_int(&pk, packData->data_items[count].type );
	}

    } else {
        printf("parameters is NULL\n" );
        return rv;
    }

    if( sbuf.data ) {
        *data = ( char * ) malloc( sizeof( char ) * sbuf.size );

        if( NULL != *data ) {
            memcpy( *data, sbuf.data, sbuf.size );
	    //printf("sbuf.data is %s sbuf.size %ld\n", sbuf.data, sbuf.size);
            rv = sbuf.size;
        }
    }

    msgpack_sbuffer_destroy( &sbuf );
    return rv;
}

