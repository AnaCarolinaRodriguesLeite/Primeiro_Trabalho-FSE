#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cliente.h"
#include "cJSON_Parse.h"

char* envia(char* ipServer, unsigned short portServer, char* mensage) {
    int clientSocket;
    struct sockaddr_in addServer;
    char buffer[300];
    unsigned int sizeMensage;
    int bytesReceived;
    int totalBytesReceived;
    char* mensageRetorn;

    // Criando um socket
    if ((clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        printf("Socket() deu erro\n");

    // Construindo  a struct da entrada dos endereços de rede
    memset(&addServer, 0, sizeof(addServer));
    addServer.sin_family = AF_INET;
    addServer.sin_addr.s_addr = inet_addr(ipServer);
    addServer.sin_port = htons(portServer);

    // Conexão
    if (connect(clientSocket, (struct sockaddr*)&addServer,
        sizeof(addServer)) < 0) {
        close(clientSocket);
        return buildMessage("Erro!", -1, -1);
    }


    sizeMensage = strlen(mensage);

    if (send(clientSocket, mensage, sizeMensage, 0) != sizeMensage)
        printf("Erro ao enviar bytes, valor diferente do que é esperado!\n");

    totalBytesReceived = 0;
    int erro = 0;
    while (totalBytesReceived < sizeMensage) {
        if ((bytesReceived = recv(clientSocket, buffer, 300 - 1, 0)) <= 0) {
            printf("Não foi compatível com os bytes que foram enviados!\n");
            mensageRetorn = buildMessage("Erro!", -1, -1);
            erro = 1;
        }
        totalBytesReceived += bytesReceived;
        buffer[bytesReceived] = '\0';
    }

    if (erro == 0) mensageRetorn = mensage;

    close(clientSocket);

    return mensageRetorn;
}