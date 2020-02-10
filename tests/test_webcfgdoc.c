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

int readFromFile(char *filename, char **data, int *len)
{
	FILE *fp;
	int ch_count = 0;
	fp = fopen(filename, "r+");
	if (fp == NULL)
	{
		printf("Failed to open file %s\n", filename);
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

int writeToFile(char *filename, char *data)
{
	FILE *fp;
	fp = fopen(filename , "w+");
	if (fp == NULL)
	{
		printf("Failed to open file %s\n", filename );
		return 0;
	}
	if(data !=NULL)
	{
		fwrite(data, strlen(data), 1, fp);
		fclose(fp);
		return 1;
	}
	else
	{
		printf("WriteToJson failed, Data is NULL\n");
		return 0;
	}
}

/* blob with 2 subdocs */
/*
[
  {
    "name": "blob1",
    "url": "http:/example.com/<mac>/blob1",
    "version": 1234
  },
  {
    "name": "blob2",
    "url": "http:/example.com/<mac>/blob2",
    "version": 65535
  }
]
*/
void test_webcfgsubdoc()
{
	webcfgdoc_t *pm;
	int err, len=0, i=0;
	char subdocfile[64] = "../../tests/doc-now.bin";
	char *binfileData = NULL;
	int status = -1;
	void* basicV; 

	status = readFromFile(subdocfile , &binfileData , &len);
	if(status)
	{
		printf("\n\nbinfileData is %s len %d\n", binfileData, len);

		printf("read status %d\n", status);
		basicV = ( void*)binfileData;

		pm = webcfgdoc_convert( basicV, len+1 );
		err = errno;
		printf( "errno: %s\n", webcfgdoc_strerror(err) );
		CU_ASSERT_FATAL( NULL != pm );
		CU_ASSERT_FATAL( NULL != pm->entries );
		CU_ASSERT_FATAL( 2 == pm->entries_count );
		CU_ASSERT_STRING_EQUAL( "blob1", pm->entries[0].name );
		CU_ASSERT_STRING_EQUAL( "http:/example.com/<mac>/blob1", pm->entries[0].url );
		CU_ASSERT_FATAL( 1234 == pm->entries[0].version );
		for(i = 0; i < (int)pm->entries_count ; i++)
		{
			printf("---->pm->entries[%d].name %s\n", i, pm->entries[i].name);
			printf("---->pm->entries[%d].url %s\n" , i, pm->entries[i].url);
			printf("---->pm->entries[%d].version %d\n", i, pm->entries[i].version);
		}

		webcfgdoc_destroy( pm );
	}
}

void test_rootdoc()
{
	webcfgparam_t *rpm;
	int err, len=0, i=0, valLen=0;
	char *binfileData = NULL;
	int status = -1;
	char rootdocfile[64] = "../../tests/doc-now.bin";

	status = readFromFile(rootdocfile, &binfileData , &len);
	if(status)
	{
	printf("read status %d\n", status);
	void* basicV;

	basicV = ( void*)binfileData;

	rpm = webcfgparam_convert( basicV, len+1 );
	err = errno;
	printf( "errno: %s\n", webcfgparam_strerror(err) );
	CU_ASSERT_FATAL( NULL != rpm );
	CU_ASSERT_FATAL( NULL != rpm->entries );
	CU_ASSERT_FATAL( 1 == rpm->entries_count );
	CU_ASSERT_STRING_EQUAL( "Device.X_RDK_WebConfig.RootConfig.Data", rpm->entries[0].name );
	CU_ASSERT_FATAL( 0 == rpm->entries[0].type );

	for(i = 0; i < (int)rpm->entries_count ; i++)
	{
		printf("---->pm->entries[%d].name %s\n", i, rpm->entries[i].name);
		printf("---->pm->entries[%d].value %s\n" , i, rpm->entries[i].value);
		printf("---->pm->entries[%d].type %d\n", i, rpm->entries[i].type);
	}

	//pass value field as void* to doc convert to decode .
	valLen = strlen(rpm->entries[0].value);
	printf("valLen is %d\n", valLen);
	/*int write_status =0;

	write_status = writeToFile("data.json", rpm->entries[0].value);
	printf("write status %d\n", write_status);*/
	webcfgdoc_t *pm;
	len =0;

	/*char *binfileData1 = NULL;
	status = readFromFile("data.json", &binfileData1 , &len);
	printf("binfileData is %s len %d\n", binfileData1, len);

	printf("read status %d\n", status);
	void* basicV1;

	basicV1 = ( void*)binfileData1;

	pm = webcfgdoc_convert( basicV1, len+1 );
	printf("len is %d\n", len);*/
	pm = webcfgdoc_convert( rpm->entries[0].value, valLen+1 );
	err = errno;
	printf( "errno: %s\n", webcfgdoc_strerror(err) );
	 CU_ASSERT_FATAL( NULL != pm );
	CU_ASSERT_FATAL( NULL != pm->entries );
	CU_ASSERT_FATAL( 1 == pm->entries_count );
	CU_ASSERT_STRING_EQUAL( "blob", pm->entries[0].name );
	CU_ASSERT_STRING_EQUAL( "http:/example.com/<mac>/blob", pm->entries[0].url );
	CU_ASSERT_FATAL( 2274 == pm->entries[0].version );
	for(i = 0; i < (int)pm->entries_count ; i++)
	{
		printf("pm->entries[%d].name %s\n", i, pm->entries[i].name);
		printf("pm->entries[%d].url %s\n" , i, pm->entries[i].url);
		printf("pm->entries[%d].version %d\n", i, pm->entries[i].version);
	}

	webcfgdoc_destroy( pm );
	}
}

void add_suites( CU_pSuite *suite )
{
    *suite = CU_add_suite( "tests", NULL, NULL );
    CU_add_test( *suite, "test_webcfgsubdoc", test_webcfgsubdoc);
    //CU_add_test( *suite, "test_rootdoc", test_rootdoc);
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
