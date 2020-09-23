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
#ifndef __WEBCFG_AKER_H__
#define __WEBCFG_AKER_H__

#include "webcfg.h"
#include <wrp-c.h>
#include <libparodus.h>
#include "webcfg_db.h"
/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
typedef enum
{
    AKER_SUCCESS = 0,
    AKER_FAILURE,
    AKER_UNAVAILABLE
} AKER_STATUS;
/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
int send_aker_blob(char *paramName, char *blob, uint32_t blobSize, uint16_t docTransId, int version);
WEBCFG_STATUS checkAkerStatus();
void processAkerUpdateDelete(wrp_msg_t *wrpMsg);
void processAkerRetrieve(wrp_msg_t *wrpMsg);
libpd_instance_t get_webcfg_instance(void);
AKER_STATUS processAkerSubdoc(webconfig_tmp_data_t *docNode, int akerIndex);
void updateAkerMaxRetry(webconfig_tmp_data_t *temp, char *docname);
int akerwait__ (unsigned int secs);
pthread_cond_t *get_global_client_con(void);
pthread_mutex_t *get_global_client_mut(void);
pthread_cond_t *get_global_svc_con(void);
pthread_mutex_t *get_global_svc_mut(void);
#endif
