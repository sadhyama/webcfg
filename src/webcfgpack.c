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

#include "helpers.h"
#include "webcfgpack.h"

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
static const struct webcfg_token WEBCFG_PARAMETERS   = { .name = "parameters", .length = sizeof( "parameters" ) - 1 };
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



ssize_t webcfg_pack_rootdoc( char *blob, const data_t *packData, void **data )
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
        __msgpack_pack_string( &pk, WEBCFG_PARAMETERS.name, WEBCFG_PARAMETERS.length );
	msgpack_pack_array( &pk, count );
        
	msgpack_pack_map( &pk, 3); //name, value, type

	for( i = 0; i < count; i++ ) //1 element
	{
	    struct webcfg_token WEBCFG_MAP_NAME;

            WEBCFG_MAP_NAME.name = "name";
            WEBCFG_MAP_NAME.length = strlen( "name" );
            __msgpack_pack_string_nvp( &pk, &WEBCFG_MAP_NAME, "Device.X_RDK_WebConfig.RootConfig.Data" );

	    struct webcfg_token WEBCFG_MAP_VALUE;

            WEBCFG_MAP_VALUE.name = "value";
            WEBCFG_MAP_VALUE.length = strlen( "value" );
	    __msgpack_pack_string_nvp( &pk, &WEBCFG_MAP_VALUE, blob );

	    struct webcfg_token WEBCFG_MAP_TYPE;

            WEBCFG_MAP_TYPE.name = "datatype";
            WEBCFG_MAP_TYPE.length = strlen( "datatype" );
             __msgpack_pack_string( &pk, WEBCFG_MAP_TYPE.name, WEBCFG_MAP_TYPE.length );
	    msgpack_pack_int(&pk, 2 );
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


