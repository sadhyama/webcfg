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
#include "../src/webcfgdoc.h"
#include "../src/webcfgparam.h"
#include "../src/multipart.h"
#include <msgpack.h>
#include <curl/curl.h>

char *url = NULL;

void test_multipart()
{
	int r_count=0;
	int configRet = -1;
        webcfgparam_t *pm;
	char *webConfigData = NULL;
	long res_code;
          
        int len =0, i=0;
	void* subdbuff;
	char *subfileData = NULL;
        int err;

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
                int status = subdocparse("part3",&subfileData,&len);
                if(status)
		{       
                        
			subdbuff = ( void*)subfileData;

			//decode root doc
			printf("--------------decode root doc-------------\n");
			pm = webcfgparam_convert( subdbuff, len+1 );
                        
                        err = errno;
			printf( "errno: %s\n", webcfgparam_strerror(err) );
			CU_ASSERT_FATAL( NULL != pm );
			CU_ASSERT_FATAL( NULL != pm->entries );
			//CU_ASSERT_FATAL( 1 == pm->entries_count );
                        //CU_ASSERT_STRING_EQUAL("154363892090392891829182011",pm->version);
			//CU_ASSERT_STRING_EQUAL( "Device.NAT.PortMapping.", pm->entries[0].name );
			//CU_ASSERT_STRING_EQUAL( blob, pm->entries[0].value );
			//CU_ASSERT_FATAL( 0 == pm->entries[0].type );
			for(i = 0; i < (int)pm->entries_count ; i++)
			{
				printf("pm->entries[%d].name %s\n", i, pm->entries[i].name);
				printf("pm->entries[%d].value %s\n" , i, pm->entries[i].value);
				printf("pm->entries[%d].type %d\n", i, pm->entries[i].type);
			}
			
                        
                 }
                   webcfgparam_destroy( pm );
		
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

