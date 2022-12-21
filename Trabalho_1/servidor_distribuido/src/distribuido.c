#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "distribuido.h"
#include "cJSON.h"

cJSON* json;
int serverSocket;
int socketClient;
struct sockaddr_in addServer;
struct sockaddr_in addClient;
unsigned int clientSize;

void processclient(int socketCliente) {
    char buffer[300];
    int sizeReceived;

    if ((sizeReceived = recv(socketCliente, buffer, 300, 0)) < 0)
        printf("Erro no recv()\n");

    buffer[sizeReceived] = '\0';

    while (sizeReceived > 0) {
        if (send(socketCliente, buffer, sizeReceived, 0) != sizeReceived)
            printf("Erro no envio - send()\n");

        if ((sizeReceived = recv(socketCliente, buffer, 300, 0)) < 0)
            printf("Erro no recv()\n");
    }

    json = cJSON_Parse(buffer);
}

void startListening(unsigned short portServer) {
    if ((serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        printf("falha no socker do Servidor\n");

    // Construindo  a struct da entrada dos endereços de rede
    memset(&addServer, 0, sizeof(addServer));
    addServer.sin_family = AF_INET;
    addServer.sin_addr.s_addr = htonl(INADDR_ANY);
    addServer.sin_port = htons(portServer);

    // Falha ao Ligar
    if (bind(serverSocket, (struct sockaddr*)&addServer, sizeof(addServer)) < 0)
        printf("Falha na ligação da conexão!\n");

    // Quando ele falha na escuta
    if (listen(serverSocket, 10) < 0)
        printf("Falha ao escutar!\n");

}

cJSON* getMenssage() {
    clientSize = sizeof(addClient);
    if ((socketClient = accept(serverSocket, (struct sockaddr*)&addClient, &clientSize)) < 0)
        printf("Falha ao aceitar a conexão!\n");

    processclient(socketClient);
    close(socketClient);

    return json;
}

void finishListening() {
    close(serverSocket);
}

void encerraServidor() {
    close(socketClient);
    close(serverSocket);
}