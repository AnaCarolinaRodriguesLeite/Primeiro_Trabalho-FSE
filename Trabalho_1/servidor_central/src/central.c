#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "central.h"
#include "cJSON.h"

cJSON* json;
int server;
int client;
struct sockaddr_in addServer;
struct sockaddr_in addClient;
unsigned int sizeClient;

void processclient(int client) {
    char buffer[30000];
    int sizeReceived;

    if ((sizeReceived = recv(client, buffer, 30000, 0)) < 0){
        printf("Erro no recebimento - recv()\n");
    }
    buffer[sizeReceived] = '\0';
    
    while (sizeReceived > 0) {
        if (send(client, buffer, sizeReceived, 0) != sizeReceived){
            printf("Erro no envio - send()\n");
        }
        if ((sizeReceived = recv(client, buffer, 30000, 0)) < 0){
            printf("Erro no recebimento - recv()\n");
        }
    }
    json = cJSON_Parse(buffer);
}

void escutando(unsigned short portServer) {
    if ((server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        printf("Falha na conexão com o socket do servidor\n");

    // Estrutura
    memset(&addServer, 0, sizeof(addServer));
    addServer.sin_family = AF_INET;
    addServer.sin_addr.s_addr = htonl(INADDR_ANY);
    addServer.sin_port = htons(portServer);

    // Falha ao Ligar
    if (bind(server, (struct sockaddr*)&addServer, sizeof(addServer)) < 0)
        printf("Falha na ligação da conexão\n");

    // Quando ele falha na escuta
    if (listen(server, 10) < 0)
        printf("Falha na escuta\n");

}

cJSON* getMenssage() {
    sizeClient = sizeof(addClient);
    if ((client = accept(server, (struct sockaddr*) &addClient, &sizeClient)) < 0){
        printf("Falha no Accept\n");
    }
    processclient(client);
    close(client);
    return json;
}

void finishListening() {
    close(server);
}

void encerraServidor() {
    close(client);
    close(server);
}