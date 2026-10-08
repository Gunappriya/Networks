#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#define PORT 8080
#define BUFFER_SIZE 1024
int server_socket;
void stop_server(int signal_number)
{
    printf("\nServer stopped.\n");
    close(server_socket);
    exit(0);
}
int main()
{
    int client_socket;
    int process_id;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    socklen_t client_length;
    char file_name[256];
    char buffer[BUFFER_SIZE];
    signal(SIGINT, stop_server);
    signal(SIGTSTP, stop_server);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }
    int option = 1;
    setsockopt(server_socket,
               SOL_SOCKET,
               SO_REUSEADDR,
               &option,
               sizeof(option));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    if (bind(server_socket,
             (struct sockaddr *)&server_address,
             sizeof(server_address)) < 0)
    {
        printf("Bind failed\n");
        close(server_socket);
        return 1;
    }
    if (listen(server_socket, 10) < 0)
    {
        printf("Listen failed\n");
        close(server_socket);
        return 1;
    }
    printf("====================================\n");
    printf("       TCP FILE SERVER\n");
    printf("====================================\n");
    printf("Server started...\n");
    printf("Port : %d\n", PORT);
    printf("Waiting for clients...\n\n");
    while (1)
    {
        client_length = sizeof(client_address);
        client_socket = accept(
            server_socket,
            (struct sockaddr *)&client_address,
            &client_length
        );
        if (client_socket < 0)
        {
            printf("Accept failed\n");
            continue;
        }
        printf("Client connected: %s\n",
               inet_ntoa(client_address.sin_addr));
        process_id = fork();
        if (process_id < 0)
        {
            printf("Fork failed\n");
            close(client_socket);
            continue;
        }
        if (process_id == 0)
        {
            FILE *file;
            int bytes_received;
            int bytes_read;
            long total_bytes = 0;
            close(server_socket);
            memset(file_name, 0, sizeof(file_name));
            bytes_received = recv(
                client_socket,
                file_name,
                sizeof(file_name) - 1,
                0
            );
            if (bytes_received <= 0)
            {
                printf("File request failed\n");
                close(client_socket);
                exit(0);
            }
            file_name[bytes_received] = '\0';
            printf("Client requested: %s\n", file_name);
            file = fopen(file_name, "rb");
            if (file == NULL)
            {
                printf("File not available.\n");
                send(client_socket,
                     "FILE_NOT_FOUND",
                     14,
                     0);
                close(client_socket);
                exit(0);
            }
            printf("File found. Sending file...\n");
            send(client_socket,
                 "FILE_FOUND",
                 10,
                 0);
            while ((bytes_read = fread(
                        buffer,
                        1,
                        BUFFER_SIZE,
                        file)) > 0)
            {
                send(client_socket,
                     buffer,
                     bytes_read,
                     0);
                total_bytes += bytes_read;
            }
            fclose(file);
            printf("File sent successfully.\n");
            printf("File name : %s\n", file_name);
            printf("File size : %ld bytes\n", total_bytes);
            shutdown(client_socket, SHUT_WR);
            close(client_socket);
            exit(0);
        }
        else
        {
            close(client_socket);
            printf("Request handled by child process.\n");
        }
    }
    close(server_socket);
    return 0;
}

$./s2
====================================
       TCP FILE SERVER
====================================
Server started...
Port : 8080
Waiting for clients...

Client connected: 172.16.16.200
Request handled by child process.
Client requested: abcd.txt
File not available.
Client connected: 172.16.16.200
Request handled by child process.
Client requested: file.txt
File found. Sending file...
File sent successfully.
File name : file.txt
File size : 0 bytes
