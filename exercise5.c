#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    int server_fd, client_fd;
    char buffer[1024];
    struct sockaddr_in server_addr, client_addr;
    socklen_t len;

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", *argv);
        return 1;
    }

    char *port_str = *(argv + 1);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(port_str));

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        return 1;
    }

    listen(server_fd, 5);
    printf("TCP Echo Server running on port %s...\n", port_str);

    while (1)
    {
        len = sizeof(client_addr);

        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }

        int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (n > 0)
        {
            buffer[n] = '\0';
            printf("Client: %s\n", buffer);
            send(client_fd, buffer, n, 0);
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}
----------------------------------------------------------------------------

  #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    int sockfd, n;
    char buffer[1024];
    struct sockaddr_in server_addr;

    if (argc != 4)
    {
        printf("Usage: %s <server_ip> <port> <message>\n", *argv);
        return 1;
    }

    char *server_ip = *(argv + 1);
    char *port_str  = *(argv + 2);
    char *message   = *(argv + 3);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port_str));

    int status = inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
    if (status == 0)
    {
        fprintf(stderr, "Error: Invalid IPv4 address '%s'\n", server_ip);
        close(sockfd);
        return 1;
    }
    else if (status < 0)
    {
        perror("inet_pton");
        close(sockfd);
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        return 1;
    }

    send(sockfd, message, strlen(message), 0);

    n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (n > 0)
    {
        buffer[n] = '\0';
        printf("Echo from server: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}
------------------------------------------------------------------------------------
  #include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

int sockfd, clientfd;
struct sockaddr_in server, client;

char str1[100];
char str2[100];
char result[256];

void createServer()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        return;
    }
    printf("Server socket created\n");
}

void bindServer()
{
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Bind failed\n");
        return;
    }
    printf("Server bind successful\n");
}

void waitClient()
{
    listen(sockfd, 5);
    printf("Waiting for client...\n");

    socklen_t len = sizeof(client);
    clientfd = accept(sockfd, (struct sockaddr *)&client, &len);

    if (clientfd < 0)
    {
        printf("Client connection failed\n");
        return;
    }
    printf("Client connected\n");
}

void receiveData()
{
    memset(str1, 0, sizeof(str1));
    memset(str2, 0, sizeof(str2));

    recv(clientfd, str1, sizeof(str1), 0);
    recv(clientfd, str2, sizeof(str2), 0);

    printf("\nReceived String 1: \"%s\"\n", str1);
    printf("Received String 2: \"%s\"\n", str2);
}

void compareStrings()
{
    int i = 0;
    int is_equal = 1;

    while (str1[i] != '\0' || str2[i] != '\0')
    {
        if (str1[i] != str2[i])
        {
            is_equal = 0;
            sprintf(result, "Strings are NOT equal (Mismatch at index %d: '%c' vs '%c')",
                    i,
                    str1[i] == '\0' ? ' ' : str1[i],
                    str2[i] == '\0' ? ' ' : str2[i]);
            break;
        }
        i++;
    }

    if (is_equal)
    {
        sprintf(result, "Strings are EQUAL (All %d characters matched successfully)", i);
    }

    printf("Comparison Result: %s\n", result);
}

void sendResult()
{
    send(clientfd, result, strlen(result) + 1, 0);
    printf("Result sent to client\n");
}

void closeServer()
{
    close(clientfd);
    close(sockfd);
    printf("Connection closed\n");
}

int main()
{
    createServer();
    bindServer();
    waitClient();
    receiveData();
    compareStrings();
    sendResult();
    closeServer();

    return 0;
}
------------------------------------------------------------------------------
  #include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

int sockfd;
struct sockaddr_in server;

char str1[100];
char str2[100];
char result[256];

void createClient()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        return;
    }
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

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Connection failed\n");
        return;
    }
    printf("Connected to server\n");
}

void getData()
{
    printf("Enter 1st string: ");
    fgets(str1, sizeof(str1), stdin);
    str1[strcspn(str1, "\n")] = '\0';

    printf("Enter 2nd string: ");
    fgets(str2, sizeof(str2), stdin);
    str2[strcspn(str2, "\n")] = '\0';
}

void sendData()
{
    send(sockfd, str1, sizeof(str1), 0);
    send(sockfd, str2, sizeof(str2), 0);
    printf("Strings sent to server for comparison\n");
}

void receiveResult()
{
    memset(result, 0, sizeof(result));
    recv(sockfd, result, sizeof(result), 0);
    printf("\nServer Response:\n%s\n", result);
}

void closeClient()
{
    close(sockfd);
    printf("Connection closed\n");
}

int main()
{
    createClient();
    connectServer();
    getData();
    sendData();
    receiveResult();
    closeClient();

    return 0;
}
-------------------------------------------------------------------------
  #include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5001
#define DATA_ROWS 3
#define DATA_COLS 4
#define TOTAL_COLS (DATA_COLS + 1)
#define TOTAL_ROWS (DATA_ROWS + 1)

int sockfd, clientfd;
struct sockaddr_in server, client;

void createServer()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { printf("Socket creation failed\n"); return; }
    printf("Server socket created\n");
}

void bindServer()
{
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Bind failed\n"); return;
    }
    printf("Server bind successful\n");
}

void waitClient()
{
    listen(sockfd, 5);
    printf("Waiting for client...\n");

    socklen_t len = sizeof(client);
    clientfd = accept(sockfd, (struct sockaddr *)&client, &len);

    if (clientfd < 0) {
        printf("Client connection failed\n"); return;
    }
    printf("Client connected\n");
}

void check2DParity()
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    recv(clientfd, buffer, sizeof(buffer) - 1, 0);

    printf("\n=== Received 2D Parity Codeword ===\n%s", buffer);

    // Parse newline-separated rows into 2D array
    char parsed[TOTAL_ROWS][TOTAL_COLS + 1];
    memset(parsed, 0, sizeof(parsed));

    int row = 0;
    char *line = strtok(buffer, "\n");
    while (line != NULL && row < TOTAL_ROWS) {
        strncpy(parsed[row], line, TOTAL_COLS);
        parsed[row][TOTAL_COLS] = '\0';
        row++;
        line = strtok(NULL, "\n");
    }

    printf("\nParsed Matrix:\n");
    for (int i = 0; i < row; i++) printf("%s\n", parsed[i]);

    int row_bad[TOTAL_ROWS] = {0};
    int col_bad[TOTAL_COLS + 1] = {0};  // 0..DATA_COLS
    int bad_rows = 0, bad_cols = 0;

    // Check each original data row (even parity over data + parity bit)
    for (int i = 0; i < DATA_ROWS; i++) {
        int cnt = 0;
        for (int j = 0; j <= DATA_COLS; j++) {
            if (parsed[i][j] == '1') cnt++;
        }
        if (cnt % 2 != 0) {
            row_bad[i] = 1;
            bad_rows++;
            printf("Row %d parity error (count=%d)\n", i, cnt);
        }
    }

    // Check each column including parity row (even parity over column)
    for (int j = 0; j <= DATA_COLS; j++) {
        int cnt = 0;
        for (int i = 0; i < DATA_ROWS; i++) {
            if (parsed[i][j] == '1') cnt++;
        }
        cnt += (parsed[DATA_ROWS][j] == '1') ? 1 : 0;

        if (cnt % 2 != 0) {
            col_bad[j] = 1;
            bad_cols++;
            printf("Col %d parity error (count=%d)\n", j, cnt);
        }
    }

    // --- Correction / Reporting ---
    if (bad_rows == 0 && bad_cols == 0) {
        printf("\nResult: No Error Detected (2D Parity Satisfied)\n");
        printf("Original Data (without parity):\n");
        for (int i = 0; i < DATA_ROWS; i++) {
            parsed[i][DATA_COLS] = '\0';  // truncate at parity bit
            printf("%s\n", parsed[i]);
        }
    } else {
        int r = -1, c = -1;
        for (int i = 0; i < DATA_ROWS; i++) if (row_bad[i]) { r = i; break; }
        for (int j = 0; j <= DATA_COLS; j++) if (col_bad[j]) { c = j; break; }

        printf("\nError Analysis: %d bad row(s), %d bad col(s)\n", bad_rows, bad_cols);

        if (bad_rows == 1 && bad_cols == 1) {
            if (r < DATA_ROWS && c < DATA_COLS) {
                // Bit at data intersection is wrong
                printf("Single Bit Error at Data (%d,%d) — flipping '%c' -> '%c'\n",
                       r, c, parsed[r][c], parsed[r][c] == '1' ? '0' : '1');
                parsed[r][c] = (parsed[r][c] == '1') ? '0' : '1';
            } else if (r < DATA_ROWS && c == DATA_COLS) {
                // Row parity bit itself is wrong
                printf("Row Parity Bit Error at Row %d, Parity Col\n", r);
                parsed[r][DATA_COLS] = (parsed[r][DATA_COLS] == '1') ? '0' : '1';
            } else {
                printf("Corrected at (%d,%d)\n", r, c);
                parsed[r][c] = (parsed[r][c] == '1') ? '0' : '1';
            }
        } else if (bad_rows == 1 && bad_cols == 0) {
            printf("Row Parity Bit Error in Row %d\n", r);
            parsed[r][DATA_COLS] = (parsed[r][DATA_COLS] == '1') ? '0' : '1';
        } else if (bad_rows == 0 && bad_cols == 1) {
            printf("Column Parity Bit Error in Col %d\n", c);
            parsed[DATA_ROWS][c] = (parsed[DATA_ROWS][c] == '1') ? '0' : '1';
        } else {
            printf("Multiple errors detected. Uncorrectable with 2D parity alone.\n");
        }

        printf("\nCorrected 2D Codeword:\n");
        for (int i = 0; i <= DATA_ROWS; i++)
            printf("%s\n", parsed[i]);

        printf("\nCorrected Original Data:\n");
        for (int i = 0; i < DATA_ROWS; i++) {
            char tmp[TOTAL_COLS + 1];
            strcpy(tmp, parsed[i]);
            tmp[DATA_COLS] = '\0';  // hide parity bit
            printf("%s\n", tmp);
        }
    }
}

void closeServer()
{
    close(clientfd);
    close(sockfd);
    printf("Server connection closed\n");
}

int main()
{
    createServer();
    bindServer();
    waitClient();
    check2DParity();
    closeServer();
    return 0;
}
------------------------------------------------------------------------
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
------------------------------------------------------------------------
