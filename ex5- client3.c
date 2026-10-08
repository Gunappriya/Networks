  #include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

int sockfd, clientfd;
struct sockaddr_in server, client;

char str1[100];
char str2[100];
char result[256];

void createServer()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        return;
    }
    printf("Server socket created\n");
}

void bindServer()
{
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Bind failed\n");
        return;
    }
    printf("Server bind successful\n");
}

void waitClient()
{
    listen(sockfd, 5);
    printf("Waiting for client...\n");

    socklen_t len = sizeof(client);
    clientfd = accept(sockfd, (struct sockaddr *)&client, &len);

    if (clientfd < 0)
    {
        printf("Client connection failed\n");
        return;
    }
    printf("Client connected\n");
}

void receiveData()
{
    memset(str1, 0, sizeof(str1));
    memset(str2, 0, sizeof(str2));

    recv(clientfd, str1, sizeof(str1), 0);
    recv(clientfd, str2, sizeof(str2), 0);

    printf("\nReceived String 1: \"%s\"\n", str1);
    printf("Received String 2: \"%s\"\n", str2);
}

void compareStrings()
{
    int i = 0;
    int is_equal = 1;

    while (str1[i] != '\0' || str2[i] != '\0')
    {
        if (str1[i] != str2[i])
        {
            is_equal = 0;
            sprintf(result, "Strings are NOT equal (Mismatch at index %d: '%c' vs '%c')",
                    i,
                    str1[i] == '\0' ? ' ' : str1[i],
                    str2[i] == '\0' ? ' ' : str2[i]);
            break;
        }
        i++;
    }

    if (is_equal)
    {
        sprintf(result, "Strings are EQUAL (All %d characters matched successfully)", i);
    }

    printf("Comparison Result: %s\n", result);
}

void sendResult()
{
    send(clientfd, result, strlen(result) + 1, 0);
    printf("Result sent to client\n");
}

void closeServer()
{
    close(clientfd);
    close(sockfd);
    printf("Connection closed\n");
}

int main()
{
    createServer();
    bindServer();
    waitClient();
    receiveData();
    compareStrings();
    sendResult();
    closeServer();

    return 0;
}
