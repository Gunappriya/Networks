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
