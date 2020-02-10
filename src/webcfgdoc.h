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
#ifndef __WEBCFGDOC_H__
#define __WEBCFGDOC_H__

#include <stdint.h>
#include <stdlib.h>
#include <msgpack.h>
typedef struct
{
    char *name;
    char *url;
    uint16_t version;
} doc_t;

typedef struct {
    doc_t *entries;       
    size_t      entries_count;
} webcfgdoc_t;

/**
 *  This function converts a msgpack buffer into an webcfgdoc_t structure
 *  if possible.
 *
 *  @param buf the buffer to convert
 *  @param len the length of the buffer in bytes
 *
 *  @return NULL on error, success otherwise
 */
webcfgdoc_t* webcfgdoc_convert( const void *buf, size_t len );

/**
 *  This function destroys an webcfgdoc_t object.
 *
 *  @param e the webcfgdoc to destroy
 */
void webcfgdoc_destroy( webcfgdoc_t *d );

/**
 *  This function returns a general reason why the conversion failed.
 *
 *  @param errnum the errno value to inspect
 *
 *  @return the constant string (do not alter or free) describing the error
 */
const char* webcfgdoc_strerror( int errnum );

#endif
