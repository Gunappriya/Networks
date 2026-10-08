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
