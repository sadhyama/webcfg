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
#include <stdlib.h>
#include <string.h>
#include <wdmp-c.h>
#include "webcfg_method.h"
#include "webcfg_rbus.h"



bool isRbusMethodName(const char *name)
{
    if (name == NULL || name[0] == '\0')
    {
        WebcfgError("[DEBUG] isRbusMethodName: Param name is NULL or empty\n");
        return false;
    }

    size_t len = strnlen(name, MAX_BUF_SIZE);
    if (!(len >= 2 && name[len - 2] == '(' && name[len - 1] == ')'))
    {
        WebcfgInfo("[DEBUG] isRbusMethodName: Not a valid method name: '%s'\n", name);
        return false;
    }

    WebcfgInfo("[DEBUG] isRbusMethodName: Valid method name: '%s'\n", name);
    return true;
}

void setMethod_rbus(const param_t paramVal[], int methodCount, WDMP_STATUS *retStatus, int *ccspRetStatus)
{
    if(!paramVal || methodCount<=0)
    {
        WebcfgError("setMethod_rbus: Invalid input parameters\n"); 
        return;
    }

    rbusHandle_t rbus_handle = get_global_rbus_handle();

    if (!rbus_handle)
    {
        WebcfgError("setMethod_rbus: rbus_handle not initialized\n");
        return;
    }

    for (int i = 0; i < methodCount; i++)
    {
        if(!paramVal[i].name || !paramVal[i].value)
        {
            WebcfgError("setMethod_rbus: Invalid param at index %d\n", i);
            *retStatus = WDMP_ERR_INVALID_PARAMETER_VALUE;
            *ccspRetStatus = CCSP_ERR_INVALID_PARAMETER_VALUE;
            return;
        }

        WebcfgInfo("Invoking rbus method.... %s\n", paramVal[i].name);

        rbusObject_t inParams = NULL, outParams = NULL;
        rbusValue_t val = NULL;
        rbusError_t ret = RBUS_ERROR_BUS_ERROR;

        rbusObject_Init(&inParams, NULL);
        rbusValue_Init(&val);
        rbusValue_SetString(val, paramVal[i].value);
        rbusObject_SetValue(inParams, "encoded_blob", val);
        
        ret = rbusMethod_Invoke(rbus_handle, paramVal[i].name, inParams, &outParams);
        WebcfgInfo("rbusMethod_Invoke(%s) returned: %d\n", paramVal[i].name, ret);

        *ccspRetStatus = mapRbusToCcspStatus((int)ret);
        *retStatus = mapStatus(*ccspRetStatus);

        WebcfgInfo("ccspRetStatus is %d\n", *ccspRetStatus);

        rbusValue_Release(val);
        rbusObject_Release(inParams);
        if (outParams)
            rbusObject_Release(outParams);
    }
}
