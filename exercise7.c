
cat cserver.c
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
void stop_server(int sig)
{
    printf("\nServer stopped.\n");
    close(server_socket);
    exit(0);
}
int main()
{
    int client_socket;
    int process_id;
    int client_number = 0;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    socklen_t client_length;
    char buffer[BUFFER_SIZE];
    signal(SIGINT, stop_server);
    signal(SIGTSTP, stop_server);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        printf("Socket creation failed.\n");
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
        printf("Bind failed.\n");
        close(server_socket);
        return 1;
    }
    if (listen(server_socket, 10) < 0)
    {
        printf("Listen failed.\n");
        close(server_socket);
        return 1;
    }
    printf("\n====================================\n");
    printf("       CONCURRENT TCP SERVER\n");
    printf("====================================\n");
    printf("Server running on port %d\n", PORT);
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
            printf("Accept failed.\n");
            continue;
        }
        client_number++;
        process_id = fork();
        if (process_id < 0)
        {
            printf("Fork failed.\n");
            close(client_socket);
            continue;
        }
        if (process_id == 0)
        {
            close(server_socket);
            printf("Client %d connected.\n",
                   client_number);

            while (1)
            {
                memset(buffer, 0, BUFFER_SIZE);
                int received = recv(
                    client_socket,
                    buffer,
                    BUFFER_SIZE - 1,
                    0
                );
                if (received <= 0)
                {
                    printf("Client %d disconnected.\n",
                           client_number);
                    break;
                }
                buffer[received] = '\0';
                printf("Client %d: %s",
                       client_number,
                       buffer);
                if (strcmp(buffer, "exit\n") == 0 ||
                    strcmp(buffer, "exit") == 0)
                {
                    printf("Client %d ended the chat.\n",
                           client_number);
                    break;
                }
                printf("Server to Client %d: ",
                       client_number);
                if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
                {
                    break;
                }
                send(
                    client_socket,
                    buffer,
                    strlen(buffer),
                    0
                );
                if (strcmp(buffer, "exit\n") == 0 ||
                    strcmp(buffer, "exit") == 0)
                {
                    printf("Chat with Client %d ended.\n",
                           client_number);
                    break;
                }
            }
            close(client_socket);
            exit(0);
        }
        else
        {
            close(client_socket);
        }
    }
    close(server_socket);
    return 0;
}

$./s1

====================================
       CONCURRENT TCP SERVER
====================================
Server running on port 8080
Waiting for clients...

Client 1 connected.
Client 1: hi
Server to Client 1: hello
Client 1: how are you
Server to Client 1: fine
Client 1: exit
Client 1 ended the chat.
^C
Server stopped.

cat fserver.c
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
^C
Server stopped.

  $cat aserver.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#define PORT 8090
#define MAX_CLIENTS 10
int server_socket;
void handle_client(int client_socket)
{
    char search_ip[50];
    char reply[200];
    char command[100];
    char line[300];
    FILE *fp;
    int found = 0;
    memset(search_ip, 0, sizeof(search_ip));
    if (recv(client_socket,
             search_ip,
             sizeof(search_ip) - 1,
             0) <= 0)
    {
        close(client_socket);
        exit(0);
    }

    search_ip[strcspn(search_ip, "\n")] = '\0';

    printf("Client requested IP: %s\n", search_ip);
    strcpy(command, "ip neigh");

    fp = popen(command, "r");
    if (fp == NULL)
    {
        strcpy(reply, "Unable to get ARP table");
        send(client_socket,
             reply,
             strlen(reply) + 1,
             0);
        close(client_socket);
        exit(0);
    }
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        char ip[50];
        char mac[50];
        if (strstr(line, search_ip) != NULL &&
            strstr(line, "lladdr") != NULL)
        {
            if (sscanf(line,
                       "%49s dev %*s lladdr %49s",
                       ip,
                       mac) == 2)
            {
                if (strcmp(ip, search_ip) == 0)
                {
                    sprintf(reply,
                            "MAC Address: %s",
                            mac);

                    found = 1;
                    break;
                }
            }
        }
    }

    pclose(fp);
    if (found)
    {
        printf("MAC found: %s\n", reply);
    }
    else
    {
        strcpy(reply, "MAC Address not available");
        printf("MAC not found.\n");
    }
    send(client_socket,
         reply,
         strlen(reply) + 1,
         0);
    close(client_socket);
    exit(0);
}
void stop_server(int signal_number)
{
    printf("\nServer stopped.\n");
    close(server_socket);
    exit(0);
}
int main()
{
    int client_socket;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t addrlen;
    signal(SIGCHLD, SIG_IGN);
    signal(SIGINT, stop_server);
    signal(SIGTSTP, stop_server);
    server_socket = socket(AF_INET,
                           SOCK_STREAM,
                           0);
    if (server_socket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }
    int option = 1;
    setsockopt(server_socket,
               SOL_SOCKET,
               SO_REUSEADDR,
               &option,
               sizeof(option));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_socket,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");

        close(server_socket);
        return 1;
    }
    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("Listen failed");
        close(server_socket);
        return 1;
    }
    printf("====================================\n");
    printf("   CONCURRENT TCP ARP SERVER\n");
    printf("====================================\n");
    printf("Server started...\n");
    printf("Port : %d\n", PORT);
    printf("Using system ARP table: ip neigh\n");
    printf("Waiting for clients...\n\n");
    while (1)
    {
        addrlen = sizeof(client_addr);
        client_socket = accept(
            server_socket,
            (struct sockaddr *)&client_addr,
            &addrlen
        );
        if (client_socket < 0)
        {
            perror("Accept failed");
            continue;
        }
        printf("New client connected: %s\n",
               inet_ntoa(client_addr.sin_addr));
        if (fork() == 0)
        {
            close(server_socket);
            handle_client(client_socket);
        }
        close(client_socket);
    }
    close(server_socket);
    return 0;
}


./s3
====================================
   CONCURRENT TCP ARP SERVER
====================================
Server started...
Port : 8090
Using system ARP table: ip neigh
Waiting for clients...

^C
Server stopped.
