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

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
struct worker_state *workers_list;
int tamanho_atual_workers = 0;

pthread_t *tid;
int tamanho_atual_tid = 0;

struct worker_state {
    int socket_id;
    bool ocioso;
};

void *handle_client(void *arg) {
    int client_sockfd = *(int *)arg;
    char buffer[BUFFER_SIZE];
    int i = 0, worker_index_list = 0;
    int available_worker_sock_id = -1;
    char result[100];

    while(1) {
        available_worker_sock_id = -1;

        // Recebe a requisição do cliente
        memset(buffer, 0, BUFFER_SIZE);
        if (recv(client_sockfd, buffer, BUFFER_SIZE, 0) < 0) {
            perror("Erro ao receber requisição.\n");
            close(client_sockfd);
            pthread_exit(NULL);
        }

        if(strcmp(buffer, "") == 0){
            memset(buffer, 0, BUFFER_SIZE);
            snprintf(buffer, BUFFER_SIZE, "quit");
            printf("%s - %i\n", buffer, client_sockfd);
            if (send(client_sockfd, buffer, strlen(buffer) + 1, 0) < 0) {
                perror("Erro ao enviar resultado ao cliente.\n");
                close(client_sockfd);
                pthread_exit(NULL);
            }
        } else {
            pthread_mutex_lock(&mutex1);
            for(i = 0; i < tamanho_atual_workers; i++) {
                if(available_worker_sock_id < 0) {
                    if(workers_list[i].ocioso) {
                        available_worker_sock_id = workers_list[i].socket_id;
                        workers_list[i].ocioso = false;
                        worker_index_list = i;
                        break;
                    }
                }
            }
            pthread_mutex_unlock(&mutex1);

            if(available_worker_sock_id < 0){
                // Envia resultado de volta ao cliente
                memset(buffer, 0, BUFFER_SIZE);
                snprintf(buffer, BUFFER_SIZE, "Sistema ocupado. Tente mais tarde.");
                if (send(client_sockfd, buffer, strlen(buffer) + 1, 0) < 0) {
                    perror("Erro ao enviar informação de sistema ocupado para cliente.\n");
                }
                close(client_sockfd);
                printf("Conexão com cliente %i encerrada por falta de workers disponíveis.\n", client_sockfd);

                pthread_mutex_lock(&mutex2);
                pthread_detach(*(tid + sizeof(pthread_t) * tamanho_atual_tid));
                tamanho_atual_tid--;
                tid = (pthread_t *)realloc(tid, tamanho_atual_tid * sizeof(pthread_t));
                pthread_mutex_unlock(&mutex2);

                pthread_exit(NULL);
            }
            // Processa a requisição e envia o resultado
            char operation[32];
            double a, b;
            sscanf(buffer, "%s %lf %lf", operation, &a, &b);
            printf("Servidor recebeu requisição: %s %.2lf %.2lf - %i\n", operation, a, b, client_sockfd);

            memset(buffer, 0, BUFFER_SIZE);
            snprintf(buffer, BUFFER_SIZE, "%s %.2lf %.2lf\n", operation, a, b);
            if (send(available_worker_sock_id, buffer, strlen(buffer) + 1, 0) < 0) {
                perror("Erro ao enviar operação ao worker.\n");
                close(client_sockfd);
                pthread_exit(NULL);
            }

            if(recv(available_worker_sock_id, result, sizeof(result), 0) < 0) {
                perror("Erro ao receber resultado do worker.\n");
                close(client_sockfd);
                pthread_exit(NULL);
            }

            sleep(0.1);

            pthread_mutex_lock(&mutex1);
            workers_list[worker_index_list].ocioso = true;
            pthread_mutex_unlock(&mutex1);

            // Envia resultado de volta ao cliente
            memset(buffer, 0, BUFFER_SIZE);
            snprintf(buffer, BUFFER_SIZE, "%.2lf", strtod(result, NULL));
            if (send(client_sockfd, buffer, strlen(buffer) + 1, 0) < 0) {
                printf("%i.\n", client_sockfd);
                perror("Erro ao enviar resultado ao cliente.\n");
                close(client_sockfd);
                pthread_exit(NULL);
            }
        }
        close(client_sockfd);
        printf("saindo thread.\n");
        pthread_exit(NULL);
    }
}

int main() {
    int server_sockfd, sockfd;
    struct sockaddr_in server_addr, client_addr, worker_addr;
    socklen_t client_len, worker_len;
    char identification[7];
    int i = 0;
    
    workers_list = (struct worker_state *)malloc(tamanho_atual_workers * sizeof(struct worker_state));
    tid = (pthread_t *)malloc(tamanho_atual_tid * sizeof(pthread_t));

    
    if (workers_list == NULL) {
        perror("Erro ao alocar memória.\n");
        exit(EXIT_FAILURE);
    }
    // Cria socket
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0) {
        perror("Erro ao criar socket.\n");
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
        perror("Erro ao associar socket.\n");
        exit(EXIT_FAILURE);
    }

    // Escuta conexões
    if (listen(server_sockfd, 5) < 0) {
        perror("Erro ao ouvir as conexões.\n");
        exit(EXIT_FAILURE);
    }

    printf("Servidor ouvindo na porta %d...\n", PORT);

    while (1) {
        // Aceita conexões
        client_len = sizeof(client_addr);
        sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (sockfd < 0) {
            perror("Erro ao receber conexão.\n");
            continue;
        }

        if(recv(sockfd, identification, sizeof(identification), 0) < 0) {
            perror("Erro ao receber identificação.\n");
            continue;
        }

        if (strcmp(identification, "client") == 0) {
            printf("Comunicação estabelecida com cliente %s:%d - %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), sockfd);
            tamanho_atual_tid++;
            tid = (pthread_t *)realloc(tid, tamanho_atual_tid * sizeof(pthread_t));
            if (tid == NULL) {
                perror("Erro ao realocar array de threads");
                close(sockfd);  // Fecha o socket em caso de erro
                exit(EXIT_FAILURE);
            }
            pthread_create(&tid[tamanho_atual_tid-1], NULL, handle_client, &sockfd);
            
        } else if (strcmp(identification, "worker") == 0) {
            printf("Comunicação estabelecida com worker %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            tamanho_atual_workers++;
            workers_list = (struct worker_state *)realloc(workers_list, tamanho_atual_workers * sizeof(struct worker_state));
            workers_list[tamanho_atual_workers-1].socket_id = sockfd;
            workers_list[tamanho_atual_workers-1].ocioso = true;
        }
    }

    // Aguarda todas as threads terminarem
    for (i = 0; i < tamanho_atual_tid; i++) {
        pthread_join(tid[i], NULL);
    }

    // Fecha socket do servidor
    close(server_sockfd);

    return 0;
}
