#ifndef DNS_MESSAGES
#define DNS_MESSAGES 
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "./dns_resource_records.h"

#endif

/*
 * This struct is done according to RFC 1035, AND accounts for the CD, AD, bits 
 * as per RFC 4035 (page 17). To take into accound of network byte order the single 
 * bit wide flags are switched around.
 */
typedef struct Dns_msg_header{

    uint16_t id;

    uint8_t rd :1;
    uint8_t tc :1;
    uint8_t aa :1;
    uint8_t opcode :4;
    uint8_t qr :1;

    uint8_t rcode :4;
    uint8_t cd :1;
    uint8_t ad :1;
    uint8_t z :1;
    uint8_t ra :1;

    uint16_t qd_count;
    uint16_t an_count;
    uint16_t ns_count;
    uint16_t ar_count;

} Dns_msg_header;

typedef struct Dns_msg_question{

    unsigned char *qname;
    size_t qname_len;
    uint16_t qtype;
    uint16_t qclass;

} Dns_msg_question;

typedef struct Dns_answer{

    uint16_t id;
    uint16_t atype;
    uint16_t aclass;
    uint32_t ttl;
    uint16_t rdata_len;
    uint8_t *rdata;

} Dns_answer;

//DNS message offsets in bytes

uint16_t dns_type_char_to_uint16(char* ch_type){
    
    if( strcmp(ch_type, "A") == 0 ){
         
        return 1;
    }

    if( strcmp(ch_type, "NS") == 0 ){
         
        return 2;
    }

    if( strcmp(ch_type, "CNAME") == 0 ){
         
        return 5;
    }

    return 0;
}

void print_unsigned_chars_in_hex(unsigned char *str){

    size_t len = strlen( (char*) str);

    for(size_t i=0; i < len; i++){

        printf("%x ", str[i]);
    }

    printf("\n");
}

uint16_t dns_generate_name_pointer(const unsigned char *dns_msg, const Dns_msg_question *dns_qstn){
    
    uint8_t first_letter = dns_qstn->qname[1]; //skip char-count number

    size_t msg_length = sizeof(char) * 255; //TO-DO

    uint16_t index = 0;

    //Find index of qname in dns message
    for(size_t i=0; i < msg_length; i++){        

        if( dns_msg[i] == first_letter && 
                strcmp((char*) &dns_msg[i], (char*) &dns_qstn->qname[1]) == 0){
            
                index = &dns_msg[i] - dns_msg - 1; //Remove 1 because we need char-count 

                break;
        }
    }

    //Add '11' bit's to beginning of name pointer.
    if(index != 0){

        index = (index | 0xC000);
    }

    return index;
}

void dns_ip_str_to_uint8(char *ip_str, uint8_t *ip_uint8){

    size_t index = 0;

    char *token =  strtok(ip_str, ".");

    while( token != NULL ){

        int num = atoi( token );

        assert(num < UINT8_MAX);
        
        ip_uint8[index] = num;

        index += 1;

        token = strtok(NULL, ".");
    }

}

char *dns_str_to_qname(const char *str){

    //qname has 1 extra char to represent the first char count
    char *qname = (char*) malloc( sizeof(char) * (strlen(str) + 1) );
    
    //strings end with zero
    size_t count = -1;

    printf("str: %s\n\n", str);

    for( int i=strlen(str); i >= 0; i--){

        printf("%x ", str[i]);

        if(str[i] == '.'){
            
            qname[i+1] = count;

            count = 0;
        }
        else{
            
            qname[i+1] = str[i];

            count += 1;
        }
    }

    printf("\n");

    qname[0] = count;

    return qname;
}

void dns_qname_to_str(const Dns_msg_question *dns_qstn, char *qname){
    
    strcpy(qname, (char*) &dns_qstn->qname[1]);

    size_t letter_count = (size_t) dns_qstn->qname[0];  

    size_t index = 0;

    while (true){

        index += letter_count;

        letter_count = qname[index];

        if(letter_count == 0){
            
            break;
        }
       
        qname[index] = '.';

        index += 1;
    }
}


void dns_print_msg_header(const Dns_msg_header *dns_header){
    
    printf("---DNS MESSAGE HEADER---\n");

    printf("ID: %x\n", dns_header->id);

    printf("QR: %hhu \nOPCODE: %hhu \nAA: %hhu\n", dns_header->qr, dns_header->opcode, dns_header->aa);

    printf("TC: %hhu \nRD: %hhu \nRA: %hhu\n", dns_header->tc, dns_header->rd, dns_header->ra);

    printf("Z: %hhu \nRCODE: %hhu \nQDCOUNT: %hu\n", dns_header->z, dns_header->rcode, ntohs(dns_header->qd_count));

    printf("ANCOUNT: %hu \nNSCOUNT: %hu \nARCOUNT: %hu\n\n", 
            ntohs(dns_header->an_count),ntohs(dns_header->ns_count), ntohs(dns_header->ar_count));

}

void dns_extract_question(const unsigned char *dns_msg, Dns_msg_question *dns_qstn){
    
    //Grab qname
    const size_t qname_loc = 12;

    dns_qstn->qname_len = strlen( (char*) &dns_msg[qname_loc]) + 1;

    dns_qstn->qname = (unsigned char*) malloc( dns_qstn->qname_len * sizeof(char) );

    assert(dns_qstn->qname != NULL);
    
    memcpy(dns_qstn->qname, &dns_msg[qname_loc], dns_qstn->qname_len);

    printf("qname: %s\n", dns_qstn->qname);

    //Insert type. All questions are going to be 'A' 
    dns_qstn->qtype = 1;

    //Insert class. All questions are going to be 'IN'
    dns_qstn->qclass = 1;
}

void dns_create_answer(const unsigned char *dns_msg, const Resource_record *rr, const Dns_msg_question *dns_qstn, Dns_answer *asr){
    
    asr->aclass = 1;
    
    asr->ttl = rr->ttl;

    asr->atype = dns_type_char_to_uint16(rr->type);

    asr->id = dns_generate_name_pointer(dns_msg, dns_qstn); 
    
    //Create rdata 
    if(asr->atype == 1){ //Type 'A'

        asr->rdata = (uint8_t *) malloc( 4 );    

        assert(asr->rdata != NULL);
        
        dns_ip_str_to_uint8(rr->location, asr->rdata);

        asr->rdata_len = 4;
    }
    else if(asr->atype == 2 || asr->atype == 5){ //Type 'NS' or 'CNAME'

        asr->rdata = (unsigned char*) dns_str_to_qname(rr->location);

        // Plus 1 to account for zero at end of string
        asr->rdata_len = sizeof(char) * (strlen( (char*) asr->rdata) + 1);

    }
    else{
        printf("Invalid Answer Type\n");

        exit(-1);
    }
}

void dns_print_answer(const Dns_answer *asr){

    printf("class: %hu \ntype: %hu \nid: %x \nttl: %u \n", asr->aclass, asr->atype, asr->id, asr->ttl);

    printf("rdata_len: %hu\n", asr->rdata_len);

    printf("rdata: ");

    for(size_t i=0; i < (asr->rdata_len); i++){
        
        printf("%x ", asr->rdata[i]);
    }

    printf("\n");
}

size_t dns_get_request_msg_size(unsigned char *dns_msg){
    
    //From the beginning of qname add the length of it 
    //+ zero at end of str + type and class + 11 bytes for the 'Additional Records' section.
    unsigned char *end_of_message = &dns_msg[12] + strlen((char*) &dns_msg[12]) 
        + 1 + sizeof(uint32_t) + (sizeof(uint8_t) * 11);

    size_t size = end_of_message - dns_msg;

    return size;
}

void dns_insert_answer(const Dns_answer *asr, unsigned char *dns_msg, size_t *msg_size){
    
    //Aditional records section is 11 char's at the end of the message
    const uint8_t AR_DATA_SIZE = 11;

    uint8_t *msg_ptr = &dns_msg[*msg_size - AR_DATA_SIZE];
  
    /*
    //Backup ar data
    uint8_t ar_data[AR_DATA_SIZE];

    memcpy(msg_ptr, ar_data, AR_DATA_SIZE);
    */

    //Copy id
    uint16_t u16_temp = htons(asr->id);

    memcpy(msg_ptr, &u16_temp, sizeof(asr->id) );

    msg_ptr = msg_ptr + sizeof(asr->id);
    
    //Copy type 
    u16_temp = htons(asr->atype);

    memcpy(msg_ptr, &u16_temp, sizeof(asr->atype) );

    msg_ptr = msg_ptr + sizeof(asr->atype);

    //Copy class
    u16_temp = htons(asr->aclass);

    memcpy(msg_ptr, &u16_temp, sizeof(asr->aclass) );

    msg_ptr = msg_ptr + sizeof(asr->aclass);

    //Copy ttl
    uint32_t u32_temp = htonl(asr->ttl);

    memcpy(msg_ptr, &u32_temp, sizeof(asr->ttl) );

    msg_ptr = msg_ptr + sizeof(asr->ttl);

    //Copy rdata length
    u16_temp = htons(asr->rdata_len);

    memcpy(msg_ptr, &u16_temp, sizeof(asr->rdata_len) );

    msg_ptr = msg_ptr + sizeof(asr->rdata_len);

    //Copy rdata
    memcpy(msg_ptr, asr->rdata, asr->rdata_len);

    msg_ptr = msg_ptr + asr->rdata_len;

    //Get new message size
    *msg_size = msg_ptr - dns_msg;
}

void dns_delete_question(Dns_msg_question *dns_qstn){

    free(dns_qstn->qname);
}

void dns_delete_answer(Dns_answer *dns_asr){

    free(dns_asr->rdata);
}

