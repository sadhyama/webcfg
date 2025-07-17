/*
 * Copyright 2025 Comcast Cable Communications Management, LLC
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
#ifndef _WEBCFG_METHOD_H_
#define _WEBCFG_METHOD_H_

#include <stdbool.h>
#include <rbus/rbus.h>
#include <rbus/rbus_object.h>
#include <rbus/rbus_value.h>
#include "webcfg.h"
#include "webcfg_log.h"
#include <wdmp-c.h>

bool isRbusMethodName(const char *name);
void setMethod_rbus(const param_t paramVal[], int methodCount, WDMP_STATUS *retStatus, int *ccspRetStatus);

#endif
