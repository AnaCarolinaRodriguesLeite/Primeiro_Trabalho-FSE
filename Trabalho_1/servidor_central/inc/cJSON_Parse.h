#ifndef CJSON_INTERFACE_H
#define CJSON_INTERFACE_H

#include "cJSON.h"

typedef struct IO {
    char* type;
    int gpio;
    char* tag;
}IO;

typedef struct JSONMessage {
    char* sensor;
    int numero;
    int comand;
}JSONMessage;


typedef struct JSONData {
    int estado_entrada;
    int estado_saida;
    int presenca;
    int fumaca;
    int janela;
    int porta;
    int lampada1;
    int lampada2;
    int projetor;
    int ar_cond;
    unsigned short distribuido_porta;
    float umidade;
    float temp;
} JSONData;

JSONData parseJson(cJSON* json);
char* buildMessage(char* name, int num, int comand);
char* getFloorName(cJSON* json);
JSONMessage parseMessage(char* message);

#endif