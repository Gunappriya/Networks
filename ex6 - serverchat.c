// server_chat
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF 1024

int main(int argc, char *argv[])
{
    int fd;
    struct sockaddr_in sa;
    struct sockaddr_in ca;

    char b[BUF];
    int p;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    p = atoi(argv[1]);

    if (p <= 0 || p > 65535)
    {
        fprintf(stderr, "Invalid port number: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_port = htons((uint16_t)p);
    sa.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
    {
        perror("bind failed");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("UDP Chat Server listening on port %d...\n", p);
    printf("Server is ready for multiple clients.\n\n");

    while (1)
    {
        socklen_t al = sizeof(ca);

        ssize_t n = recvfrom(fd, b, BUF - 1, 0, (struct sockaddr *)&ca, &al);

        if (n < 0)
        {
            perror("recvfrom failed");
            continue;
        }

        b[n] = '\0';

        printf("Client [%s:%d]: %s\n",
               inet_ntoa(ca.sin_addr),
               ntohs(ca.sin_port),
               b);

        if (strcmp(b, "exit") == 0)
        {
            printf("Client ended the chat.\n");
            printf("Waiting for another client...\n\n");
            continue;
        }

        printf("You: ");

        if (fgets(b, BUF, stdin) == NULL)
        {
            strcpy(b, "exit");
        }

        b[strcspn(b, "\n")] = '\0';

        if (sendto(fd, b, strlen(b), 0, (struct sockaddr *)&ca, al) < 0)
        {
            perror("sendto failed");
            continue;
        }

        if (strcmp(b, "exit") == 0)
        {
            printf("Chat with this client ended.\n");
            printf("Waiting for another client...\n\n");
        }

        printf("\n");
    }

    close(fd);

    return 0;
}
