#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8085

struct Packet
{
    int frame_no;
    int seq_no;
};

int main()
{
    int sockfd;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t addr_len;

    struct Packet packet;

    int expected_seq = 0;
    int ack;

    srand(time(NULL));

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        printf("Bind failed\n");
        close(sockfd);
        return 1;
    }

    printf("Stop-and-Wait UDP Receiver started...\n");
    printf("Waiting for frames...\n\n");

    while (1)
    {
        addr_len = sizeof(client_addr);

        recvfrom(sockfd,
                 &packet,
                 sizeof(packet),
                 0,
                 (struct sockaddr *)&client_addr,
                 &addr_len);

        printf("Receiver: Frame %d received.\n",
               packet.frame_no);

        printf("Receiver: Sequence Number = %d\n",
               packet.seq_no);

        if (rand() % 5 == 0)
        {
            printf("Receiver: ERROR occurred!\n");

            printf("Receiver: Frame %d is lost.\n",
                   packet.frame_no);

            printf("Receiver: No ACK sent.\n");

            printf("----------------------------------\n\n");

            continue;
        }

        printf("Receiver: No error in Frame %d.\n",
               packet.frame_no);

        if (packet.seq_no == expected_seq)
        {
            printf("Receiver: Frame %d accepted.\n",
                   packet.frame_no);
            ack = 1 - expected_seq;

            printf("Receiver: Sending ACK %d.\n",
                   ack);

            sendto(sockfd,
                   &ack,
                   sizeof(ack),
                   0,
                   (struct sockaddr *)&client_addr,
                   addr_len);
            expected_seq = ack;
        }
        else
        {
            printf("Receiver: Duplicate Frame %d received.\n", packet.frame_no);

            ack = expected_seq;

            printf("Receiver: Sending ACK %d again.\n",
                   ack);

            sendto(sockfd,
                   &ack,
                   sizeof(ack),
                   0,
                   (struct sockaddr *)&client_addr,
                   addr_len);
        }

        printf("----------------------------------\n\n");
    }

    close(sockfd);

    return 0;
}
$gcc c1.c -o c1
$./c1
Stop-and-Wait UDP Receiver started...
Waiting for frames...

Receiver: Frame 0 received.
Receiver: Sequence Number = 0
Receiver: No error in Frame 0.
Receiver: Frame 0 accepted.
Receiver: Sending ACK 1.
----------------------------------

Receiver: Frame 1 received.
Receiver: Sequence Number = 1
Receiver: No error in Frame 1.
Receiver: Frame 1 accepted.
Receiver: Sending ACK 0.
----------------------------------

Receiver: Frame 2 received.
Receiver: Sequence Number = 0
Receiver: No error in Frame 2.
Receiver: Frame 2 accepted.
Receiver: Sending ACK 1.
----------------------------------
