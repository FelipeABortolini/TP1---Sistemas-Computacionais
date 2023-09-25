#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024
#define PORT 8080
#define MAX_CLIENTS 10

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

double perform_operation(const char *operation, double a, double b) {
    if (strcmp(operation, "add") == 0) {
        return a + b;
    } else if (strcmp(operation, "subtract") == 0) {
        return a - b;
    } else if (strcmp(operation, "multiply") == 0) {
        return a * b;
    } else if (strcmp(operation, "divide") == 0) {
        return a / b;
    }

    return 0;
}

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
    char operation[32];
    double a, b;
    sscanf(buffer, "%s %lf %lf", operation, &a, &b);
    printf("Server received request: %s %.2lf %.2lf\n", operation, a, b);

    double result = perform_operation(operation, a, b);

    // Send the result back to the client
    snprintf(buffer, BUFFER_SIZE, "%.2lf", result);
    if (send(client_sockfd, buffer, strlen(buffer) + 1, 0) < 0) {
        perror("Error sending result");
        close(client_sockfd);
        pthread_exit(NULL);
    }

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
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to address
    if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_sockfd, 5) < 0) {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
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
