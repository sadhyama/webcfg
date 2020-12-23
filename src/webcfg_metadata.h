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
#ifndef __WEBCFG__METADATA_H__
#define __WEBCFG__METADATA_H__

#include <stdint.h>
#include <string.h>
#include "webcfg.h"
/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
#ifdef BUILD_YOCTO
#define WEBCFG_PROPERTIES_FILE 	    "/etc/webconfig.properties"
#else
#define WEBCFG_PROPERTIES_FILE 	    "/tmp/webconfig.properties"
#endif

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
WEBCFG_STATUS isSubDocSupported(char *subDoc);
void initWebcfgProperties(char * filename);
void setsupportedDocs( char * value);
void setsupportedVersion( char * value);
void setsupplementaryDocs( char * value);
char * getsupportedDocs();
char * getsupportedVersion();
char * getsupplementaryDocs();
char * getSupplementaryUrls();
void supplementaryUrls();
WEBCFG_STATUS isSupplemetaryDoc(char *subDoc);
#endif
