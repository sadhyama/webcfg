/*
 * Copyright 2019 Comcast Cable Communications Management, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.pam
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
#ifndef __PAM_PARAM_H__
#define __PAM_PARAM_H__
#include <stdint.h>
#include <stdlib.h>
#include <msgpack.h>

typedef struct
{
    size_t    entries_count;	   
} wifi_doc_t;

typedef struct
{
    char * vap_name;
    char * vlan;
    char * bridge;
    bool   enable;      
} tunnel_t;

typedef struct
{
    tunnel_t * entries;
    size_t    entries_count;	   
} tunnelTable_t;


typedef struct {
    char *        gre_name;
    char *        gre_primaryendpoint;
    char *        gre_secep;
    char *        gre_dev;
    char *        dscp;
    bool          enable;
    tunnelTable_t * table_param;
} tdoc_t;

typedef struct
{
    tdoc_t * entries;
    size_t    entries_count;	   
} tunneldoc_t;

typedef struct
{
    char *name;
    char *value;
    uint32_t   value_size;
    uint16_t type;
} pparam_t;

typedef struct {
    pparam_t   *entries;
    size_t      entries_count;
    char *        subdoc_name;
    uint32_t      version;
    uint16_t      transaction_id;
} pamparam_t;
/**
 *  This function converts a msgpack buffer into an pamparam_t structure
 *  if possible.
 *
 *  @param buf the buffer to convert
 *  @param len the length of the buffer in bytes
 *
 *  @return NULL on error, success otherwise
 */
pamparam_t* pamdoc_convert( const void *buf, size_t len );
/**
 *  This function destroys an tunneldoc_t object.
 *
 *  @param e the pamdoc to destroy
 */
void pamdoc_destroy( pamparam_t *pd );
/**
 *  This function returns a general reason why the conversion failed.
 *
 *  @param errnum the errno value to inspect
 *
 *  @return the constant string (do not alter or free) describing the error
 */
const char* pamdoc_strerror( int errnum );

tunneldoc_t* tunneldoc_convert(const void *buf, size_t len);

wifi_doc_t* wifi_doc_convert(const void *buf, size_t len);

void tunneldoc_destroy( tunneldoc_t *td );

void wifi_doc_destroy( wifi_doc_t *wd );
#endif
