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
-------------------------------------------------------------------------------
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
------------------------------------------------------------------------------
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
--------------------------------------------------------------------
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
-----------------------------------------------------------------------------
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
--------------------------------------------------------------------------------
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
---------------------------------------------------------------------------------
