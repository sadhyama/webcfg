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
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <msgpack.h>
#include <pthread.h>
#include "webcfg_helpers.h"
#include "webcfg_multipart.h"
#include "webcfg_param.h"
#include "webcfg_log.h"
#include "webcfg_db.h"
#include "webcfg_pack.h"

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
enum {
    WD_OK                       = HELPERS_OK,
    WD_OUT_OF_MEMORY            = HELPERS_OUT_OF_MEMORY,
    WD_INVALID_FIRST_ELEMENT    = HELPERS_INVALID_FIRST_ELEMENT,
    WD_INVALID_DATATYPE,
    WD_INVALID_WD_OBJECT,
};

enum {
    BD_OK                       = HELPERS_OK,
    BD_OUT_OF_MEMORY            = HELPERS_OUT_OF_MEMORY,
    BD_INVALID_FIRST_ELEMENT    = HELPERS_INVALID_FIRST_ELEMENT,
    BD_INVALID_DATATYPE,
    BD_INVALID_BD_OBJECT,
};

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
static webconfig_tmp_data_t * g_head = NULL;
static blob_t * webcfgdb_blob = NULL;
static webconfig_db_data_t* webcfgdb_data = NULL;
pthread_mutex_t webconfig_db_mut=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t webconfig_tmp_data_mut=PTHREAD_MUTEX_INITIALIZER;
static int numOfMpDocs = 0;
static int success_doc_count = 0;
static int doc_fail_flag = 0;
/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
int process_webcfgdbparams( webconfig_db_data_t *e, msgpack_object_map *map );
int process_webcfgdb( webconfig_db_data_t *wd, msgpack_object *obj );

int process_webcfgdbblob( blob_struct_t *bd, msgpack_object *obj );
int process_webcfgdbblobparams( blob_data_t *e, msgpack_object_map *map );

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

//To initialize the DB when DB file is present
WEBCFG_STATUS initDB(char * db_file_path )
{
     FILE *fp = NULL;
     size_t sz = 0;
     char *data = NULL;
     size_t len = 0;
     int ch_count=0;
     webconfig_db_data_t* dm = NULL;

     WebcfgDebug("DB file path is %s\n", db_file_path);
     fp = fopen(db_file_path,"rb");

     if (fp == NULL)
     {
	WebcfgError("Failed to open file %s\n", db_file_path);
	return WEBCFG_FAILURE;
     }
     
     fseek(fp, 0, SEEK_END);
     ch_count = ftell(fp);
     if (ch_count == (int)-1)
     {
         WebcfgError("fread failed.\n");
	 fclose(fp);
         return WEBCFG_FAILURE;
     }
     fseek(fp, 0, SEEK_SET);
     data = (char *) malloc(sizeof(char) * (ch_count + 1));
     if(NULL == data)
     {
         WebcfgError("Memory allocation for data failed.\n");
         fclose(fp);
         return WEBCFG_FAILURE;
     }
     memset(data,0,(ch_count + 1));
     sz = fread(data, 1, ch_count,fp);
     if (sz == (size_t)-1) 
	{	
		fclose(fp);
		WebcfgError("fread failed.\n");
		WEBCFG_FREE(data);
		return WEBCFG_FAILURE;
	}
     len = ch_count;
     fclose(fp);

     dm = decodeData((void *)data, len);
     if(NULL == dm)
     {
	 WebcfgError("Msgpack decode failed\n");
         return WEBCFG_FAILURE;
     }
     else
     {
         webcfgdb_destroy (dm );
     }
     WEBCFG_FREE(data);
     generateBlob();
     return WEBCFG_SUCCESS;
     
}

//addNewDocEntry function will pack the DB blob and persist to a bin file
WEBCFG_STATUS addNewDocEntry(size_t count)
{    
     size_t webcfgdbPackSize = -1;
     void* data = NULL;
 
     WebcfgDebug("DB docs count %ld\n", (size_t)count);
     webcfgdbPackSize = webcfgdb_pack(webcfgdb_data, &data, count);
     WebcfgDebug("size of webcfgdbPackSize %ld\n", webcfgdbPackSize);
     WebcfgDebug("writeToDBFile %s\n", WEBCFG_DB_FILE);
     writeToDBFile(WEBCFG_DB_FILE,(char *)data,webcfgdbPackSize);
     if(data)
     {
	WEBCFG_FREE(data);
     }
     return WEBCFG_SUCCESS;
}

//generateBlob function is used to pack webconfig_tmp_data_t and webconfig_db_data_t
WEBCFG_STATUS generateBlob()
{
    size_t webcfgdbBlobPackSize = -1;
    void * data = NULL;

    if(webcfgdb_blob)
    {
	WebcfgDebug("Delete existing webcfgdb_blob.\n");
	WEBCFG_FREE(webcfgdb_blob->data);
	WEBCFG_FREE(webcfgdb_blob);
	webcfgdb_blob = NULL;
    }
    WebcfgDebug("Generate new blob\n");
    if(webcfgdb_data != NULL || g_head != NULL)
    {
        webcfgdbBlobPackSize = webcfgdb_blob_pack(webcfgdb_data, g_head, &data);
        webcfgdb_blob = (blob_t *)malloc(sizeof(blob_t));
        if(webcfgdb_blob != NULL)
        {
            memset( webcfgdb_blob, 0, sizeof( blob_t ) );

            webcfgdb_blob->data = (char *)data;
            webcfgdb_blob->len  = webcfgdbBlobPackSize;

            WebcfgDebug("The webcfgdbBlobPackSize is : %ld\n",webcfgdb_blob->len);
            return WEBCFG_SUCCESS;
        }
        else
        {
            WebcfgError("Failed in memory allocation for webcfgdb_blob\n");
            return WEBCFG_FAILURE;
        }
    }
    else
    {
        WebcfgError("Failed in packing blob\n");
        return WEBCFG_FAILURE;
    }
}

int writeToDBFile(char *db_file_path, char *data, size_t size)
{
	FILE *fp;
	fp = fopen(db_file_path , "w+");
	if (fp == NULL)
	{
		WebcfgError("Failed to open file in db %s\n", db_file_path );
		return 0;
	}
	if(data !=NULL)
	{
		fwrite(data, size, 1, fp);
		fclose(fp);
		return 1;
	}
	else
	{
		WebcfgError("WriteToJson failed, Data is NULL\n");
		fclose(fp);
		return 0;
	}
}

//Used to decode the DB bin file 
webconfig_db_data_t* decodeData(const void * buf, size_t len)
{
     return helper_convert( buf, len, sizeof(webconfig_db_data_t),"webcfgdb",
                           MSGPACK_OBJECT_ARRAY, true,
                           (process_fn_t) process_webcfgdb,
                           (destroy_fn_t) webcfgdb_destroy );
}

//Used to decode the webconfig_tmp_data_t and webconfig_db_data_t msgpack
blob_struct_t* decodeBlobData(const void * buf, size_t len)
{
     return helper_convert( buf, len, sizeof(blob_struct_t),"webcfgblob",
                           MSGPACK_OBJECT_ARRAY, true,
                           (process_fn_t) process_webcfgdbblob,
                           (destroy_fn_t) webcfgdbblob_destroy );
}


void webcfgdb_destroy( webconfig_db_data_t *pm )
{
	if( NULL != pm )
	{
		if( NULL != pm->name )
		{
			WEBCFG_FREE( pm->name );
		}
		WEBCFG_FREE( pm );
	}
}

void webcfgdbblob_destroy( blob_struct_t *bd )
{
    if( NULL != bd ) {
        size_t i;
        for( i = 0; i < bd->entries_count; i++ ) {
            if( NULL != bd->entries[i].name ) {
                free( bd->entries[i].name );
            }
            if( NULL != bd->entries[i].status ) {
                free( bd->entries[i].status );
            }
	    if( NULL != bd->entries[i].error_details ) {
                free( bd->entries[i].error_details );
            }
	    if( NULL != bd->entries[i].root_string ) {
                free( bd->entries[i].root_string );
            }
        }
        if( NULL != bd->entries ) {
            free( bd->entries );
        }
        free( bd );
    }
}

const char* webcfgdbblob_strerror( int errnum )
{
    struct error_map {
        int v;
        const char *txt;
    } map[] = {
        { .v = BD_OK,                               .txt = "No errors." },
        { .v = BD_OUT_OF_MEMORY,                    .txt = "Out of memory." },
        { .v = BD_INVALID_FIRST_ELEMENT,            .txt = "Invalid first element." },
        { .v = BD_INVALID_DATATYPE,                 .txt = "Invalid 'datatype' value." },
        { .v = BD_INVALID_BD_OBJECT,                .txt = "Invalid 'parameters' array." },
        { .v = 0, .txt = NULL }
    };
    int i = 0;

    while( (map[i].v != errnum) && (NULL != map[i].txt) ) { i++; }

    if( NULL == map[i].txt ) {
        return "Unknown error.";
    }

    return map[i].txt;
}

const char* webcfgdbparam_strerror( int errnum )
{
    struct error_map {
        int v;
        const char *txt;
    } map[] = {
        { .v = WD_OK,                               .txt = "No errors." },
        { .v = WD_OUT_OF_MEMORY,                    .txt = "Out of memory." },
        { .v = WD_INVALID_FIRST_ELEMENT,            .txt = "Invalid first element." },
        { .v = WD_INVALID_DATATYPE,                 .txt = "Invalid 'datatype' value." },
        { .v = WD_INVALID_WD_OBJECT,                .txt = "Invalid 'parameters' array." },
        { .v = 0, .txt = NULL }
    };
    int i = 0;

    while( (map[i].v != errnum) && (NULL != map[i].txt) ) { i++; }

    if( NULL == map[i].txt ) {
        return "Unknown error.";
    }

    return map[i].txt;
}

webconfig_db_data_t * get_global_db_node(void)
{
    webconfig_db_data_t* tmp = NULL;
    pthread_mutex_lock (&webconfig_db_mut);
    tmp = webcfgdb_data;
    pthread_mutex_unlock (&webconfig_db_mut);
    return tmp;
}

webconfig_tmp_data_t * get_global_tmp_node(void)
{
    webconfig_tmp_data_t * tmp = NULL;
    pthread_mutex_lock (&webconfig_tmp_data_mut);
    tmp = g_head;
    pthread_mutex_unlock (&webconfig_tmp_data_mut);
    return tmp;
}

void set_global_tmp_node(webconfig_tmp_data_t *new)
{
    pthread_mutex_lock (&webconfig_tmp_data_mut);
    g_head = new;
    pthread_mutex_unlock (&webconfig_tmp_data_mut);
}

int get_numOfMpDocs()
{
    return numOfMpDocs;
}

int get_successDocCount()
{
    return success_doc_count;
}

void reset_successDocCount()
{
    success_doc_count = 0;
}


int get_doc_fail()
{
    return doc_fail_flag;
}

void set_doc_fail( int value)
{
    doc_fail_flag = value;
}

blob_t * get_DB_BLOB()
{
     WebcfgDebug("Proceed to generateBlob\n");
     if(generateBlob() != WEBCFG_SUCCESS)
     {
	WebcfgError("Failed in Blob generation\n");
     }
     return webcfgdb_blob;
}

//new_node indicates the docs which need to be added to list
WEBCFG_STATUS addToTmpList()
{
	int retStatus = WEBCFG_FAILURE;
	int mp_count = 0;
	char * cloud_transaction_id = NULL;

	mp_count = get_multipartdoc_count();
	WebcfgInfo("multipartdoc count is %d\n", mp_count);
	//numOfMpDocs = 0;
	//WebcfgDebug("reset numOfMpDocs to %d\n", numOfMpDocs);

	//delete_tmp_doc_list();
	//WebcfgDebug("Deleted existing tmp list, proceed to addToTmpList\n");

	WebcfgInfo("checkTmpRootUpdate, proceed to addToTmpList\n");
	checkTmpRootUpdate();

	delete_tmp_docs_list();
	WebcfgInfo("Deleted existing tmp list based on sync type, proceed to addToTmpList\n");

	multipartdocs_t *mp_node = NULL;
	mp_node = get_global_mp();

	WebcfgInfo("The numdocs is %d\n",numOfMpDocs);
	cloud_transaction_id = get_global_transID();
	while(mp_node != NULL)
	{
		webconfig_tmp_data_t *new_node = NULL;

		if(numOfMpDocs == 0)
		{
			new_node=(webconfig_tmp_data_t *)malloc(sizeof(webconfig_tmp_data_t));

			if(new_node)
			{
				memset( new_node, 0, sizeof( webconfig_tmp_data_t ) );

				WebcfgInfo("Adding root doc to list\n");
				new_node->name = strdup("root");
				new_node->version = 0;
				if(!get_global_supplementarySync())
				{
					WebcfgInfo("Primary sync , update global root version to tmp list\n");
					new_node->version = get_global_root();
				}
				new_node->status = strdup("pending");
				//For root, isSupplementarySync is always 0 as root version is for primary sync.
				new_node->isSupplementarySync = 0;
				WebcfgInfo("new_node->isSupplementarySync is %d\n", new_node->isSupplementarySync);
				new_node->error_details = strdup("none");
				new_node->error_code = 0;
				new_node->trans_id = 0;
				new_node->retry_count = 0;
				new_node->retry_expiry_timestamp = 0;
				new_node->cloud_trans_id = strdup(cloud_transaction_id);
			}
		}
		else
		{
			//Add new nodes based on sync type primary/secondary
			if(mp_node->isSupplementarySync == get_global_supplementarySync())
			{
				new_node=(webconfig_tmp_data_t *)malloc(sizeof(webconfig_tmp_data_t));
				if(new_node)
				{
					memset( new_node, 0, sizeof( webconfig_tmp_data_t ) );

					new_node->name = strdup(mp_node->name_space);
					WebcfgDebug("mp_node->name_space is %s\n", mp_node->name_space);
					new_node->version = mp_node->etag;
					new_node->status = strdup("pending");
					new_node->isSupplementarySync = mp_node->isSupplementarySync;
					new_node->error_details = strdup("none");
					new_node->error_code = 0;
					new_node->trans_id = 0;
					new_node->retry_count = 0;
					new_node->retry_expiry_timestamp = 0;
					new_node->cloud_trans_id = strdup(cloud_transaction_id);

					WebcfgDebug("new_node->name is %s\n", new_node->name);
					WebcfgDebug("new_node->version is %lu\n", (long)new_node->version);
					WebcfgDebug("new_node->status is %s\n", new_node->status);
					WebcfgDebug("new_node->isSupplementarySync is %d\n", new_node->isSupplementarySync);
					WebcfgDebug("new_node->error_details is %s\n", new_node->error_details);
					WebcfgDebug("new_node->retry_count is %d\n", new_node->retry_count);
					WebcfgDebug("new_node->retry_expiry_timestamp is %lld\n", new_node->retry_expiry_timestamp);
					WebcfgDebug("new_node->cloud_trans_id is %s\n", new_node->cloud_trans_id);
				}

			}
			mp_node = mp_node->next;
		}

		if(new_node)
		{
			new_node->next=NULL;
			pthread_mutex_lock (&webconfig_tmp_data_mut);
			if (g_head == NULL)
			{
				g_head = new_node;
				pthread_mutex_unlock (&webconfig_tmp_data_mut);
			}
			else
			{
				webconfig_tmp_data_t *temp = NULL;
				WebcfgDebug("Adding docs to list\n");
				temp = g_head;
				while(temp->next !=NULL)
				{
					temp=temp->next;
				}
				temp->next=new_node;
				pthread_mutex_unlock (&webconfig_tmp_data_mut);
			}

			WebcfgDebug("--->>doc %s with version %lu is added to list\n", new_node->name, (long)new_node->version);
			numOfMpDocs = numOfMpDocs + 1;
		}
		WebcfgDebug("numOfMpDocs %d\n", numOfMpDocs);

		//if(mp_count+1 == numOfMpDocs) //TODO:check if it can be optimized based on count.
		//{
			WebcfgDebug("addToTmpList success\n");
			retStatus = WEBCFG_SUCCESS;
		//}
	}
	WebcfgDebug("addToList return %d\n", retStatus);
	return retStatus;
}


void checkDBList(char *docname, uint32_t version, char* rootstr)
{
	if(updateDBlist(docname, version, rootstr) != WEBCFG_SUCCESS)
	{
		webconfig_db_data_t * webcfgdb = NULL;
		webcfgdb = (webconfig_db_data_t *) malloc (sizeof(webconfig_db_data_t));
		if(webcfgdb != NULL)
		{
			memset( webcfgdb, 0, sizeof( webconfig_db_data_t ) );

			webcfgdb->name = strdup(docname);
			webcfgdb->version = version;
			if( strcmp("root", webcfgdb->name) == 0)
			{
				if(rootstr !=NULL)
				{
					webcfgdb->root_string = strdup(rootstr);
				}
			}
			webcfgdb->next = NULL;

			addToDBList(webcfgdb);
			if(webcfgdb->root_string !=NULL)
			{
				WebcfgInfo("webcfgdb->name added to DB %s webcfgdb->version %lu webcfgdb->root_string %s\n",webcfgdb->name, (long)webcfgdb->version, webcfgdb->root_string);
			}
			else
			{
				WebcfgInfo("webcfgdb->name added to DB %s webcfgdb->version %lu\n",webcfgdb->name, (long)webcfgdb->version);
			}
		}
		else
		{
			WebcfgError("Failed in memory allocation for webcfgdb\n");
		}
	}
}

WEBCFG_STATUS updateDBlist(char *docname, uint32_t version, char* rootstr)
{
	webconfig_db_data_t *webcfgdb = NULL;
	webcfgdb = get_global_db_node();

	//Traverse through doc list & update required doc
	while (NULL != webcfgdb)
	{
		pthread_mutex_lock (&webconfig_db_mut);
		WebcfgDebug("mutex_lock in updateDBlist\n");
		WebcfgDebug("node is pointing to webcfgdb->name %s, docname %s, dblen %zu, doclen %zu webcfgdb->root_string %s\n",webcfgdb->name, docname, strlen(webcfgdb->name), strlen(docname), webcfgdb->root_string);
		if( strcmp(docname, webcfgdb->name) == 0)
		{
			webcfgdb->version = version;
			if( strcmp("root", webcfgdb->name) == 0)
			{
				if(webcfgdb->root_string !=NULL)
				{
					WEBCFG_FREE(webcfgdb->root_string);
					webcfgdb->root_string = NULL;
				}
				if(rootstr!=NULL)
				{
					webcfgdb->root_string = strdup(rootstr);
				}
			}
			WebcfgDebug("webcfgdb %s is updated to version %lu webcfgdb->root_string %s\n", docname, (long)webcfgdb->version, webcfgdb->root_string);
			pthread_mutex_unlock (&webconfig_db_mut);
			WebcfgDebug("mutex_unlock if docname is webcfgdb name\n");
			return WEBCFG_SUCCESS;
		}
		webcfgdb= webcfgdb->next;
		pthread_mutex_unlock (&webconfig_db_mut);
		WebcfgDebug("mutex_unlock in doc name is not webcfgdb name\n");
	}
	return WEBCFG_FAILURE;
}
//update version, status for each doc
WEBCFG_STATUS updateTmpList(webconfig_tmp_data_t *temp, char *docname, uint32_t version, char *status, char *error_details, uint16_t error_code, uint16_t trans_id, int retry)
{
	if (NULL != temp)
	{
		WebcfgDebug("updateTmpList: node is pointing to temp->name %s \n",temp->name);
		pthread_mutex_lock (&webconfig_tmp_data_mut);
		WebcfgDebug("mutex_lock in updateTmpList\n");
		if( strcmp(docname, temp->name) == 0)
		{
			temp->version = version;
			if(strcmp(temp->status, status) !=0)
			{
				WEBCFG_FREE(temp->status);
				temp->status = NULL;
				temp->status = strdup(status);
			}
			if(strcmp(temp->error_details, error_details) !=0)
			{
				WEBCFG_FREE(temp->error_details);
				temp->error_details = NULL;
				temp->error_details = strdup(error_details);
			}
			temp->error_code = error_code;
			temp->trans_id = trans_id;
			WebcfgDebug("updateTmpList: retry %d\n", retry);
			if(!retry)
			{
				WebcfgDebug("updateTmpList: reset temp->retry_count\n");
				temp->retry_count = 0;
			}
			WebcfgInfo("doc %s is updated to version %lu status %s error_details %s error_code %lu trans_id %lu temp->retry_count %d\n", docname, (long)temp->version, temp->status, temp->error_details, (long)temp->error_code, (long)temp->trans_id, temp->retry_count);
			pthread_mutex_unlock (&webconfig_tmp_data_mut);
			WebcfgDebug("mutex_unlock in current temp details\n");
			return WEBCFG_SUCCESS;
		}
		pthread_mutex_unlock (&webconfig_tmp_data_mut);
		WebcfgDebug("mutex_unlock in updateTmpList\n");
	}
	WebcfgError("updateTmpList failed as doc %s is not in tmp list\n", docname);
	return WEBCFG_FAILURE;
}


//delete doc from webcfg Tmp list
WEBCFG_STATUS deleteFromTmpList(char* doc_name)
{
	webconfig_tmp_data_t *prev_node = NULL, *curr_node = NULL;

	if( NULL == doc_name )
	{
		WebcfgError("Invalid value for doc\n");
		return WEBCFG_FAILURE;
	}
	WebcfgDebug("doc to be deleted: %s\n", doc_name);

	prev_node = NULL;
	pthread_mutex_lock (&webconfig_tmp_data_mut);	
	curr_node = g_head ;
	// Traverse to get the doc to be deleted
	while( NULL != curr_node )
	{
		if(strcmp(curr_node->name, doc_name) == 0)
		{
			WebcfgDebug("Found the node to delete\n");
			if( NULL == prev_node )
			{
				WebcfgDebug("need to delete first doc\n");
				g_head = curr_node->next;
			}
			else
			{
				WebcfgDebug("Traversing to find node\n");
				prev_node->next = curr_node->next;
			}

			WebcfgDebug("Deleting the node entries\n");
			WEBCFG_FREE( curr_node->name );
			WEBCFG_FREE( curr_node->status );
			WEBCFG_FREE( curr_node->error_details );
			WEBCFG_FREE( curr_node );
			curr_node = NULL;
			WebcfgDebug("Deleted successfully and returning..\n");
			numOfMpDocs =numOfMpDocs - 1;
			WebcfgDebug("numOfMpDocs after delete is %d\n", numOfMpDocs);
			pthread_mutex_unlock (&webconfig_tmp_data_mut);
			return WEBCFG_SUCCESS;
		}

		prev_node = curr_node;
		curr_node = curr_node->next;
	}
	pthread_mutex_unlock (&webconfig_tmp_data_mut);
	WebcfgError("Could not find the entry to delete from list\n");
	return WEBCFG_FAILURE;
}

void delete_tmp_doc_list()
{
   webconfig_tmp_data_t *temp = NULL;
   webconfig_tmp_data_t *head = NULL;
   head = get_global_tmp_node();

    while(head != NULL)
    {
        temp = head;
	head = head->next;
	WebcfgDebug("Delete node--> temp->name %s temp->version %lu temp->status %s temp->isSupplementarySync %d temp->error_details %s temp->error_code %lu temp->trans_id %lu temp->retry_count %d temp->cloud_trans_id %s\n",temp->name, (long)temp->version, temp->status, temp->isSupplementarySync, temp->error_details, (long)temp->error_code, (long)temp->trans_id, temp->retry_count, temp->cloud_trans_id);
	free(temp);
	temp = NULL;
    }
	pthread_mutex_lock (&webconfig_tmp_data_mut);
    	g_head = NULL;
	pthread_mutex_unlock (&webconfig_tmp_data_mut);
    	WebcfgDebug("mutex_unlock Deleted all docs from tmp list\n");
}

//Delete all docs other than root from tmp list based on sync type primary/secondary
void delete_tmp_docs_list()
{
   webconfig_tmp_data_t *temp = NULL;
   temp = get_global_tmp_node();

    WebcfgInfo("Inside delete_tmp_docs_list()\n");
    while(temp != NULL)
    {
	//skip root delete
	if((strcmp(temp->name, "root") !=0) && (temp->isSupplementarySync == get_global_supplementarySync()))
	{
		WebcfgInfo("Delete node--> temp->name %s temp->version %lu temp->status %s temp->isSupplementarySync %d temp->error_details %s temp->error_code %lu temp->trans_id %lu temp->retry_count %d temp->cloud_trans_id %s\n",temp->name, (long)temp->version, temp->status, temp->isSupplementarySync, temp->error_details, (long)temp->error_code, (long)temp->trans_id, temp->retry_count, temp->cloud_trans_id);
		deleteFromTmpList(temp->name);
	}
	temp = temp->next;
    }
	//pthread_mutex_lock (&webconfig_tmp_data_mut); //TODO:mutex locks
	//pthread_mutex_unlock (&webconfig_tmp_data_mut);
}

//Update tmp root for primary sync
void checkTmpRootUpdate()
{
   if((numOfMpDocs !=0) && !get_global_supplementarySync())
   {
	WebcfgInfo("Inside tmp root Update\n");
	webconfig_tmp_data_t * root_node = NULL;
	root_node = getTmpNode("root");
	WebcfgInfo("Update root version %lu to tmp list.\n", (long)get_global_root());
	updateTmpList(root_node, "root", get_global_root(), "pending", "none", 0, 0, 0);
   }
   WebcfgInfo("root updateTmpList done\n");
}

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/

/**
 *  Convert the msgpack map into the webconfig_db_data_t structure.
 *
 *  @param e    the entry pointer
 *  @param map  the msgpack map pointer
 *
 *  @return 0 on success, error otherwise
 */
int process_webcfgdbparams( webconfig_db_data_t *e, msgpack_object_map *map )
{
    int left = map->size;
    uint8_t objects_left = 0x03;
    msgpack_object_kv *p;

    p = map->ptr;
    while( (0 < objects_left) && (0 < left--) )
    {
        if( MSGPACK_OBJECT_STR == p->key.type )
        {
            if( MSGPACK_OBJECT_POSITIVE_INTEGER == p->val.type )
            {
                if( 0 == match(p, "version") )
                {
                    if( UINT32_MAX < p->val.via.u64 )
                    {
			//WebcfgDebug("e->type is %d\n", e->type);
                        errno = WD_INVALID_DATATYPE;
                        return -1;
                    }
                    else
                    {
                        e->version = (uint32_t) p->val.via.u64;
			//WebcfgDebug("e->version is %lu\n", (long)e->version);
                    }
                    objects_left &= ~(1 << 1);
		    //WebcfgDebug("objects_left after datatype %d\n", objects_left);
                }
                
            }
            else if( MSGPACK_OBJECT_STR == p->val.type )
            {
                if( 0 == match(p, "name") )
                {
                    e->name = strndup( p->val.via.str.ptr, p->val.via.str.size );
		    //WebcfgDebug("e->name is %s\n", e->name);
                    objects_left &= ~(1 << 2);
		    //WebcfgDebug("objects_left after name %d\n", objects_left);
                }
		else if( 0 == match(p, "root_string") )
                {
                    e->root_string = strndup( p->val.via.str.ptr, p->val.via.str.size );
		    //WebcfgDebug("e->root_string is %s\n", e->root_string);
                    objects_left &= ~(1 << 0);
		    //WebcfgDebug("objects_left after root_string %d\n", objects_left);
                }
            }
        }
        p++;
    }
    if( 1 & objects_left )
    {
	if( (1 << 0) & objects_left )
	{
		WebcfgDebug("Skip optional root_string element\n");
		objects_left &= ~(1 << 0);
	}
    }
    else
    {
        errno = WD_OK;
    }

    return (0 == objects_left) ? 0 : -1;
}


int process_webcfgdb( webconfig_db_data_t *wd, msgpack_object *obj )
{
    if(NULL == obj)
    {
	WebcfgError("Msgpack decode obj NULL\n");
        return -1;
    }

    msgpack_object_array *array = &obj->via.array;
    if( 0 < array->size )
    {
        size_t i;
        size_t entries_count = -1;

        entries_count = array->size;
        webcfgdb_data = NULL;
        WebcfgDebug("entries_count %zu\n",entries_count);
        for( i = 0; i < entries_count; i++ )
        {
            wd = (webconfig_db_data_t *) malloc (sizeof(webconfig_db_data_t));
            if(NULL == wd)
            {
                WebcfgError("Failed in memory allocation for wd\n");
                return -1;
            }

            memset( wd, 0, sizeof( webconfig_db_data_t ) );

            if( MSGPACK_OBJECT_MAP != array->ptr[i].type )
            {
                errno = WD_INVALID_WD_OBJECT;
		WEBCFG_FREE(wd);
                return -1;
            }
            if( 0 != process_webcfgdbparams(wd, &array->ptr[i].via.map) )
            {
		WebcfgError("process_webcfgdbparam failed\n");
		WEBCFG_FREE(wd);
                return -1;
            }
            else
            {
                wd->next = NULL;
                addToDBList(wd);
               
            }
        }
    }

    return 0;
}

void addToDBList(webconfig_db_data_t *webcfgdb)
{
      pthread_mutex_lock (&webconfig_db_mut); 
      if(webcfgdb_data == NULL)
      {
          webcfgdb_data = webcfgdb;
	  pthread_mutex_unlock (&webconfig_db_mut);
          success_doc_count++;
	  WebcfgInfo("Producer added webcfgdb->name %s, webcfg->version %lu, success_doc_count %d\n",webcfgdb->name, (long)webcfgdb->version, success_doc_count);
      }
      else
      {
          webconfig_db_data_t *temp = NULL;
          temp = webcfgdb_data;
          while(temp->next)
          {   
              temp = temp->next;
          }
          temp->next = webcfgdb;
          pthread_mutex_unlock (&webconfig_db_mut);
          success_doc_count++;
	  WebcfgInfo("Producer added webcfgdb->name %s, webcfg->version %lu, success_doc_count %d\n",webcfgdb->name, (long)webcfgdb->version, success_doc_count);
      }
}

char * get_DB_BLOB_base64()
{
    char* b64buffer =  NULL;
    char * decodeMsg = NULL;
    size_t k ;
    blob_t * db_blob = get_DB_BLOB();
    size_t encodeSize = -1;
    size_t size =0;
    size_t decodeMsgSize =0;

    if(db_blob != NULL)
    {
        WebcfgDebug("-----------Start of Base64 Encode ------------\n");
        encodeSize = b64_get_encoded_buffer_size( db_blob->len );
        WebcfgDebug("encodeSize is %zu\n", encodeSize);
        b64buffer = malloc(encodeSize + 1);
        if(b64buffer != NULL)
        {
            memset( b64buffer, 0, sizeof( encodeSize )+1 );

            b64_encode((uint8_t *)db_blob->data, db_blob->len, (uint8_t *)b64buffer);
            b64buffer[encodeSize] = '\0' ;

	    //Start of b64 decoding for debug purpose
	    WebcfgDebug("----Start of b64 decoding----\n");
	    decodeMsgSize = b64_get_decoded_buffer_size(strlen(b64buffer));
	    WebcfgDebug("expected b64 decoded msg size : %ld bytes\n",decodeMsgSize);

	    decodeMsg = (char *) malloc(sizeof(char) * decodeMsgSize);
	    if(decodeMsg)
	    {
		memset( decodeMsg, 0, sizeof(char) *  decodeMsgSize );
		size = b64_decode( (const uint8_t *)b64buffer, strlen(b64buffer), (uint8_t *)decodeMsg );

		WebcfgInfo("base64 decoded data containing %zu bytes\n",size);

		blob_struct_t *bd = NULL;
		bd = decodeBlobData((void *)decodeMsg, size);
		WebcfgDebug("Size of webcfgdbblob %ld\n", (size_t)bd);
		if(bd != NULL)
		{
			for(k = 0;k< bd->entries_count ; k++)
			{
				if(bd->entries[k].root_string !=NULL)
				{
					WebcfgInfo("Blob bd->entries[%zu].name %s, version: %lu, status: %s, error_details: %s, error_code: %d root_string: %s\n", k, bd->entries[k].name, (long)bd->entries[k].version, bd->entries[k].status, bd->entries[k].error_details, bd->entries[k].error_code, bd->entries[k].root_string );
				}
				else
				{
					WebcfgInfo("Blob bd->entries[%zu].name %s, version: %lu, status: %s, error_details: %s, error_code: %d\n", k, bd->entries[k].name, (long)bd->entries[k].version, bd->entries[k].status, bd->entries[k].error_details, bd->entries[k].error_code );
				}
			}

		}
		webcfgdbblob_destroy(bd);
		WEBCFG_FREE(decodeMsg);
	    }

            WebcfgDebug("---------- End of Base64 decode -------------\n");
        }
     }
     else
     {
        WebcfgError("Blob is NULL\n");
     }
    return b64buffer;
}

/* For Blob decode purpose */

int process_webcfgdbblobparams( blob_data_t *e, msgpack_object_map *map )
{
    int left = map->size;
    uint8_t objects_left = 0x0f;
    msgpack_object_kv *p;

    p = map->ptr;
    while( (0 < objects_left) && (0 < left--) )
    {
        if( MSGPACK_OBJECT_STR == p->key.type )
        {
            if( MSGPACK_OBJECT_POSITIVE_INTEGER == p->val.type )
            {
                if( 0 == match(p, "version") )
                {
                    if( UINT32_MAX < p->val.via.u64 )
                    {
                        errno = BD_INVALID_DATATYPE;
                        return -1;
                    }
                    else
                    {
                        e->version = (uint32_t) p->val.via.u64;
			//WebcfgDebug("e->version is %lu\n", (long)e->version);
                    }
                    objects_left &= ~(1 << 0);
                }
                else if( 0 == match(p, "error_code"))
                {
                    if( UINT32_MAX < p->val.via.u64 )
                    {
                        errno = BD_INVALID_DATATYPE;
                        return -1;
                    }
                    else
                    {
                        e->error_code = (uint16_t) p->val.via.u64;
			//WebcfgDebug("e->version is %d\n", e->error_code);
                    }
                    objects_left &= ~(1 << 4);
                
                }
            }
            else if( MSGPACK_OBJECT_STR == p->val.type )
            {
                if( 0 == match(p, "name") )
                {
                    e->name = strndup( p->val.via.str.ptr, p->val.via.str.size );
		    //WebcfgDebug("e->name is %s\n", e->name);
                    objects_left &= ~(1 << 0);
                }
                else if(0 == match(p, "status") )
                {
                     e->status = strndup( p->val.via.str.ptr, p->val.via.str.size );
		     //WebcfgDebug("e->status is %s\n", e->status);
                     objects_left &= ~(1 << 1);
                }
		else if(0 == match(p, "error_details") )
                {
                     e->error_details = strndup( p->val.via.str.ptr, p->val.via.str.size );
		     //WebcfgDebug("e->error_details is %s\n", e->error_details);
                     objects_left &= ~(1 << 2);
                }
		else if(0 == match(p, "root_string") )
                {
                     e->root_string = strndup( p->val.via.str.ptr, p->val.via.str.size );
		     //WebcfgDebug("e->root_string is %s\n", e->root_string);
                     objects_left &= ~(1 << 3);
                }
            }
        }
        p++;
    }
    if( 1 & objects_left )
    {
    }
    else if( (1 << 3) & objects_left ) {
	    WebcfgDebug("Skip optional root_string element\n");
        objects_left &= ~(1 << 3);
    }
    else
    {
        errno = BD_OK;
    }

    return (0 == objects_left) ? 0 : -1;
}

int process_webcfgdbblob( blob_struct_t *bd, msgpack_object *obj )
{
    WebcfgDebug(" process_webcfgdbblob \n");
    msgpack_object_array *array = &obj->via.array;
    if( 0 < array->size )
    {
        size_t i;

        bd->entries_count = array->size;
        bd->entries = (blob_data_t *) malloc( sizeof(blob_data_t) * bd->entries_count );
        if( NULL == bd->entries )
        {
            bd->entries_count = 0;
            return -1;
        }
        
        WebcfgDebug("bd->entries_count %zu\n",bd->entries_count);
        memset( bd->entries, 0, sizeof(blob_data_t) * bd->entries_count );
        for( i = 0; i < bd->entries_count; i++ )
        {
            if( MSGPACK_OBJECT_MAP != array->ptr[i].type )
            {
                errno = BD_INVALID_BD_OBJECT;
                return -1;
            }
            if( 0 != process_webcfgdbblobparams(&bd->entries[i], &array->ptr[i].via.map) )
            {
		WebcfgError("process_webcfgdbblobparam failed\n");
                return -1;
            }
        }
    }

    return 0;
}

char * base64blobencoder(char * blob_data, size_t blob_size )
{
	char* b64buffer =  NULL;
	size_t encodeSize = -1;
   	WebcfgDebug("Data is %s\n", blob_data);
     	WebcfgDebug("-----------Start of Base64 Encode ------------\n");
        encodeSize = b64_get_encoded_buffer_size(blob_size);
        WebcfgDebug("encodeSize is %zu\n", encodeSize);
        b64buffer = malloc(encodeSize + 1);
        if(b64buffer != NULL)
        {
            memset( b64buffer, 0, sizeof( encodeSize )+1 );

            b64_encode((uint8_t *)blob_data, blob_size, (uint8_t *)b64buffer);
            b64buffer[encodeSize] = '\0' ;
        }
	return b64buffer;
}
int writebase64ToDBFile(char *base64_file_path, char *data)
{
	FILE *fp;
	fp = fopen(base64_file_path , "w+");
	if (fp == NULL)
	{
		WebcfgError("Failed to open base64_file in db %s\n", base64_file_path);
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
		WebcfgError("WriteToJson failed, Data is NULL\n");
		fclose(fp);
		return 0;
	}
}

//To get individual subdoc details from tmp cached list.
webconfig_tmp_data_t * getTmpNode(char *docname)
{
	webconfig_tmp_data_t *temp = NULL;
	temp = get_global_tmp_node();

	//Traverse through doc list & fetch required doc.
	while (NULL != temp)
	{
		WebcfgDebug("getTmpNode: temp->name %s, temp->version %lu\n",temp->name, (long)temp->version);
		if( strcmp(docname, temp->name) == 0)
		{
			WebcfgDebug("subdoc node : name %s version %lu status %s error_details %s error_code %hu trans_id %hu temp->retry_count %d temp->cloud_trans_id %s\n", temp->name, (long)temp->version, temp->status, temp->error_details, temp->error_code, temp->trans_id, temp->retry_count, temp->cloud_trans_id);
			return temp;
		}
		temp= temp->next;
	}
	WebcfgError("getTmpNode failed for doc %s\n", docname);
	return NULL;
}

//update retry_expiry_timestamp for each doc
WEBCFG_STATUS updateFailureTimeStamp(webconfig_tmp_data_t *temp, char *docname, long long timestamp)
{
	if (NULL != temp)
	{
		WebcfgDebug("updateFailureTimeStamp: node is pointing to temp->name %s \n",temp->name);
		pthread_mutex_lock (&webconfig_tmp_data_mut);
		WebcfgDebug("mutex_lock in updateFailureTimeStamp\n");
		if( strcmp(docname, temp->name) == 0)
		{
			temp->retry_expiry_timestamp = timestamp;
			WebcfgInfo("doc %s is updated with timestamp %lld\n", docname, timestamp);
			pthread_mutex_unlock (&webconfig_tmp_data_mut);
			WebcfgDebug("mutex_unlock in current temp details\n");
			return WEBCFG_SUCCESS;
		}
		pthread_mutex_unlock (&webconfig_tmp_data_mut);
		WebcfgDebug("mutex_unlock in updateTmpList\n");
	}
	WebcfgError("updateFailureTimeStamp failed as doc %s is not in tmp list\n", docname);
	return WEBCFG_FAILURE;
}
