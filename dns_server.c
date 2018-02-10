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

        printf("Message Received\n");
        
        //WORK BELOW HERE
        char fn[] = "dns-master.txt";
        Resource_record *root = (Resource_record*) malloc( sizeof(Resource_record) );        
        file_to_list(fn, root);
        char search_name[] = "host11.student.test";
        Resource_record *rr = search_record_names(root, search_name);

        Dns_msg_question *qstn = (Dns_msg_question*) malloc( sizeof(Dns_msg_question) );

        dns_extract_question(dns_msg, qstn);
        
        Dns_answer *asr = (Dns_answer*) malloc( sizeof(Dns_answer) );
        dns_create_answer(dns_msg, rr, qstn, asr);
        dns_print_answer(asr);
        








    }// END while

    //Close socket
    close(sockfd);


    return 0;
}
