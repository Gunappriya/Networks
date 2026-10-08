cat s1.c
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

  $cat s2.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 8087
#define BUFFER_SIZE 10
#define WINDOW_SIZE 4
#define TIMEOUT_SEC 3

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

    int ack[BUFFER_SIZE];
    int sent[BUFFER_SIZE];

    int total_packets;
    int base = 0;
    int next_frame;
    int received_ack;

    int i;
    int completed = 0;

    struct timeval start_time[BUFFER_SIZE];
    struct timeval current_time;

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
    struct timeval receive_timeout;

    receive_timeout.tv_sec = 0;
    receive_timeout.tv_usec = 100000;

    setsockopt(sockfd,
               SOL_SOCKET,
               SO_RCVTIMEO,
               &receive_timeout,
               sizeof(receive_timeout));

    printf("Selective Repeat UDP Sender started...\n\n");

    printf("Enter number of packets: ");
    scanf("%d", &total_packets);

    if (total_packets <= 0 || total_packets > BUFFER_SIZE)
    {
        printf("Enter packets between 1 and %d.\n",
               BUFFER_SIZE);

        close(sockfd);
        return 1;
    }

    printf("\n");
    printf("Window Size = %d\n", WINDOW_SIZE);
    printf("Circular Buffer Size = %d\n",
           BUFFER_SIZE);

    printf("Timer = %d seconds\n\n",
           TIMEOUT_SEC);


    for (i = 0; i < total_packets; i++)
    {
        buffer[i].frame_no = i;
        buffer[i].seq_no = i;

        ack[i] = 0;
        sent[i] = 0;
    }
    while (completed < total_packets)
    {
    for (next_frame = base;
             next_frame < base + WINDOW_SIZE &&
             next_frame < total_packets;
             next_frame++)
        {
            if (sent[next_frame] == 0)
            {
                printf("Sender: Frame %d added to window.\n",
                       next_frame);

                printf("Sender: Sequence Number = %d\n",
                       buffer[next_frame].seq_no);

                printf("Sender: Sending Frame %d...\n",
                       next_frame);

                sendto(sockfd,
                       &buffer[next_frame],
                       sizeof(buffer[next_frame]),
                       0,
                       (struct sockaddr *)&server_addr,
                       sizeof(server_addr));

                gettimeofday(&start_time[next_frame],
                             NULL);

                sent[next_frame] = 1;

                printf("Sender: Timer started for Frame %d.\n",
                       next_frame);

                printf("----------------------------------\n");
            }
        }

        while (1)
        {
        if (recvfrom(sockfd,
                         &received_ack,
                         sizeof(received_ack),
                         0,
                         NULL,
                         NULL) > 0)
            {
                if (received_ack >= 0 &&
                    received_ack < total_packets)
                {
                    if (ack[received_ack] == 0)
                    {
                        ack[received_ack] = 1;
                        completed++;

                        printf("\n");
                        printf("Sender: ACK %d received.\n",
                               received_ack);

                        printf("Sender: Frame %d acknowledged.\n",
                               received_ack);

                        printf("Sender: Timer stopped for Frame %d.\n",
                               received_ack);

                        printf("----------------------------------\n");
                    }
                    else
                    {
                        printf("\n");
                        printf("Sender: Duplicate ACK %d received.\n",
                               received_ack);
                    }
                }
            }

            gettimeofday(&current_time, NULL);

            for (i = base;
                 i < base + WINDOW_SIZE &&
                 i < total_packets;
                 i++)
            {
                if (sent[i] == 1 && ack[i] == 0)
                {
                    long elapsed;

                    elapsed =
                        current_time.tv_sec -
                        start_time[i].tv_sec;


                    if (elapsed >= TIMEOUT_SEC)
                    {
                        printf("\n");
                        printf("Sender: TIMEOUT for Frame %d!\n",
                               i);

                        printf("Sender: Timer stopped for Frame %d.\n",
                               i);

                        printf("Sender: Retransmitting ONLY Frame %d...\n",
                               i);

                        sendto(sockfd,
                               &buffer[i],
                               sizeof(buffer[i]),
                               0,
                               (struct sockaddr *)&server_addr,
                               sizeof(server_addr));

                        gettimeofday(&start_time[i],
                                     NULL);

                        printf("Sender: Timer restarted for Frame %d.\n",
                               i);

                        printf("----------------------------------\n");
                    }
                }
            }
            while (base < total_packets &&
                   ack[base] == 1)
            {
                printf("\n");
                printf("Sender: Frame %d removed from window.\n",
                       base);

                base++;
            }
            if (completed == total_packets)
            {
                break;
            }
            for (next_frame = base;
                 next_frame < base + WINDOW_SIZE &&
                 next_frame < total_packets;
                 next_frame++)
            {
                if (sent[next_frame] == 0)
                {
                    printf("\n");
                    printf("Sender: New Frame %d entered window.\n",
                           next_frame);

                    printf("Sender: Sending Frame %d...\n",
                           next_frame);

                    sendto(sockfd,
                           &buffer[next_frame],
                           sizeof(buffer[next_frame]),
                           0,
                           (struct sockaddr *)&server_addr,
                           sizeof(server_addr));

                    gettimeofday(&start_time[next_frame],
                                 NULL);

                    sent[next_frame] = 1;

                    printf("Sender: Timer started for Frame %d.\n",
                           next_frame);

                    printf("----------------------------------\n");
                }
            }

            usleep(100000);
        }
    }

    printf("\n");
    printf("All %d packets transmitted successfully!\n",
           total_packets);

    close(sockfd);

    return 0;
}
$gcc s2.c -o s2
$./s2
Selective Repeat UDP Sender started...

Enter number of packets: 3

Window Size = 4
Circular Buffer Size = 10
Timer = 3 seconds

Sender: Frame 0 added to window.
Sender: Sequence Number = 0
Sender: Sending Frame 0...
Sender: Timer started for Frame 0.
----------------------------------
Sender: Frame 1 added to window.
Sender: Sequence Number = 1
Sender: Sending Frame 1...
Sender: Timer started for Frame 1.
----------------------------------
Sender: Frame 2 added to window.
Sender: Sequence Number = 2
Sender: Sending Frame 2...
Sender: Timer started for Frame 2.
----------------------------------

Sender: ACK 0 received.
Sender: Frame 0 acknowledged.
Sender: Timer stopped for Frame 0.
----------------------------------

Sender: Frame 0 removed from window.

Sender: ACK 1 received.
Sender: Frame 1 acknowledged.
Sender: Timer stopped for Frame 1.
----------------------------------

Sender: Frame 1 removed from window.

Sender: TIMEOUT for Frame 2!
Sender: Timer stopped for Frame 2.
Sender: Retransmitting ONLY Frame 2...
Sender: Timer restarted for Frame 2.
----------------------------------

Sender: ACK 2 received.
Sender: Frame 2 acknowledged.
Sender: Timer stopped for Frame 2.
----------------------------------

Sender: Frame 2 removed from window.

All 3 packets transmitted successfully!

  $cat c1.c
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


$cat c2.c

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

