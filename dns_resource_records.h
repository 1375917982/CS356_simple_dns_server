#ifndef DNS_RESOURCE_RECORDS
#define DNS_RESOURCE_RECORDS
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#endif

//Global settings
static const char COMMENT_LINE = '#';

static char *record_auth_domain = NULL;

static int record_global_ttl = -1;

//DNS record types
static const short RECORD_TYPE_A = 1;

static const short RECORD_TYPE_NS = 2;

static const short RECORD_TYPE_CNAME = 5;

//Structs
typedef struct Resource_record{

    char *name;

    char  *type;

    int ttl;

    char *location;

    struct Resource_record *next;

} Resource_record; 

void print_record(Resource_record *r){
    
    if(r->name != NULL){
        
        printf("Name: %s\n Type: %s\n TTL: %d\n Loc: %s\n\n"
            , r->name, r->type, r->ttl, r->location);
    }

}

void print_all_records(Resource_record *head){
    
    while(head != NULL){
        
        print_record(head);

        head = head->next;
    }
}

void insert_record(Resource_record *head, Resource_record *new_record){

    //Put new record at the end of the list
    while(head->next != NULL){
        
        head = head->next;
    }
    
    head->next = new_record;
}

Resource_record *search_record_names(Resource_record *head, char *find_name){

    while(head != NULL){

        if(head->name && strcmp(head->name, find_name) == 0){
            
            return head;
        }
        else{
            
            head = head->next;
        }
    }

    return NULL;
}

void file_to_list(char *filename, Resource_record *head){
    
    //Open the file for reading
    FILE *fp;

    fp = fopen(filename, "r");

    if(fp == NULL){

        perror("Unable to open file");

        exit(EXIT_FAILURE);
    }

    //Read line from file and split it into tokens
    char *token;

    const char *delim = "\t\n";

    ssize_t num_of_chars;

    size_t bufsize = 255;

	char *buffer = (char *)malloc(bufsize * sizeof(char));

    assert(buffer != NULL);
    
    while( (num_of_chars = getline(&buffer, &bufsize, fp)) != -1 ){
        
        //Check and ignore comment line
        if(buffer[0] == COMMENT_LINE){

            continue;
        }

        //Split line into tokens
        token = strtok(buffer, delim);     

        while( token != NULL ){

            //Get auth domain
            if(record_auth_domain == NULL){
                
                record_auth_domain = (char *) malloc(strlen(token) );

                assert(record_auth_domain != NULL);

                strcpy(record_auth_domain, token);
            }

            //Get global TTL
            else if(record_global_ttl == -1){
                
                record_global_ttl = atoi(token);
            }

            //Create and add new record
            else{
                
                Resource_record *new_record = (Resource_record *) malloc( sizeof(Resource_record) );

                assert(new_record != NULL);

                //Set Name
                new_record->name = (char *) malloc( sizeof(strlen(token)) );

                assert(new_record->name != NULL);

                strcpy(new_record->name, token);

                if( (token = strtok(NULL, delim)) == NULL ){
                    
                    printf("Error reading master file\n");

                    exit(EXIT_FAILURE);
                }

                //Set type
                new_record->type = (char *) malloc( sizeof(strlen(token)) );

                assert(new_record->type != NULL);
                
                strcpy(new_record->type, token);

                if( (token = strtok(NULL, delim)) == NULL ){
                    
                    printf("Error reading master file\n");

                    exit(EXIT_FAILURE);
                }

                //Set location
                new_record->location = (char *) malloc( sizeof(strlen(token)) );

                assert(new_record->name != NULL);

                strcpy(new_record->location, token);

                //Set ttl
                new_record->ttl = record_global_ttl;

                //Add to list
                insert_record(head, new_record);
            }

            token = strtok(NULL, delim);
        }

    }

    //Clean up
    free(buffer);

    fclose(fp);
}

