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
