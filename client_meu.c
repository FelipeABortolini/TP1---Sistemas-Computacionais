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
        printf("Uso: %s <ip address>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd;
    struct sockaddr_in server_addr;

    /* Create socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Erro ao criar o socket.\n");
        exit(EXIT_FAILURE);
    }

    /* Configure server address */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    /* Connect to the server */
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao conectar com o servidor.\n");
        exit(EXIT_FAILURE);
    }

    /* Send message identifying itself as a client. */
    const char client_hello[] = "client";
    if (send(sockfd, client_hello, strlen(client_hello) + 1, 0) < 0) {
        perror("Erro ao enviar client hello.\n");
        exit(EXIT_FAILURE);
    }

    char operation[BUFFER_SIZE];

    while (1) {
        printf("Entre com uma operação (add, subtract, multiply, divide) ou 'quit' para sair: ");
        fgets(operation, BUFFER_SIZE, stdin);

        /* Remove newline character from input */
        size_t len = strlen(operation);
        if (len > 0 && operation[len - 1] == '\n') {
            operation[len - 1] = '\0';
        }

        /* Send the operation to the server */
        if (send(sockfd, operation, strlen(operation) + 1, 0) < 0) {
            perror("Erro ao enviar operação ao servidor.\n");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        /* If the client wants to quit, exit */
        if (strcmp(operation, "quit") == 0) {
            printf("Saindo...\n");
            break;
        }

        /* Receive and print the result from the server */
        memset(operation, 0, BUFFER_SIZE);
        if (recv(sockfd, operation, BUFFER_SIZE, 0) < 0) {
            perror("Erro ao receber resultado do servidor.\n");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        if(operation == "Sistema ocupado. Tente mais tarde."){
            printf("%s\n", operation);
            printf("Saindo...\n");
            break;
        } else {
            printf("Resultado: %s\n", operation);
        }
    }

    /* Close the socket */
    close(sockfd);

    return 0;
}
