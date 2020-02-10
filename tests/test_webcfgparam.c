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

#define WEB_CFG_FILE "../../tests/webcfg-now100.bin"
int readFromFile(char **data, int *len)
{
	FILE *fp;
	int ch_count = 0;
	fp = fopen(WEB_CFG_FILE, "r+");
	if (fp == NULL)
	{
		printf("Failed to open file %s. provide the file\n", WEB_CFG_FILE);
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	ch_count = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	*data = (char *) malloc(sizeof(char) * (ch_count + 1));
	fread(*data, 1, ch_count,fp);
        
	*len = ch_count;
	(*data)[ch_count] ='\0';
	fclose(fp);
	return 1;
}

void test_basic()
{

	webcfgparam_t *pm = NULL;
	int err, len=0, i=0;

	char *binfileData = NULL;
	int status = -1;
	printf("readFromFile\n");
	status = readFromFile(&binfileData , &len);

	printf("read status %d\n", status);
	if(status)
	{
		void* basicV;

		basicV = ( void*)binfileData;

		pm = webcfgparam_convert( basicV, len+1 );
		err = errno;
		printf( "errno: %s\n", webcfgparam_strerror(err) );
		CU_ASSERT_FATAL( NULL != pm );
		CU_ASSERT_FATAL( NULL != pm->entries );
		CU_ASSERT_FATAL( 100 == pm->entries_count );
		CU_ASSERT_STRING_EQUAL( "Device.DeviceInfo.X_RDKCENTRAL-COM_CloudUIEnable", pm->entries[0].name );
		CU_ASSERT_STRING_EQUAL( "false", pm->entries[0].value );
		CU_ASSERT_FATAL( 3 == pm->entries[0].type );
		for(i = 0; i < (int)pm->entries_count ; i++)
		{
			printf("pm->entries[%d].name %s\n", i, pm->entries[i].name);
			printf("pm->entries[%d].value %s\n" , i, pm->entries[i].value);
			printf("pm->entries[%d].type %d\n", i, pm->entries[i].type);
		}

	    	webcfgparam_destroy( pm );
	}
}

void add_suites( CU_pSuite *suite )
{
    *suite = CU_add_suite( "tests", NULL, NULL );
    CU_add_test( *suite, "Full", test_basic);
}

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
int main( int argc, char *argv[] )
{
    unsigned rv = 1;
    CU_pSuite suite = NULL;
 
    (void ) argc;
    (void ) argv;
    
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
