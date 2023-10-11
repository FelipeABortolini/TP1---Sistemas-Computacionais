#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024
#define PORT 8080
#define MAX_CLIENTS 10

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
struct worker_state *workers_list;
int tamanho_atual = 0;

pthread_t *tid;
int tamanho_atual_tid = 0;

// int isNumeric(const char *str) {
//     // Verifica se todos os caracteres da string são numéricos
//     for (int i = 0; str[i] != '\0'; i++) {
//         if (!isdigit(str[i])) {
//             return 0; // Se encontrar um caractere não numérico, retorna falso
//         }
//     }
//     return 1; // Se todos os caracteres são numéricos, retorna verdadeiro
// }

struct worker_state {
    int socket_id;
    bool ocioso;
};

void *handle_client(void *arg) {
    int client_sockfd = *(int *)arg;
    char buffer[BUFFER_SIZE];
    int i = 0, worker_index_list = 0;
    int available_sock_id = -1;
    char result[100];

    while(1) {

        // Receive request from client
        memset(buffer, 0, BUFFER_SIZE);
        if (recv(client_sockfd, buffer, BUFFER_SIZE, 0) < 0) {
            perror("Error receiving request");
            // close(client_sockfd);
            // pthread_exit(NULL);
        }

        // printf("response client in server: %s", buffer);

        pthread_mutex_lock(&mutex);
        for(i; i < tamanho_atual; i++) {
            if(!workers_list[i].ocioso) {
                continue;
            } else {
                available_sock_id = workers_list[i].socket_id;
                workers_list[i].ocioso = false;
                worker_index_list = i;
            }
        }
        pthread_mutex_unlock(&mutex);

        if(available_sock_id < 0){
            printf("Sistema ocupado. Tente mais tarde.");
            close(client_sockfd);

            pthread_mutex_lock(&mutex);
            tamanho_atual_tid--;
            tid = (pthread_t *)realloc(tid, tamanho_atual_tid * sizeof(pthread_t));
            pthread_detach(*tid);
            pthread_mutex_unlock(&mutex);

            pthread_exit(NULL);
        }

        // Process request and send response
        char operation[32];
        double a, b;
        sscanf(buffer, "%s %lf %lf", operation, &a, &b);
        printf("Server received request: %s %.2lf %.2lf\n", operation, a, b);

        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, BUFFER_SIZE, "%s %.2lf %.2lf\n", operation, a, b);
        if (send(available_sock_id, buffer, strlen(buffer) + 1, 0) < 0) {
            perror("Error sending result");
            // close(client_sockfd);
            // pthread_exit(NULL);
        }

        if(recv(available_sock_id, result, sizeof(result), 0) < 0) {
            perror("Error receiving identification");
        }

        // printf("response worker in server: %s", result);

        pthread_mutex_lock(&mutex);
        workers_list[worker_index_list].ocioso = true;
        pthread_mutex_unlock(&mutex);
        
        // Send the result back to the client
        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, BUFFER_SIZE, "%.2lf", strtod(result, NULL));
        if (send(client_sockfd, buffer, strlen(buffer) + 1, 0) < 0) {
            perror("Error sending result");
            // close(client_sockfd);
            // pthread_exit(NULL);
        }
    }
}

int main() {
    int server_sockfd, sockfd;
    struct sockaddr_in server_addr, client_addr, worker_addr;
    socklen_t client_len, worker_len;
    char identification[7];
    int i = 0;
    
    workers_list = (struct worker_state *)malloc(tamanho_atual);
    tid = (pthread_t *)malloc(tamanho_atual_tid);
    
    if (workers_list == NULL) {
        perror("Erro ao alocar memória");
        exit(EXIT_FAILURE);
    }

    // Create socket
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    /* Configura servidor para receber conexoes de qualquer endereço:
	 * INADDR_ANY e ouvir na porta 8080 */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    /* Associa o socket a estrutura sockaddr_in */
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
        sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (sockfd < 0) {
            perror("Error accepting connection");
            continue;
        }

        if(recv(sockfd, identification, sizeof(identification), 0) < 0) {
            perror("Error receiving identification");
            continue;
        }

        if (strcmp(identification, "client") == 0) {
            // Handle client in a separate thread
            printf("Comunicação estabelecida com cliente %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            tamanho_atual_tid++;
            tid = (pthread_t *)realloc(tid, tamanho_atual_tid * sizeof(pthread_t));
            pthread_create(&tid[tamanho_atual_tid-1], NULL, handle_client, &sockfd);

            // pthread_detach(tid);
            
        } else if (strcmp(identification, "worker") == 0) {
            printf("Comunicação estabelecida com worker %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            tamanho_atual++;
            workers_list = (struct worker_state *)realloc(workers_list, tamanho_atual * sizeof(struct worker_state));
            workers_list[tamanho_atual-1].socket_id = sockfd;
            workers_list[tamanho_atual-1].ocioso = true;
        }
    }

    // Aguarda todas as threads terminarem
    for (i = 0; i < tamanho_atual_tid; i++) {
        pthread_join(tid[i], NULL);
    }

    // Close server socket
    close(server_sockfd);

    return 0;
}
