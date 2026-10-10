[24bcs137@mepcolinux ex9]$cat remrec.c
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
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(sockfd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(1);
    }
    printf("UDP RPC RECEIVER\n");
    printf("Server waiting for requests...\n");
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                 (struct sockaddr *)&client_addr, &client_len);
        printf("\nReceived request: %s\n", buffer);
        char procedure[20];
        char arg1[20], arg2[20];
        int count = sscanf(buffer, "%s %s %s",
                           procedure, arg1, arg2);
        int expected_args = 0;
        int a, b, result;
        if (strcmp(procedure, "ADD") == 0 ||
            strcmp(procedure, "SUB") == 0 ||
            strcmp(procedure, "MUL") == 0 ||
            strcmp(procedure, "DIV") == 0)
        {
            expected_args = 2;
        }
        else if (strcmp(procedure, "SQUARE") == 0)
        {
            expected_args = 1;
        }
        else
        {
            strcpy(response, "ERROR: Unknown procedure");
            sendto(sockfd, response, strlen(response) + 1, 0,
                   (struct sockaddr *)&client_addr, client_len);
            continue;
        }
        if (count - 1 != expected_args)
        {
            sprintf(response,
                    "ERROR: Wrong number of arguments %s requires %d argument",
                    procedure, expected_args);
            sendto(sockfd, response, strlen(response) + 1, 0,
                   (struct sockaddr *)&client_addr, client_len);

            continue;
        }
        if (expected_args == 2)
        {
            if (sscanf(arg1, "%d", &a) != 1 ||
                sscanf(arg2, "%d", &b) != 1)
            {
                strcpy(response, "ERROR: Arguments must be integers");
                sendto(sockfd, response, strlen(response) + 1, 0,
                       (struct sockaddr *)&client_addr, client_len);
                continue;
            }
            if (strcmp(procedure, "ADD") == 0)
            {
                result = a + b;
            }
            else if (strcmp(procedure, "SUB") == 0)
            {
                result = a - b;
            }
            else if (strcmp(procedure, "MUL") == 0)
            {
                result = a * b;
            }
            else
            {
                if (b == 0)
                {
                    strcpy(response, "ERROR: Division by zero");
                    sendto(sockfd, response, strlen(response) + 1, 0,
                           (struct sockaddr *)&client_addr, client_len);
                    continue;
                }
                result = a / b;
            }
            sprintf(response, "Result = %d", result);
        }
        else
        {
            if (sscanf(arg1, "%d", &a) != 1)
            {
                strcpy(response, "ERROR: Argument must be an integer");
                sendto(sockfd, response, strlen(response) + 1, 0,
                       (struct sockaddr *)&client_addr, client_len);
                continue;
            }
            result = a * a;
            sprintf(response, "Result = %d", result);
        }
        sendto(sockfd, response, strlen(response) + 1, 0,
               (struct sockaddr *)&client_addr, client_len);
        printf("Sent response: %s\n", response);
    }
    close(sockfd);
    return 0;
}
[24bcs137@mepcolinux ex9]$cat remsend.c
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

[24bcs137@mepcolinux ex9]$s1
UDP RPC SENDER

Available procedures:
ADD - 2 arguments
SUB - 2 arguments
MUL - 2 arguments
DIV - 2 arguments
SQUARE - 1 argument
EXIT - Exit

Enter procedure: ADD
Enter number of arguments: 2
Enter argument 1: 34
Enter argument 2: 43

Sending request: ADD 34 43
Server response: Result = 77

Available procedures:
ADD - 2 arguments
SUB - 2 arguments
MUL - 2 arguments
DIV - 2 arguments
SQUARE - 1 argument
EXIT - Exit

Enter procedure: SQUARE
Enter number of arguments: 2
Enter argument 1: 12
Enter argument 2: 32

Sending request: SQUARE 12 32
Server response: ERROR: Wrong number of arguments SQUARE requires 1 argument

Available procedures:
ADD - 2 arguments
SUB - 2 arguments
MUL - 2 arguments
DIV - 2 arguments
SQUARE - 1 argument
EXIT - Exit

Enter procedure: POWER
Enter number of arguments: 1
Enter argument 1: 23

Sending request: POWER 23
Server response: ERROR: Unknown procedure

Available procedures:
ADD - 2 arguments
SUB - 2 arguments
MUL - 2 arguments
DIV - 2 arguments
SQUARE - 1 argument
EXIT - Exit

Enter procedure: EXIT
[24bcs137@mepcolinux ex9]$./r1
UDP RPC RECEIVER
Server waiting for requests...

Received request: ADD 34 43
Sent response: Result = 77

Received request: SQUARE 12 32

Received request: POWER 23
