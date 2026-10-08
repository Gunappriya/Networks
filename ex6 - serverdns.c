// server_dns
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF 1024
#define TSZ 10

typedef struct Nd
{
    char dm[100];
    char ip[50];
    struct Nd *nxt;
} Nd;

Nd *tbl[TSZ];

int hf(char *dm)
{
    int h = 0;

    for (int i = 0; dm[i] != '\0'; i++)
    {
        h = (h + dm[i]) % TSZ;
    }

    return h;
}

void ins(char *dm, char *ip)
{
    int idx = hf(dm);

    Nd *nn = (Nd *)malloc(sizeof(Nd));

    strcpy(nn->dm, dm);
    strcpy(nn->ip, ip);

    nn->nxt = tbl[idx];
    tbl[idx] = nn;
}

char *srch(char *dm)
{
    int idx = hf(dm);

    Nd *cur = tbl[idx];

    while (cur != NULL)
    {
        if (strcmp(cur->dm, dm) == 0)
        {
            return cur->ip;
        }

        cur = cur->nxt;
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    int fd;
    int p;

    struct sockaddr_in sa;
    struct sockaddr_in ca;

    char dm[BUF];
    char res[BUF];

    for (int i = 0; i < TSZ; i++)
    {
        tbl[i] = NULL;
    }

    ins("google.com", "142.250.195.14");
    ins("youtube.com", "142.250.72.206");
    ins("facebook.com", "157.240.241.35");
    ins("example.com", "93.184.216.34");
    ins("amazon.com", "98.137.11.163");
    ins("github.com", "140.82.114.4");
    ins("wikipedia.org", "208.80.154.224");
    ins("instagram.com", "157.240.241.174");
    ins("microsoft.com", "20.112.250.133");
    ins("apple.com", "17.253.144.10");

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    p = atoi(argv[1]);

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
    sa.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
    {
        perror("Bind failed");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("DNS UDP Server started on port %d\n", p);
    printf("Hash table size: %d\n", TSZ);
    printf("Server is ready for multiple clients.\n\n");

    while (1)
    {
        socklen_t al = sizeof(ca);

        ssize_t n = recvfrom(fd, dm, BUF - 1, 0, (struct sockaddr *)&ca, &al);

        if (n < 0)
        {
            perror("recvfrom failed");
            continue;
        }

        dm[n] = '\0';

        printf("Client [%s:%d] requested: %s\n",
               inet_ntoa(ca.sin_addr),
               ntohs(ca.sin_port),
               dm);

        char *ip = srch(dm);

        if (ip != NULL)
        {
            snprintf(res, BUF, "Domain: %s\nIP Address: %s", dm, ip);
        }
        else
        {
            snprintf(res, BUF, "Domain not found");
        }

        printf("Result: %s\n\n", res);

        if (sendto(fd, res, strlen(res), 0, (struct sockaddr *)&ca, al) < 0)
        {
            perror("sendto failed");
        }
    }

    close(fd);

    return 0;
}
