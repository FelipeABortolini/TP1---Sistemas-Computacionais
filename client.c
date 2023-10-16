#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/select.h>

#define BUFFER_SIZE 1024
#define SERVER_PORT 8080

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <ip address>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd;
    struct sockaddr_in server_addr;

    /* Cria socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Erro ao criar socket.\n");
        exit(EXIT_FAILURE);
    }

    /* Configura endereço do servidor */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    /* Se conecta ao servidor */
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao conectar com servidor.\n");
        exit(EXIT_FAILURE);
    }

    /* Envia mensagem se identificando como cliente */
    const char client_hello[] = "client";
    if (send(sockfd, client_hello, strlen(client_hello) + 1, 0) < 0) {
        perror("Erro ao enviar client hello.\n");
        exit(EXIT_FAILURE);
    }

    char *operation;
    operation = calloc(BUFFER_SIZE, sizeof(char));
    
    if (operation == NULL) {
       perror("Erro ao alocar memória.\n");
        exit(EXIT_FAILURE);
        }

    /* Se o cliente deseja sair */
    if (strcmp(argv[2], "quit") == 0) {
        printf("Saindo...\n");
    } else {
        strncat(operation, argv[2], strlen(argv[2])+1);
        strncat(operation, " ", 2);
        strncat(operation, argv[3], strlen(argv[3])+1);
        strncat(operation, " ", 2);
        strncat(operation, argv[4], strlen(argv[4])+1);
        printf("Operação: %s\n", operation);
        if (send(sockfd, operation, strlen(operation) + 1, 0) < 0) {
            perror("Erro ao enviar operação ao servidor.\n");
            exit(EXIT_FAILURE);
        }
    }

    /* Recebe e mostra o resultado do servidor */
    memset(operation, 0, BUFFER_SIZE);
    if (recv(sockfd, operation, BUFFER_SIZE, 0) < 0) {
        perror("Erro ao receber resultado do servidor.\n");
        exit(EXIT_FAILURE);
    }

    if(operation == "Sistema ocupado. Tente mais tarde." || operation == "quit"){
        printf("%s\n", operation);
        printf("Saindo...\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Resultado: %s\n", operation);
    }

    /* Fechar o socket após receber a resposta do servidor */
    if (close(sockfd) < 0) {
        perror("Erro ao fechar o socket.\n");
        exit(EXIT_FAILURE);
    }

    free(operation);  // Liberar a memória alocada

    return 0;
}
