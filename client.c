#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PORT 8080

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in server_addr;
    const char client_hello[] = "client";
    char buffer[BUFFER_SIZE];

    if (argc < 2) {
        printf("Usage: %s <ip address>\n", argv[0]);
        exit(0);
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    // Send message identifying itself as a client
    if (send(sockfd, client_hello, strlen(client_hello) + 1, 0) < 0) {
        perror("Error sending client hello");
        exit(EXIT_FAILURE);
    }

    // Send request to server
    char operation[32];
    double a, b;
    printf("Enter operation (add, subtract, multiply, divide): ");
    scanf("%s", operation);
    printf("Enter two numbers: ");
    scanf("%lf %lf", &a, &b);

    snprintf(buffer, BUFFER_SIZE, "%s %.2lf %.2lf", operation, a, b);
    if (send(sockfd, buffer, strlen(buffer) + 1, 0) < 0) {
        perror("Error sending request");
        exit(EXIT_FAILURE);
    }

    // Receive response from server
    memset(buffer, 0, BUFFER_SIZE);
    if (recv(sockfd, buffer, BUFFER_SIZE, 0) < 0) {
        perror("Error receiving response");
        exit(EXIT_FAILURE);
    }

    printf("Result: %s\n", buffer);

    // Close connection
    close(sockfd);

    return 0;
}