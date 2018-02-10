// @author Nicholas Siviglia 31360256
// @Class CS356-101 (Evening)
// @version 1.0
// @date 2018-01-31
// @assignment DNS Server

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define true 1
#define false 0

const int DROP_RATE_PRCNT = 4;

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
    
    //Setup random numbers for dropped pings
    srand(time(NULL));

    //Wait for connection from client
    struct sockaddr_in client;

    socklen_t client_len = sizeof(client);

    ssize_t msg_rc = 0;

    unsigned int recv_msg[2] = {0, 0};

    printf("The server is ready to receive on port: %s\n", argv[1]);
    while(true){
        
        //Receive request 
        msg_rc = recvfrom(sockfd, recv_msg, sizeof(recv_msg), 0, (struct sockaddr *) &client, &client_len);

        if(msg_rc == -1){
            
            printf("ERROR receiving message\n");

            return -1;
        }

        //Check if message needs to be dropped
        if( (rand() % 10 + 1) < DROP_RATE_PRCNT){

            printf("Message with sequence number %d dropped\n", recv_msg[1]); 

            continue;
        }

        //Send response
        printf("Responding to ping request with sequence number %d\n", recv_msg[1]);

        recv_msg[0] = 2;

        msg_rc = sendto(sockfd, recv_msg, sizeof(recv_msg), 0, (struct sockaddr *) &client, client_len);

        if(msg_rc == -1){
            
            printf("ERROR sending message\n");

            return -1;
        }

    }

    //Close socket
    close(sockfd);

    return 0;
}
