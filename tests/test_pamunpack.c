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
#include <stdlib.h>
#include <base64.h>
#include <CUnit/Basic.h>
#include "../src/pam_param.h"
#include "../src/comp_helpers.h"

void pamUnpack(char *blob);

int readFromFile1(char *filename, char **data, int *len)
{
	FILE *fp;
	int ch_count = 0;
	fp = fopen(filename, "r");
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
        //fgets(*data,400,fp);
        printf("........data is %s len%lu\n", *data, strlen(*data));
	*len = ch_count;
	(*data)[ch_count] ='\0';
        printf("character count is %d\n",ch_count);
        printf("data is %s len %lu\n", *data, strlen(*data));
	fclose(fp);
	return 1;
}

char * base64encoder1(char * blob_data, size_t blob_size )
{
	char* b64buffer =  NULL;
	size_t encodeSize = -1;
   	printf("Data is %s\n", blob_data);
     	printf("-----------Start of Base64 Encode ------------\n");
        encodeSize = b64_get_encoded_buffer_size(blob_size);
        printf("encodeSize is %zu\n", encodeSize);
        b64buffer = malloc(encodeSize + 1);
        if(b64buffer != NULL)
        {
            memset( b64buffer, 0, sizeof( encodeSize )+1 );

            b64_encode((uint8_t *)blob_data, blob_size, (uint8_t *)b64buffer);
            b64buffer[encodeSize] = '\0' ;
        }
	printf("The encode value is %s\n", b64buffer);
	return b64buffer;
}

void test_pam_unpack()
{
	int len=0;
	char subdocfile[64] = "../../tests/pam.bin";

	char *binfileData = NULL;
	int status = -1;
	char* blobbuff = NULL; 
	char * encodedData = NULL;

	status = readFromFile1(subdocfile , &binfileData , &len);

	printf("read status %d\n", status);
	if(status == 1 )
	{
		blobbuff = ( char*)binfileData;
		printf("blobbuff %s blob len %lu\n", blobbuff, strlen(blobbuff));

	}
	encodedData = base64encoder1(blobbuff, len);
	pamUnpack(encodedData);
}

void pamUnpack(char *blob)
{
	pamparam_t *pd = NULL;
	tunneldoc_t * td = NULL;
	wifi_doc_t* wd = NULL;
	int err;
	int i ,j =0;

	if(blob != NULL)
	{

		printf("---------------start of b64 decode--------------\n");

		char * decodeMsg =NULL;
		int decodeMsgSize =0;
		int size =0;

		msgpack_zone mempool;
		msgpack_object deserialized;
		msgpack_unpack_return unpack_ret;

		decodeMsgSize = b64_get_decoded_buffer_size(strlen(blob));
		decodeMsg = (char *) malloc(sizeof(char) * decodeMsgSize);
		size = b64_decode((uint8_t *) blob, strlen(blob),(uint8_t *) decodeMsg );
		printf("base64 decoded data contains %d bytes\n",size);
	
		msgpack_zone_init(&mempool, 2048);
		unpack_ret = msgpack_unpack(decodeMsg, size, NULL, &mempool, &deserialized);
		switch(unpack_ret)
		{
			case MSGPACK_UNPACK_SUCCESS:
				printf("MSGPACK_UNPACK_SUCCESS :%d\n",unpack_ret);
			break;
			case MSGPACK_UNPACK_EXTRA_BYTES:
				printf("MSGPACK_UNPACK_EXTRA_BYTES :%d\n",unpack_ret);
			break;
			case MSGPACK_UNPACK_CONTINUE:
				printf("MSGPACK_UNPACK_CONTINUE :%d\n",unpack_ret);
			break;
			case MSGPACK_UNPACK_PARSE_ERROR:
				printf("MSGPACK_UNPACK_PARSE_ERROR :%d\n",unpack_ret);
			break;
			case MSGPACK_UNPACK_NOMEM_ERROR:
				printf("MSGPACK_UNPACK_NOMEM_ERROR :%d\n",unpack_ret);
			break;
			default:
				printf("Message Pack decode failed with error: %d\n", unpack_ret);
		}
		msgpack_zone_destroy(&mempool);

		printf("---------------End of b64 decode--------------\n");

		if(unpack_ret == MSGPACK_UNPACK_SUCCESS)
		{
			pd = pamdoc_convert(decodeMsg, size);//used to process the incoming msgobject
			err = errno;
			printf( "errno: %s\n", pamdoc_strerror(err) );

			if(pd != NULL)
			{
				td = tunneldoc_convert( pd->entries[0].value, pd->entries[0].value_size );
				err = errno;
				printf( "errno: %s\n", pamdoc_strerror(err) );

				wd = wifi_doc_convert( pd->entries[1].value, pd->entries[1].value_size);
				err = errno;
				printf( "errno: %s\n", pamdoc_strerror(err) );

				for(i = 0; i<(int) td->entries_count ; i++)
				{
					printf("td->entries[%d].gre_name is %s\n", i, td->entries[i].gre_name);
					printf("td->entries[%d].gre_primaryendpoint is %s\n", i, td->entries[i].gre_primaryendpoint);
					printf("td->entries[%d].gre_secep is %s\n", i, td->entries[i].gre_secep);
					printf("td->entries[%d].gre_dev is %s\n", i, td->entries[i].gre_dev);
					printf("td->entries[%d].dscp is %s\n", i, td->entries[i].dscp);
					printf("td->entries[%d].enable %s\n", i, (1 == td->entries[i].enable)?"true":"false");
					for(j = 0; j<(int) td->entries[i].table_param->entries_count; j++)
					{
						printf("td->entries[%d].table_param->entries[%d].vap_name %s\n",i, j, td->entries[i].table_param->entries[j].vap_name);
						printf("td->entries[%d].table_param->entries[%d].vlan %s\n",i, j,td->entries[i].table_param->entries[j].vlan);
						printf("td->entries[%d].table_param->entries[%d].bridge %s\n",i, j,td->entries[i].table_param->entries[j].bridge);
						printf("td->entries[%d].table_param->entries[%d].enable %s\n",i, j,(1 == td->entries[i].table_param->entries[j].enable)?"true":"false");	
					}
				}
				
			}
			pamdoc_destroy(pd);
			tunneldoc_destroy(td);
			wifi_doc_destroy(wd);
		}
	}
}
void add_suites( CU_pSuite *suite )
{
    *suite = CU_add_suite( "tests", NULL, NULL );
    CU_add_test( *suite, "test_pam_unpack", test_pam_unpack);
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
