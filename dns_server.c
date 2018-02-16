// @author Nicholas Siviglia 31360256
// @Class CS356-101 (Evening)
// @version 1.0
// @date 2018-01-31
// @assignment DNS Server

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "./dns_messages.h"

int main(int argc, char** argv){

    //Check arguments
    if(argc != 2){
    
        printf("Invalid Arguments\n ./server [port]\n");

        return 0;
    }

    //Parse master list
    char fn[] = "dns-master.txt";

    Resource_record *root = (Resource_record*) malloc( sizeof(Resource_record) );        

    assert(root != NULL);

    file_to_list(fn, root);

    //Open Socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if(sockfd == -1){

        printf("ERROR unable to open socket\n");

        return -1;
    }

    //Bind the socket
    struct sockaddr_in server;

    memset((char *) &server, 0, sizeof(server));

    server.sin_family = AF_INET;

    server.sin_addr.s_addr = htonl(INADDR_ANY);

    server.sin_port = htons( atoi(argv[1]) );

    int bind_rc = bind(sockfd, (struct sockaddr *) &server, sizeof(server));

    if(bind_rc){

        printf("ERROR unable to bind socket\n");

        return -1;
    }

    //Wait for connection from client
    struct sockaddr_in client;

    socklen_t client_len = sizeof(client);

    ssize_t msg_rc = 0;

    unsigned char dns_msg[255];

    printf("The server is ready to receive on port: %s\n", argv[1]);

    while(true){
        
        //Receive request 
        msg_rc = recvfrom(sockfd, dns_msg, sizeof(dns_msg), 0, (struct sockaddr *) &client, &client_len);

        if(msg_rc == -1){
            
            printf("ERROR receiving message\n");

            return -1;
        }

        //Get request header
        Dns_msg_header *msg_header = (Dns_msg_header*) dns_msg;

        //Get msg size
        size_t msg_size = dns_get_request_msg_size(dns_msg);

        printf("Message Received\n");
        
        //Retrieve question
        Dns_msg_question qstn;

        dns_extract_question(dns_msg, &qstn);

        //Search for question in list
        char qname[255]; 

        dns_qname_to_str(&qstn, qname);

        printf("searching for: %s\n", qname);

        Resource_record *rr = search_record_names(root, qname);

        //Record not found
        if(rr == NULL){

            printf("Record not found\n");
            
            msg_header->qr = 1;

            msg_header->ra = 1;

            msg_header->rcode = 3;
        }
        else{
            
            //Create answer
            Dns_answer asr;

            dns_create_answer(dns_msg, rr, &qstn, &asr);

            //Insert Answer
            dns_insert_answer(&asr, dns_msg, &msg_size);

            //Set msg header
            msg_header->ra = 1;
            
            msg_header->rd = 1;

            msg_header->qr = 1;

            msg_header->aa = 1;

            msg_header->ar_count = htons(0);

            msg_header->an_count = htons(1);

            //Delete answer
            dns_delete_answer(&asr);
        }

        //Send Response  
        msg_rc = sendto(sockfd, dns_msg, msg_size, 0, (struct sockaddr *) &client, client_len);

        if(msg_rc == -1){
            
            printf("ERROR sending message\n");

            return -1;
        }

        //Clean up
        dns_delete_question(&qstn);

        printf("Clean up complete\n");

    }// END while

    //Close socket
    close(sockfd);

    return 0;
}
