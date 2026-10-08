  #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    int sockfd, n;
    char buffer[1024];
    struct sockaddr_in server_addr;

    if (argc != 4)
    {
        printf("Usage: %s <server_ip> <port> <message>\n", *argv);
        return 1;
    }

    char *server_ip = *(argv + 1);
    char *port_str  = *(argv + 2);
    char *message   = *(argv + 3);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port_str));

    int status = inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
    if (status == 0)
    {
        fprintf(stderr, "Error: Invalid IPv4 address '%s'\n", server_ip);
        close(sockfd);
        return 1;
    }
    else if (status < 0)
    {
        perror("inet_pton");
        close(sockfd);
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        return 1;
    }

    send(sockfd, message, strlen(message), 0);

    n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (n > 0)
    {
        buffer[n] = '\0';
        printf("Echo from server: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}
