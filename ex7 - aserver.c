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
