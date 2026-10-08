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
