#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdbool.h>

#include "exit-codes.h"

#define FIFO_NAME "/tmp/test_fifo"

int main(){

    printf("MQTT Client Started\n");

    printf("Waiting for a connection");

    int fd = open(FIFO_NAME, O_RDONLY);
    if (fd == -1) {
        perror("Error opening FIFO");
        return OS_EXIT_GEN_FAILURE;
    }

    printf("Writer connected\n");
    
    char buffer[256];
    ssize_t bytesRead;

    while ((bytesRead = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        printf("Received: %s", buffer);
    }

    if (bytesRead == -1) {
        perror("Error reading from FIFO");
        close(fd);
        return OS_EXIT_GEN_FAILURE;
    } else {
        printf("End of file (writer closed connection)\n");
    }

    close(fd); 

    printf("%s", buffer);


    return OS_EXIT_SUCCESS;
}

