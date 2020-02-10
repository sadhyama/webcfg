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

#include "multipart.h"
#include <uuid/uuid.h>
#include <string.h>
#include <stdlib.h>
/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
#define MAX_HEADER_LEN			4096
#define ETAG_HEADER 		       "Etag:"
#define CURL_TIMEOUT_SEC	   25L
#define CA_CERT_PATH 		   "/etc/ssl/certs/ca-certificates.crt"
#define WEBPA_READ_HEADER             "/etc/parodus/parodus_read_file.sh"
#define WEBPA_CREATE_HEADER           "/etc/parodus/parodus_create_file.sh"
/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
struct token_data {
    size_t size;
    char* data;
};

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
static char g_interface[32]="enp0s5";
static char g_ETAG[64]={'\0'};
char webpa_auth_token[4096]={'\0'};
#define DATA_SIZE 2321
int Curr_index ;
/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
size_t write_callback_fn(void *buffer, size_t size, size_t nmemb, struct token_data *data);
size_t header_callback(char *buffer, size_t size, size_t nitems);
void stripSpaces(char *str, char **final_str);
void createCurlheader( struct curl_slist *list, struct curl_slist **header_list);
void parse_multipart(char *ptr, int no_of_bytes, int part_no);
//static size_t datareader(void *ptr, size_t size, size_t nmemb, void* userp);
/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
/*
* @brief Initialize curl object with required options. create configData using libcurl.
* @param[out] configData 
* @param[in] len total configData size
* @param[in] r_count Number of curl retries on ipv4 and ipv6 mode during failure
* @return returns 0 if success, otherwise failed to fetch auth token and will be retried.
*/
int webcfg_http_request(char *webConfigURL, char **configData, int r_count, long *code)
{
	CURL *curl;
	CURLcode res;
	CURLcode time_res;
	struct curl_slist *list = NULL;
	struct curl_slist *headers_list = NULL;
	int rv=1;
	double total;
	long response_code = 0;
	char *interface = NULL;
	char *ct = NULL;
	char *boundary = NULL;
	char *str=NULL;
	char *line_boundary = NULL;
	char *last_line_boundary = NULL;
	char *str_body = NULL;

	//char *webConfigURL= NULL;
	int content_res=0;
	struct token_data data;
	data.size = 0;
	void * dataVal = NULL;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(curl)
	{
		//this memory will be dynamically grown by write call back fn as required
		data.data = (char *) malloc(sizeof(char) * 1);
		if(NULL == data.data)
		{
			printf("Failed to allocate memory.\n");
			return rv;
		}
		data.data[0] = '\0';
		createCurlheader(list, &headers_list);
		//getConfigURL(index, &configURL);

		if(webConfigURL !=NULL)
		{
			printf("webconfig root ConfigURL is %s\n", webConfigURL);
			curl_easy_setopt(curl, CURLOPT_URL, webConfigURL );
		}
		else
		{
			printf("Failed to get webconfig root configURL\n");
		}
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, CURL_TIMEOUT_SEC);
		if(strlen(g_interface) == 0)
		{
			//get_webCfg_interface(&interface);
			if(interface !=NULL)
		        {
		               strncpy(g_interface, interface, sizeof(g_interface)-1);
		               printf("g_interface copied is %s\n", g_interface);
		               WEBCFG_FREE(interface);
		        }
		}
		printf("g_interface fetched is %s\n", g_interface);
		if(strlen(g_interface) > 0)
		{
			printf("setting interface %s\n", g_interface);
			curl_easy_setopt(curl, CURLOPT_INTERFACE, g_interface);
		}

		// set callback for writing received data
		dataVal = &data;
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_fn);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, dataVal);

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_list);

		//printf("Set CURLOPT_HEADERFUNCTION option\n");
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);

		//curl_easy_setopt(curl, CURLOPT_READFUNCTION, datareader);
    		//curl_easy_setopt(curl, CURLOPT_READDATA,dataVal);
		// setting curl resolve option as default mode.
		//If any failure, retry with v4 first and then v6 mode. 
		if(r_count == 1)
		{
			printf("curl Ip resolve option set as V4 mode\n");
			curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
		}
		else if(r_count == 2)
		{
			printf("curl Ip resolve option set as V6 mode\n");
			curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V6);
		}
		else
		{
			printf("curl Ip resolve option set as default mode\n");
			curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_WHATEVER);
		}
		curl_easy_setopt(curl, CURLOPT_CAINFO, CA_CERT_PATH);
		// disconnect if it is failed to validate server's cert 
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		// Verify the certificate's name against host 
  		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
		// To use TLS version 1.2 or later 
  		curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
		// To follow HTTP 3xx redirections
  		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		// Perform the request, res will get the return code 
		res = curl_easy_perform(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
		printf("webConfig curl response %d http_code %ld\n", res, response_code);
		*code = response_code;
		time_res = curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total);
		if(time_res == 0)
		{
			printf("curl response Time: %.1f seconds\n", total);
		}
		curl_slist_free_all(headers_list);
		WEBCFG_FREE(webConfigURL);
		if(res != 0)
		{
			printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		else
		{
                        printf("checking content type\n");
			content_res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
			printf("ct is %s, content_res is %d\n", ct, content_res);
			// fetch boundary
			str = strtok(ct,";");
			str = strtok(NULL, ";");			
			boundary= strtok(str,"=");
			boundary= strtok(NULL,"=");
			printf( "boundary %s\n", boundary );
			int boundary_len= strlen(boundary);

			line_boundary  = (char *)malloc(sizeof(char) * (boundary_len +5));
			snprintf(line_boundary,boundary_len+5,"--%s\r\n",boundary);
			printf( "line_boundary %s, len %ld\n", line_boundary, strlen(line_boundary) );

			last_line_boundary  = (char *)malloc(sizeof(char) * (boundary_len + 5));
			snprintf(last_line_boundary,boundary_len+5,"--%s--",boundary);
			printf( "last_line_boundary %s, len %ld\n", last_line_boundary, strlen(last_line_boundary) );

			// Use --boundary to split
			str_body = malloc(sizeof(char) * data.size + 1);
			str_body = memcpy(str_body, data.data, data.size + 1);
			int num_of_parts = 0;
			char *ptr_lb=str_body;
			int index1=0, index2 =0;
			
			while((ptr_lb - str_body) < (int)data.size) {
				ptr_lb = memchr(ptr_lb, '-', data.size - (ptr_lb - str_body));
				if(0 == memcmp(ptr_lb, last_line_boundary, strlen(last_line_boundary)))
				{
					num_of_parts++;	
					printf("last line boundary match found at location %ld\n", ptr_lb-str_body);
					if(num_of_parts % 2 ==0 ) {
						index2 = ptr_lb-str_body-strlen(line_boundary);
						printf("index2 %d\n",index2);
						parse_multipart(str_body+index1,index2 - index1 - 1, num_of_parts);
					}
					else {
						index1 = ptr_lb-str_body;
						printf("index1 %d\n",index1);
						parse_multipart(str_body+index2,index1 - index2 - 1,num_of_parts);
					}
					
					break;

				}		
				else if(0 == memcmp(ptr_lb, line_boundary, strlen(line_boundary)))
				{
					num_of_parts++;	
					printf("%d line boundary match found at location %ld\n", num_of_parts, ptr_lb-str_body);
					ptr_lb = ptr_lb + strlen(line_boundary);
					if(num_of_parts % 2 ==0 ) {
						index2 = ptr_lb-str_body-strlen(line_boundary);
						printf("index2 %d\n",index2);
						parse_multipart(str_body+index1,index2 - index1 - 1,num_of_parts);
						index2 = index2+strlen(line_boundary);
						printf("index2 %d\n",index2);
					}
					else {
						index1 = ptr_lb-str_body;
						printf("index1 %d\n",index1);
					}
				}
				else
				{
					printf("No match for line boundary\n");
					ptr_lb++;
					//break;
				}
				
			}

			printf("Number of sub docs %d\n",num_of_parts-1);
			*configData=str_body;

		}
		WEBCFG_FREE(data.data);
		curl_easy_cleanup(curl);
		rv=0;
	}
	else
	{
		printf("curl init failure\n");
	}
	return rv;
}

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
void xxd( const void *buffer, const size_t length )
{
    const char hex[17] = "0123456789abcdef";
    const char *data = (char *) buffer;
    const char *end = &data[length];
    char output[70];
    size_t line = 0;

    if( (NULL == buffer) || (0 == length) ) {
        return;
    }

    while( data < end ) {
        int shiftCount;
        size_t i;
        char *text_ptr = &output[51];
        char *ptr = output;

        /* Output the '00000000:' portion */
        for (shiftCount=28; shiftCount >= 0; shiftCount -= 4) {
            *ptr++ = hex[(line >> shiftCount) & 0x0F];
        }
        *ptr++ = ':';
        *ptr++ = ' ';

        for( i = 0; i < 16; i++ ) {
            if( data < end ) {
                *ptr++ = hex[(0x0f & (*data >> 4))];
                *ptr++ = hex[(0x0f & (*data))];
                if( (' ' <= *data) && (*data <= '~') ) {
                    *text_ptr++ = *data;
                } else {
                    *text_ptr++ = '.';
                }
                data++;
            } else {
                *ptr++ = ' ';
                *ptr++ = ' ';
                *text_ptr++ = ' ';
            }
            if( 0x01 == (0x01 & i) ) {
                *ptr++ = ' ';
            }
        }
        line += 16;
        *ptr = ' ';

        *text_ptr++ = '\n';
        *text_ptr = '\0';

        puts( output );
    }
}

/** static size_t datareader(void *ptr, size_t size, size_t nmemb, void* userp)
{
  //read callback
  char *pooh = ( char *)userp;

  if(size*nmemb < 1)
    return 0;

  if(Curr_index < DATA_SIZE) {
    *(char *)ptr = pooh[Curr_index]; // copy one single byte 
    Curr_index++;
    return 1;                        // we return 1 byte at a time! s
  }
	printf("=========inside reader.. ptr %s\n", (char*)ptr);
  return 0;
}*/

/* @brief callback function for writing libcurl received data
 * @param[in] buffer curl delivered data which need to be saved.
 * @param[in] size size is always 1
 * @param[in] nmemb size of delivered data
 * @param[out] data curl response data saved.
*/
size_t write_callback_fn(void *buffer, size_t size, size_t nmemb, struct token_data *data)
{
    //size_t index = data->size;
    size_t n = (size * nmemb);
    char* tmp; 
    //char*mime_value = NULL;
//char* final_mime = NULL;
//	char mime_str[1024] = {'\0'};
    data->size += (size * nmemb);

    tmp = realloc(data->data, data->size + 1); // +1 for '\0' 

    if(tmp) {
        data->data = tmp;
    } else {
        if(data->data) {
            free(data->data);
        }
        printf("Failed to allocate memory for data\n");
        return 0;
    }
	//printf("------hex dump--------\n");
	 //xxd( buffer, nmemb );
	//printf("------hex dump--------\n");

	

	/*char *p = strtok(buffer,"2xKIxjfJuErFW+hmNCwEoMoY8I+ECM9efrV6EI4efSSW9QjI");
	char n1[2300];
	while(p != NULL)
	{
		strcpy(n1,p);
		p = strtok(NULL,"2xKIxjfJuErFW+hmNCwEoMoY8I+ECM9efrV6EI4efSSW9QjI");
		printf("%s\n",n1);
	}
	printf("%s\n",n1);
	printf("\n\nmime_value is %s\n", mime_value);*/
	/*while( mime_value != NULL )
	{
		mime_value = strtok(NULL, "2xKIxjfJuErFW+hmNCwEoMoY8I+ECM9efrV6EI4efSSW9QjI");
		printf("mime_value is %s\n", mime_value);
		if(mime_value !=NULL)
		{
			strncpy(mime_str, mime_value, sizeof(mime_str)-1);
			//printf("mime_str is %s\n", mime_str);
			stripSpaces(mime_str, &final_mime);

			//printf("final_mime is %s len %lu\n", final_mime, strlen(final_mime));
			strncpy(g_MIME, final_mime, sizeof(g_MIME)-1);
		}
	}*/


    memcpy(data->data, buffer, n);
    data->data[data->size] = '\0';
    printf("data size %lu\n", data->size);
    return size * nmemb;
}

/* @brief callback function to extract response header data.
*/
size_t header_callback(char *buffer, size_t size, size_t nitems)
{
	size_t etag_len = 0;
	char* header_value = NULL;
	char* final_header = NULL;
	char header_str[64] = {'\0'};

	etag_len = strlen(ETAG_HEADER);
	if( nitems > etag_len )
	{
		if( strncasecmp(ETAG_HEADER, buffer, etag_len) == 0 )
		{
			header_value = strtok(buffer, ":");
			while( header_value != NULL )
			{
				header_value = strtok(NULL, ":");
				if(header_value !=NULL)
				{
					strncpy(header_str, header_value, sizeof(header_str)-1);
					stripSpaces(header_str, &final_header);

					strncpy(g_ETAG, final_header, sizeof(g_ETAG)-1);
				}
			}
		}
	}
	printf("header_callback size %lu\n", size);
	return nitems;
}

//To strip all spaces , new line & carriage return characters from header output
void stripSpaces(char *str, char **final_str)
{
	int i=0, j=0;

	for(i=0;str[i]!='\0';++i)
	{
		if(str[i]!=' ')
		{
			if(str[i]!='\n')
			{
				if(str[i]!='\r')
				{
					str[j++]=str[i];
				}
			}
		}
	}
	str[j]='\0';
	*final_str = str;
}

/* @brief Function to create curl header options
 * @param[in] list temp curl header list
 * @param[in] device status value
 * @param[out] header_list output curl header list
*/
void createCurlheader( struct curl_slist *list, struct curl_slist **header_list)
{
	char *auth_header = NULL;

	printf("Start of createCurlheader\n");
	//add auth token here. for test purpose.
	strcpy(webpa_auth_token, "eyJhbGciOiJSUzI1NiIsImtpZCI6InRoZW1pcy0yMDE3MDEiLCJ0eXAiOiJKV1QifQ.eyJhdWQiOiJYTWlEVCIsImNhcGFiaWxpdGllcyI6WyJ4MTppc3N1ZXI6dGVzdDouKjphbGwiXSwiZXhwIjoxNTgzNTI2MjgwLCJpYXQiOjE1ODA5MzQyODAsImlzcyI6InRoZW1pcyIsImp0aSI6Il9wbXVHeVk4YkE4MmxONlI0dF9zYlEiLCJtYWMiOiJiNDJhMGU4NWU3OWEiLCJuYmYiOjE1ODA5MzQyNjUsInBhcnRuZXItaWQiOiJjb21jYXN0Iiwic3ViIjoiY2xpZW50OnN1cHBsaWVkIiwidHJ1c3QiOjEwMDAsInV1aWQiOiIxMjM0NSJ9.ZoneYFhGDpWF2xi-05vobf9s6ywPv-AuQjbHcFqduNYjEM0fAZCJgHJIYI4n0N3rc-qqYDqSa8ixF67W9FId5rvmTXZjzAuiBjHI7FKZY2cOGKDWM57u4j8LOy8X9hEzL2p0WCxVk9MWnK-zZO0K7XXdKc-f4BBohFCvZPbMQw7_erseOXfFiDFcKy19s1NsNXWkEzQNJYhfbwQfXcbMQwnqbRxO9iBx_pW-p2KpajD94yGtXkCpcVuCfbgv3llgymvrGlv6YZyz-yPb1xUex9G0Nrx3nz3Cpp7FzzdbuoW07tmxhMd_O2kyJfoQkPZd_8tbv6rWqtrREqBfp1WIDg");
	if(strlen(webpa_auth_token)==0)
	{
		printf(">>>>>>><token> is NULL.. hardcode token here. for test purpose.\n");
	}
	
	auth_header = (char *) malloc(sizeof(char)*MAX_HEADER_LEN);
	if(auth_header !=NULL)
	{
		snprintf(auth_header, MAX_HEADER_LEN, "Authorization:Bearer %s", (0 < strlen(webpa_auth_token) ? webpa_auth_token : NULL));
		list = curl_slist_append(list, auth_header);
		WEBCFG_FREE(auth_header);
	}
	
	list = curl_slist_append(list, "Accept: application/msgpack");

	*header_list = list;
}


int writeToFile(char *filename, char *data, int len)
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
		fwrite(data, len, 1, fp);
		fclose(fp);
		return 1;
	}
	else
	{
		printf("WriteToFile failed, Data is NULL\n");
		return 0;
	}
}

void parse_multipart(char *ptr, int no_of_bytes, int part_no) {
	
	printf("########################################\n");
	int i = 0;
	char *filename = malloc(sizeof(char)*6);
	snprintf(filename,6,"%s%d","part",part_no);
	while(i <= no_of_bytes)
	{
		putc(*(ptr+i),stdout);
		i++;
	}
	printf("########################################\n");
	writeToFile(filename,ptr,no_of_bytes);
}

