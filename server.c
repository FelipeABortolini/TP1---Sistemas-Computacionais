#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define PORT 8080
#define MAX_CLIENTS 10

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
    int client_sockfd = *(int *)arg;
    char buffer[BUFFER_SIZE];

    // Receive request from client
    memset(buffer, 0, BUFFER_SIZE);
    if (recv(client_sockfd, buffer, BUFFER_SIZE, 0) < 0) {
        perror("Error receiving request");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Process request and send response
    // Implement your logic here

    // Close connection
    close(client_sockfd);
    pthread_exit(NULL);
}

int main() {
    int server_sockfd, client_sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    pthread_t tid;

    // Create socket
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0) {
        perror("Error creating socket");
        return 1;
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to address
    if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        return 1;
    }

    // Listen for connections
    if (listen(server_sockfd, MAX_CLIENTS) < 0) {
        perror("Error listening for connections");
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept incoming connection
        client_len = sizeof(client_addr);
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (client_sockfd < 0) {
            perror("Error accepting connection");
            continue;
        }

        // Handle client in a separate thread
        pthread_create(&tid, NULL, handle_client, &client_sockfd);
        pthread_detach(tid);
    }

    // Close server socket
    close(server_sockfd);

    return 0;
}

