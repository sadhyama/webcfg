 /**
  * Copyright 2019 Comcast Cable Communications Management, LLC
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
 */
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <CUnit/Basic.h>
#include "../src/webcfgparam.h"
#include "../src/multipart.h"
#include <msgpack.h>
#include <curl/curl.h>

char *url = NULL;
void test_multipart()
{
	int r_count=0;
	int configRet = -1;
	char *webConfigData = NULL;
	long res_code;
	if(url == NULL)
	{
		printf("\nProvide config URL as argument\n");
		return;
	}
	configRet = webcfg_http_request(url, &webConfigData, r_count, &res_code);
	if(configRet == 0)
	{
		printf("config ret success\n");
		//printf("webConfigData is %s\n", webConfigData);
		
	}	
	else
	{
		printf("webcfg_http_request failed\n");
	}
}

void add_suites( CU_pSuite *suite )
{
    *suite = CU_add_suite( "tests", NULL, NULL );
    CU_add_test( *suite, "Full", test_multipart);
}

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
int main( int argc, char *argv[] )
{
    unsigned rv = 1;
    CU_pSuite suite = NULL;
 
    printf("argc %d \n", argc );
    if(argv[1] !=NULL)
    {
    	url = strdup(argv[1]);
    }
    if( CUE_SUCCESS == CU_initialize_registry() ) {
        add_suites( &suite );

        if( NULL != suite ) {
            CU_basic_set_mode( CU_BRM_VERBOSE );
            CU_basic_run_tests();
            printf( "\n" );
            CU_basic_show_failures( CU_get_failure_list() );
            printf( "\n\n" );
            rv = CU_get_number_of_tests_failed();
        }

        CU_cleanup_registry();

    }
    return rv;
}

