// client_dhcp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF 4096

int main(int argc, char *argv[])
{
    int fd;
    int p;

    struct sockaddr_in sa;

    char blk[50];
    char msg[BUF];
    char res[BUF];

    int sc;

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

    printf("Connected to DHCP Server %s:%d\n\n", argv[1], p);

    printf("Enter IP block: ");
    scanf("%49s", blk);

    printf("Enter number of subnets: ");
    scanf("%d", &sc);

    sprintf(msg, "SETUP %s %d", blk, sc);

    if (sendto(fd, msg, strlen(msg), 0, (struct sockaddr *)&sa, al) < 0)
    {
        perror("sendto failed");
        close(fd);
        exit(EXIT_FAILURE);
    }

    ssize_t n = recvfrom(fd, res, BUF - 1, 0, NULL, NULL);

    if (n < 0)
    {
        perror("recvfrom failed");
        close(fd);
        exit(EXIT_FAILURE);
    }

    res[n] = '\0';

    printf("\n%s\n", res);

    while (1)
    {
        int sn;
        int req;

        printf("Enter subnet number (0 to exit): ");
        scanf("%d", &sn);

        if (sn == 0)
        {
            strcpy(msg, "EXIT");

            sendto(fd, msg, strlen(msg), 0, (struct sockaddr *)&sa, al);

            n = recvfrom(fd, res, BUF - 1, 0, NULL, NULL);

            if (n >= 0)
            {
                res[n] = '\0';
                printf("\n%s\n", res);
            }

            break;
        }

        printf("Enter number of IP addresses to allot: ");
        scanf("%d", &req);

        sprintf(msg, "ALLOCATE %d %d", sn, req);

        if (sendto(fd, msg, strlen(msg), 0, (struct sockaddr *)&sa, al) < 0)
        {
            perror("sendto failed");
            break;
        }

        n = recvfrom(fd, res, BUF - 1, 0, NULL, NULL);

        if (n < 0)
        {
            perror("recvfrom failed");
            break;
        }

        res[n] = '\0';

        printf("\nDHCP Server Response:\n");
        printf("%s\n\n", res);
    }

    close(fd);

    return 0;
}
