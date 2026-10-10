#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 8080
#define BUFFER_SIZE 1024
int main()
{
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_len = sizeof(server_addr);
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("172.16.16.200");
    printf("UDP RPC SENDER\n");
    while (1)
    {
        char procedure[20];
        int n;
        printf("\nAvailable procedures:\n");
        printf("ADD - 2 arguments\n");
        printf("SUB - 2 arguments\n");
        printf("MUL - 2 arguments\n");
        printf("DIV - 2 arguments\n");
        printf("SQUARE - 1 argument\n");
        printf("EXIT - Exit\n");
        printf("\nEnter procedure: ");
        scanf("%s", procedure);
        if (strcmp(procedure, "EXIT") == 0)
            break;
        printf("Enter number of arguments: ");
        scanf("%d", &n);
        strcpy(buffer, procedure);
        for (int i = 0; i < n; i++)
        {
            char argument[50];
            printf("Enter argument %d: ", i + 1);
            scanf("%s", argument);
            strcat(buffer, " ");
            strcat(buffer, argument);
        }
        printf("\nSending request: %s\n", buffer);
        sendto(sockfd, buffer, strlen(buffer) + 1, 0,
               (struct sockaddr *)&server_addr, server_len);
        memset(response, 0, BUFFER_SIZE);
        recvfrom(sockfd, response, BUFFER_SIZE - 1, 0,
                 (struct sockaddr *)&server_addr, &server_len);
        printf("Server response: %s\n", response);
    }
    close(sockfd);
    return 0;
}
