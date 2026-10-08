// client_dns
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
    int p;

    struct sockaddr_in sa;

    char dm[BUF];
    char res[BUF];

    if (argc != 3)
    {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    p = atoi(argv[2]);

    if (p <= 0 || p > 65535)
    {
        printf("Invalid port number\n");
        exit(EXIT_FAILURE);
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_port = htons((uint16_t)p);

    if (inet_pton(AF_INET, argv[1], &sa.sin_addr) <= 0)
    {
        printf("Invalid server IP address\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    socklen_t al = sizeof(sa);

    printf("Connected to DNS Server %s:%d\n", argv[1], p);
    printf("Type 'exit' to stop.\n\n");

    while (1)
    {
        printf("Enter Domain Name: ");

        if (fgets(dm, BUF, stdin) == NULL)
        {
            break;
        }

        dm[strcspn(dm, "\n")] = '\0';

        if (strcmp(dm, "exit") == 0)
        {
            printf("DNS client terminated.\n");
            break;
        }

        if (sendto(fd, dm, strlen(dm), 0, (struct sockaddr *)&sa, al) < 0)
        {
            perror("sendto failed");
            break;
        }

        ssize_t n = recvfrom(fd, res, BUF - 1, 0, (struct sockaddr *)&sa, &al);

        if (n < 0)
        {
            perror("recvfrom failed");
            break;
        }

        res[n] = '\0';

        printf("\nDNS Server Response:\n");
        printf("%s\n\n", res);
    }

    close(fd);

    return 0;
}
