  #include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5001
#define DATA_ROWS 3
#define DATA_COLS 4
#define TOTAL_COLS (DATA_COLS + 1)   // + parity column
#define TOTAL_ROWS (DATA_ROWS + 1)   // + parity row

int sockfd;
struct sockaddr_in server;

char data[DATA_ROWS][DATA_COLS + 1];          // original input
char matrix[TOTAL_ROWS][TOTAL_COLS + 1];      // full 2D codeword (+null)

void createClient()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { printf("Socket creation failed\n"); return; }
    printf("Client socket created\n");
}

void connectServer()
{
    server.sin_family = AF_INET;
    /* CHANGED: replace with the SERVER laptop's actual LAN IPv4 address
       (find it using ipconfig on Windows / ip addr on Linux).
       Example below assumes Server Laptop IP = 192.168.1.10 */
    inet_pton(AF_INET, "192.168.1.10", &server.sin_addr);
    server.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Connection failed\n"); return;
    }
    printf("Connected to server\n");
}

void getData()
{
    printf("Enter %d binary rows (each %d bits, e.g. 1011):\n", DATA_ROWS, DATA_COLS);
    for (int i = 0; i < DATA_ROWS; i++) {
        printf("Row %d: ", i + 1);
        scanf("%s", data[i]);
    }
}

/* Build 2D parity: row parity + column parity + bottom-right corner */
void make2DParity()
{
    // 1) Copy data and append even-parity bit to each row
    for (int i = 0; i < DATA_ROWS; i++) {
        int count = 0;
        for (int j = 0; j < DATA_COLS; j++) {
            matrix[i][j] = data[i][j];
            if (data[i][j] == '1') count++;
        }
        matrix[i][DATA_COLS] = (count % 2 == 0) ? '0' : '1';  // even parity
        matrix[i][TOTAL_COLS] = '\0';
    }

    // 2) Compute bottom parity row (even parity over each column, including parity col)
    for (int j = 0; j <= DATA_COLS; j++) {
        int count = 0;
        for (int i = 0; i < DATA_ROWS; i++) {
            if (matrix[i][j] == '1') count++;
        }
        matrix[DATA_ROWS][j] = (count % 2 == 0) ? '0' : '1';
    }
    matrix[DATA_ROWS][TOTAL_COLS] = '\0';

    printf("\n--- Generated 2D Parity Codeword ---\n");
    for (int i = 0; i < TOTAL_ROWS; i++)
        printf("%s\n", matrix[i]);
}

void sendData()
{
    char buffer[512];
    buffer[0] = '\0';
    for (int i = 0; i < TOTAL_ROWS; i++) {
        strcat(buffer, matrix[i]);
        strcat(buffer, "\n");
    }
    send(sockfd, buffer, strlen(buffer), 0);
    printf("\n2D Codeword sent to server.\n");
}

void closeClient()
{
    close(sockfd);
    printf("Client connection closed\n");
}

int main()
{
    createClient();
    connectServer();
    getData();
    make2DParity();
    sendData();
    closeClient();
    return 0;
}
