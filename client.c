#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024
#define SERVER_PORT 8080

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <ip address>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd;
    struct sockaddr_in server_addr;

    /* Create socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    /* Configure server address */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    /* Connect to the server */
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    /* Send message identifying itself as a client. */
    const char client_hello[] = "client";
    if (send(sockfd, client_hello, strlen(client_hello) + 1, 0) < 0) {
        perror("Error sending client hello");
        exit(EXIT_FAILURE);
    }

    char operation[BUFFER_SIZE];

    while (1) {
        printf("Enter operation (add, subtract, multiply, divide) or 'quit' to exit: ");
        fgets(operation, BUFFER_SIZE, stdin);

        /* Remove newline character from input */
        size_t len = strlen(operation);
        if (len > 0 && operation[len - 1] == '\n') {
            operation[len - 1] = '\0';
        }

        /* Send the operation to the server */
        if (send(sockfd, operation, strlen(operation) + 1, 0) < 0) {
            perror("Error sending operation to server");
            exit(EXIT_FAILURE);
        }

        /* If the client wants to quit, exit */
        if (strcmp(operation, "quit") == 0) {
            printf("Quitting...\n");
            break;
        }

        /* Receive and print the result from the server */
        memset(operation, 0, BUFFER_SIZE);
        if (recv(sockfd, operation, BUFFER_SIZE, 0) < 0) {
            perror("Error receiving result from server");
            exit(EXIT_FAILURE);
        }

        printf("Result: %s\n", operation);
    }

    /* Close the socket */
    close(sockfd);

    return 0;
}
