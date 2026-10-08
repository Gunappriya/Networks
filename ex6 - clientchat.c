// client_chat
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
    char b[BUF];
    int p;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    p = atoi(argv[2]);

    if (p <= 0 || p > 65535)
    {
        fprintf(stderr, "Invalid port number: %s\n", argv[2]);
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

    if (inet_pton(AF_INET, argv[1], &sa.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid server IP address: %s\n", argv[1]);
        close(fd);
        exit(EXIT_FAILURE);
    }

    socklen_t al = sizeof(sa);

    printf("Connected to chat server at %s:%d\n", argv[1], p);
    printf("Type 'exit' to end the chat.\n\n");

    while (1)
    {
        printf("You: ");

        if (fgets(b, BUF, stdin) == NULL)
        {
            strcpy(b, "exit");
        }

        b[strcspn(b, "\n")] = '\0';

        if (sendto(fd, b, strlen(b), 0, (struct sockaddr *)&sa, al) < 0)
        {
            perror("sendto failed");
            break;
        }

        if (strcmp(b, "exit") == 0)
        {
            printf("You ended the chat.\n");
            break;
        }

        ssize_t n = recvfrom(fd, b, BUF - 1, 0, (struct sockaddr *)&sa, &al);

        if (n < 0)
        {
            perror("recvfrom failed");
            break;
        }

        b[n] = '\0';

        printf("Server: %s\n", b);

        if (strcmp(b, "exit") == 0)
        {
            printf("Server ended the chat.\n");
            break;
        }
    }

    close(fd);

    return 0;
}
