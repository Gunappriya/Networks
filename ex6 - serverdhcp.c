// server_dhcp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF 4096
#define MAXSUB 256
#define MAXHOST 65534

typedef struct
{
    uint32_t nw;
    uint32_t fh;
    uint32_t lh;
    uint32_t bc;
    int th;
    int alc;
    int usd[MAXHOST];
} Sub;

uint32_t ip2i(char *ip)
{
    unsigned int a, b, c, d;

    sscanf(ip, "%u.%u.%u.%u", &a, &b, &c, &d);

    return (a << 24) | (b << 16) | (c << 8) | d;
}

void i2ip(uint32_t ip, char *str)
{
    sprintf(str, "%u.%u.%u.%u",
            (ip >> 24) & 255,
            (ip >> 16) & 255,
            (ip >> 8) & 255,
            ip & 255);
}

int pw2(int x)
{
    int r = 1;

    for (int i = 0; i < x; i++)
    {
        r = r * 2;
    }

    return r;
}

int fbit(int sc)
{
    int x = 0;

    while (pw2(x) < sc)
    {
        x++;
    }

    return x;
}

int main(int argc, char *argv[])
{
    int fd;
    int p;

    struct sockaddr_in sa;
    struct sockaddr_in ca;

    char buf[BUF];
    char res[BUF];

    static Sub sub[MAXSUB];

    int sc = 0;
    int pfx = 0;
    int npfx = 0;
    int hb = 0;

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

    printf("DHCP UDP Server started on port %d\n", p);
    printf("Server is ready for clients.\n\n");

    while (1)
    {
        socklen_t al = sizeof(ca);

        ssize_t n = recvfrom(fd, buf, BUF - 1, 0, (struct sockaddr *)&ca, &al);

        if (n < 0)
        {
            perror("recvfrom failed");
            continue;
        }

        buf[n] = '\0';

        if (strncmp(buf, "SETUP", 5) == 0)
        {
            char ip[50];
            int rs;

            if (sscanf(buf, "SETUP %49[^/]/%d %d", ip, &pfx, &rs) != 3)
            {
                strcpy(res, "Invalid SETUP format.");

                sendto(fd, res, strlen(res), 0, (struct sockaddr *)&ca, al);

                continue;
            }

            if (rs <= 0 || rs > MAXSUB)
            {
                strcpy(res, "Invalid number of subnets.");

                sendto(fd, res, strlen(res), 0, (struct sockaddr *)&ca, al);

                continue;
            }

            sc = rs;

            int bb = fbit(sc);

            npfx = pfx + bb;

            if (pfx < 0 || pfx > 30 || npfx > 30)
            {
                strcpy(res, "Invalid IP prefix.");

                sendto(fd, res, strlen(res), 0, (struct sockaddr *)&ca, al);

                continue;
            }

            hb = 32 - npfx;

            uint32_t aps = pw2(hb);

            uint32_t bip = ip2i(ip);

            printf("Client [%s:%d]\n",
                   inet_ntoa(ca.sin_addr),
                   ntohs(ca.sin_port));

            printf("IP Block: %s/%d\n", ip, pfx);
            printf("Required Subnets: %d\n", sc);
            printf("Borrowed Bits: %d\n", bb);
            printf("New Prefix: /%d\n\n", npfx);

            for (int i = 0; i < sc; i++)
            {
                sub[i].nw = bip + (i * aps);
                sub[i].bc = sub[i].nw + aps - 1;
                sub[i].fh = sub[i].nw + 1;
                sub[i].lh = sub[i].bc - 1;
                sub[i].th = aps - 2;
                sub[i].alc = 0;

                for (int j = 0; j < sub[i].th; j++)
                {
                    sub[i].usd[j] = 0;
                }
            }

            strcpy(res, "Subnet calculation:\n\n");

            for (int i = 0; i < sc; i++)
            {
                char ntw[30];
                char fst[30];
                char lst[30];
                char bcst[30];

                i2ip(sub[i].nw, ntw);
                i2ip(sub[i].fh, fst);
                i2ip(sub[i].lh, lst);
                i2ip(sub[i].bc, bcst);

                char tmp[300];

                sprintf(tmp,
                        "Subnet %d: %s/%d\n"
                        "Network: %s\n"
                        "First Host: %s\n"
                        "Last Host: %s\n"
                        "Broadcast: %s\n"
                        "Usable Hosts: %d\n\n",
                        i + 1,
                        ntw,
                        npfx,
                        ntw,
                        fst,
                        lst,
                        bcst,
                        sub[i].th);

                if (strlen(res) + strlen(tmp) < BUF)
                {
                    strcat(res, tmp);
                }
                else
                {
                    strcat(res, "\nResponse too large to display completely.\n");
                    break;
                }
            }

            printf("%s", res);

            sendto(fd, res, strlen(res), 0, (struct sockaddr *)&ca, al);
        }

        else if (strncmp(buf, "ALLOCATE", 8) == 0)
        {
            int sn;
            int req;

            if (sscanf(buf, "ALLOCATE %d %d", &sn, &req) != 2)
            {
                strcpy(res, "Invalid ALLOCATE format.");

                sendto(fd, res, strlen(res), 0, (struct sockaddr *)&ca, al);

                continue;
            }

            printf("Allocation request:\n");
            printf("Subnet: %d\n", sn);
            printf("Required IPs: %d\n", req);

            if (sn < 1 || sn > sc)
            {
                strcpy(res, "Invalid subnet number.");
            }

            else if (req <= 0)
            {
                strcpy(res, "Invalid number of IP addresses.");
            }

            else if (req > sub[sn - 1].th - sub[sn - 1].alc)
            {
                strcpy(res, "Subnet is full or not enough IP addresses are available.");
            }

            else
            {
                Sub *sp = &sub[sn - 1];

                strcpy(res, "IP addresses allotted successfully:\n");

                int an = 0;

                for (int i = 0; i < sp->th && an < req; i++)
                {
                    if (sp->usd[i] == 0)
                    {
                        uint32_t ip = sp->fh + i;

                        char ips[30];

                        i2ip(ip, ips);

                        sp->usd[i] = 1;
                        sp->alc++;

                        char tmp[50];

                        sprintf(tmp, "%s\n", ips);

                        strcat(res, tmp);

                        an++;
                    }
                }
            }

            printf("%s\n\n", res);

            sendto(fd, res, strlen(res), 0, (struct sockaddr *)&ca, al);
        }

        else if (strcmp(buf, "EXIT") == 0)
        {
            strcpy(res, "DHCP client disconnected.");

            sendto(fd, res, strlen(res), 0, (struct sockaddr *)&ca, al);

            printf("Client disconnected.\n\n");
        }

        else
        {
            strcpy(res, "Invalid request.");

            sendto(fd, res, strlen(res), 0, (struct sockaddr *)&ca, al);
        }
    }

    close(fd);

    return 0;
}
