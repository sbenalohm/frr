#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

int main(int argc, char *argv[])
{

    int welcomeSocket, newSocket;
    char buffer[1024];
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;

    welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7891);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    while (1)
    {
        if (listen(welcomeSocket, 5) == 0)
        {
            printf("Listening...\n");
        }
        else
        {
            printf("Error on listen.\n");
        }

        addr_size = sizeof serverStorage;
        newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);

        recv(newSocket, buffer, 1023, 0);
        printf("Received:\n%s\n", buffer);

        close(newSocket);        
    }

    return 0;
}