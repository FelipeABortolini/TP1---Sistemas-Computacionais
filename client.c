/* This client connects to the server and 
   sends requistions to the server.
*/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/tcp.h>

#define BUFFER_SIZE 1024
#define PORT 8080


int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in server_addr;
    const char client_hello[] = "client";
    char buffer[BUFFER_SIZE];
    
    if (argc < 3){
        printf("Usage: %s <ip address> <list of parameters>\n", argv[0]);
        exit(0);
    }

    // Create socket
    //sockfd = ;
    if (socket(AF_INET, SOCK_STREAM, 0) < 0) {
        perror("Error creating socket");
        return 1;
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    //server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        return 1;
    }

    // Send message identifying itself as a client
    if (send(sockfd, client_hello, strlen(client_hello) + 1, 0) < 0) {
        perror("Error sending client hello");
        return 1;
    }
    
    /* Aguarda o recebimento de dados do servidor. 
	 * Enquanto n for maior que 0. */
    while ( (n = recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0)) > 0)
    {
		/* Coloca null no final da string. */
        recvBuff[n] = '\0';
        if(fputs(recvBuff, stdout) == EOF)
        {
            perror("fputs");
        }
    } 

    // Send request to server
    // Implement your logic here

    // Receive response from server
    // Implement your logic here

    // Close connection
    close(sockfd);

    return 0;
}



    
