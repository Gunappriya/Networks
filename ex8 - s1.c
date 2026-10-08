#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 8085
#define BUFFER_SIZE 5
#define WINDOW_SIZE 1

struct Packet
{
    int frame_no;
    int seq_no;
};

int main()
{
    int sockfd;
    struct sockaddr_in server_addr;

    struct Packet buffer[BUFFER_SIZE];
    struct Packet packet;

    int front = 0;
    int rear = -1;
    int count = 0;

    int total_packets;
    int produced = 0;
    int completed = 0;

    int ack;
    int expected_ack;

    struct timeval timeout;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
               &timeout, sizeof(timeout));

    printf("Stop-and-Wait UDP Sender started...\n\n");

    printf("Enter number of packets: ");
    scanf("%d", &total_packets);

    printf("\n");
    printf("Window Size = %d\n", WINDOW_SIZE);
    printf("Circular Buffer Size = %d\n\n", BUFFER_SIZE);

    while (completed < total_packets)
    {
        while (produced < total_packets && count < BUFFER_SIZE)
        {
            rear = (rear + 1) % BUFFER_SIZE;

            buffer[rear].frame_no = produced;


            buffer[rear].seq_no = produced % 2;

            printf("Sender: Packet %d added to circular buffer.\n",
                   produced);

            produced++;
            count++;
        }

        packet = buffer[front];

        printf("\n");
        printf("Sender: Frame %d taken from circular buffer.\n",
               packet.frame_no);

        printf("Sender: Sequence Number = %d\n",
               packet.seq_no);

        expected_ack = 1 - packet.seq_no;

        while (1)
        {
            printf("\n");
            printf("Sender: Sending Frame %d (Seq %d)...\n",
                   packet.frame_no,
                   packet.seq_no);

            sendto(sockfd,
                   &packet,
                   sizeof(packet),
                   0,
                   (struct sockaddr *)&server_addr,
                   sizeof(server_addr));

            printf("Sender: Timer started.\n");
            printf("Sender: Waiting for ACK %d...\n",
                   expected_ack);

            if (recvfrom(sockfd,
                         &ack,
                         sizeof(ack),
                         0,
                         NULL,
                         NULL) < 0)
            {
                printf("\n");
                printf("Sender: No ACK received.\n");
                printf("Sender: TIMEOUT occurred!\n");
                printf("Sender: Timer stopped.\n");

                printf("Sender: Resending Frame %d...\n",
                       packet.frame_no);

                continue;
            }

            printf("\n");
            printf("Sender: ACK %d received.\n", ack);

            if (ack == expected_ack)
            {
                printf("Sender: Correct ACK received.\n");
                printf("Sender: Frame %d successfully transmitted.\n",
                       packet.frame_no);

                printf("Sender: Timer stopped.\n");

                break;
            }
            else
            {
                printf("Sender: Wrong ACK received.\n");
                printf("Sender: Expected ACK %d.\n",
                       expected_ack);

                printf("Sender: Resending Frame %d...\n",
                       packet.frame_no);
            }
        }
        front = (front + 1) % BUFFER_SIZE;
        count--;

        completed++;

        printf("\n");
        printf("Sender: Frame %d removed from circular buffer.\n",
               packet.frame_no);

        printf("Sender: Front = %d, Rear = %d\n",
               front, rear);

        printf("----------------------------------\n");

        sleep(1);
    }

    printf("\n");
    printf("All %d packets transmitted successfully!\n",
           total_packets);

    close(sockfd);

    return 0;
}
$gcc s1.c -o s1
./s1
Stop-and-Wait UDP Sender started...

Enter number of packets: 3

Window Size = 1
Circular Buffer Size = 5

Sender: Packet 0 added to circular buffer.
Sender: Packet 1 added to circular buffer.
Sender: Packet 2 added to circular buffer.

Sender: Frame 0 taken from circular buffer.
Sender: Sequence Number = 0

Sender: Sending Frame 0 (Seq 0)...
Sender: Timer started.
Sender: Waiting for ACK 1...

Sender: ACK 1 received.
Sender: Correct ACK received.
Sender: Frame 0 successfully transmitted.
Sender: Timer stopped.

Sender: Frame 0 removed from circular buffer.
Sender: Front = 1, Rear = 2
----------------------------------

Sender: Frame 1 taken from circular buffer.
Sender: Sequence Number = 1

Sender: Sending Frame 1 (Seq 1)...
Sender: Timer started.
Sender: Waiting for ACK 0...

Sender: ACK 0 received.
Sender: Correct ACK received.
Sender: Frame 1 successfully transmitted.
Sender: Timer stopped.

Sender: Frame 1 removed from circular buffer.
Sender: Front = 2, Rear = 2
----------------------------------

Sender: Frame 2 taken from circular buffer.
Sender: Sequence Number = 0

Sender: Sending Frame 2 (Seq 0)...
Sender: Timer started.
Sender: Waiting for ACK 1...

Sender: ACK 1 received.
Sender: Correct ACK received.
Sender: Frame 2 successfully transmitted.
Sender: Timer stopped.

Sender: Frame 2 removed from circular buffer.
Sender: Front = 3, Rear = 2
----------------------------------

All 3 packets transmitted successfully!
