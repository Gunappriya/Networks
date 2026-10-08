#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8087
#define BUFFER_SIZE 10
#define WINDOW_SIZE 4

struct Packet
{
    int frame_no;
    int seq_no;
};

int frame2_ack_dropped = 0;

int main()
{
    int sockfd;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t addr_len;

    struct Packet packet;

    int received[BUFFER_SIZE];

    int ack;
    int i;

    for (i = 0; i < BUFFER_SIZE; i++)
    {
        received[i] = 0;
    }

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

    printf("Selective Repeat UDP Receiver started...\n");
    printf("Window Size = %d\n",
           WINDOW_SIZE);

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

        if (received[packet.seq_no] == 0)
        {
            received[packet.seq_no] = 1;

            printf("Receiver: Frame %d accepted.\n",
                   packet.frame_no);

            ack = packet.seq_no;
            if (packet.frame_no == 2 &&
                frame2_ack_dropped == 0)
            {
                printf("Receiver: ACK %d lost.\n",
                       ack);

                printf("Receiver: No ACK sent.\n");

                frame2_ack_dropped = 1;

                printf("----------------------------------\n\n");

                continue;
            }

            printf("Receiver: Sending ACK %d.\n",
                   ack);

            sendto(sockfd,
                   &ack,
                   sizeof(ack),
                   0,
                   (struct sockaddr *)&client_addr,
                   addr_len);
        }
        else
        {
        printf("Receiver: Duplicate Frame %d received.\n",
                   packet.frame_no);

            ack = packet.seq_no;

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

$gcc c2.c -o c2
$./c2
Selective Repeat UDP Receiver started...
Window Size = 4
Waiting for frames...

Receiver: Frame 0 received.
Receiver: Sequence Number = 0
Receiver: Frame 0 accepted.
Receiver: Sending ACK 0.
----------------------------------

Receiver: Frame 1 received.
Receiver: Sequence Number = 1
Receiver: Frame 1 accepted.
Receiver: Sending ACK 1.
----------------------------------

Receiver: Frame 2 received.
Receiver: Sequence Number = 2
Receiver: Frame 2 accepted.
Receiver: ACK 2 lost.
Receiver: No ACK sent.
----------------------------------

Receiver: Frame 2 received.
Receiver: Sequence Number = 2
Receiver: Duplicate Frame 2 received.
Receiver: Sending ACK 2 again.
----------------------------------
