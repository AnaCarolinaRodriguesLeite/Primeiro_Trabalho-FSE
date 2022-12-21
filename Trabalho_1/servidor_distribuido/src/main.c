#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>
#include <ncurses.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include "temp.h"
#include "cJSON_Parse.h"
#include <cliente.h>
#include "distribuido.h"

float temp, umidade, tempAnt, umidadeAnt;
int altera;
JSONData info;
pthread_t threadCliente, threadDht22;


void ajusteSinal(int signum) {
    encerraServidor();
    pthread_cancel(threadCliente);
    pthread_cancel(threadDht22);
    exit(0);
}


int numeroGpio(IO* listaIO, int tamLista, char* nameSensor, int numberSensor) {
    int num = 1;
    for (int i = 0; i < tamLista; i++) {
        if (strcmp(nameSensor, listaIO[i].type) == 0) {
            if (num == numberSensor)
                return listaIO[i].gpio;
            num++;
        }
    }
    return -1;
}

void atualizaJson(StateSensor estados) {
    cJSON* status = buildJson(estados, info.porta_servidor_distribuido);
    char* mensage = cJSON_Print(status);

    int envio = envia(info.ip_servidor_central, info.porta_servidor_central, mensage);
    if (envio == -1) {
        printf("Não tem conexão com o servidor central!\n");
        ajusteSinal(envio);
    }
}

int compareStatus(StateSensor estado1, StateSensor estado2) {
    if (estado1.estado_entrada != estado2.estado_entrada || estado1.estado_saida != estado2.estado_saida)
        return 1;

    if (estado1.fumaca != estado2.fumaca || estado1.presenca != estado2.presenca || estado1.porta != estado2.porta)
        return 1;

    if (estado1.janela != estado2.janela)
        return 1;

    if (estado1.lampada1 != estado2.lampada1 || estado1.lampada2 != estado2.lampada2 || estado1.projetor != estado2.projetor || estado1.ar_cond != estado2.ar_cond)
        return 1;

    if (temp != tempAnt || umidade != umidadeAnt) {
        tempAnt = temp;
        umidadeAnt = umidade;
        return 1;
    }

    return 0;
}

StateSensor pega_estados() {

    StateSensor estados;
    int sensor_entrada = numeroGpio(info.inputs, info.qntd_inputs, "contagem", 1);
    int sensor_saida = numeroGpio(info.inputs, info.qntd_inputs, "contagem", 2);
    int sensor_presenca = numeroGpio(info.inputs, info.qntd_inputs, "presenca", 1);
    int sensor_fumaca = numeroGpio(info.inputs, info.qntd_inputs, "fumaca", 1);
    int sensor_janela = numeroGpio(info.inputs, info.qntd_inputs, "janela", 1);
    int sensor_ar_cond = numeroGpio(info.outputs, info.qntd_outputs, "ar-condicionado", 1);
    int sensor_lampada1 = numeroGpio(info.outputs, info.qntd_outputs, "lampada", 1);
    int sensor_lampada2 = numeroGpio(info.outputs, info.qntd_outputs, "lampada", 2);
    int sensor_projetor = numeroGpio(info.outputs, info.qntd_outputs, "projetor", 3);
    int sensor_porta = numeroGpio(info.inputs, info.qntd_inputs, "porta", 1);

    estados.estado_entrada = read_sensor_value(sensor_entrada);
    estados.estado_saida = read_sensor_value(sensor_saida);
    estados.presenca = read_sensor_value(sensor_presenca);
    estados.fumaca = read_sensor_value(sensor_fumaca);
    estados.janela = read_sensor_value(sensor_janela);
    estados.ar_cond = read_sensor_value(sensor_ar_cond);
    estados.lampada1 = read_sensor_value(sensor_lampada1);
    estados.lampada2 = read_sensor_value(sensor_lampada2);
    estados.projetor = read_sensor_value(sensor_projetor);

    estados.temp = temp;
    estados.umidade = umidade;
    if (sensor_porta != -1) {
        estados.porta = read_sensor_value(sensor_porta);
    }
    else {
        estados.porta = 0;

    }

    return estados;
}

void sendStatus() {
    StateSensor estados_iniciais;
    memset(&estados_iniciais, 0, sizeof(estados_iniciais));
    cJSON* json = buildJson(estados_iniciais, info.porta_servidor_distribuido);
    char* mensage = cJSON_Print(json);

    char* nome_andar = cJSON_Print(buildJsonToName(info.nome));
    while (envia(info.ip_servidor_central, info.porta_servidor_central, mensage) == -1) {
        printf("Esperando a conexão com o servidor central\n");
        sleep(2);
    }

    envia(info.ip_servidor_central, info.porta_servidor_central, nome_andar);
}

void altera_todos_sensore(int value) {
    for (int i = 0; i < info.qntd_outputs; i++) {
        if (strcmp(info.outputs[i].type, "aspersor") != 0)
            write_sensor_value(info.outputs[i].gpio, value);
    }
}


void* verificaSensores(void* args) {
    int cont = 0;
    StateSensor estados_anteriores = *(StateSensor*)args;
    StateSensor estados;

    while (1) {

        if (cont % 9 == 0) {
            cont = 0;
            estados = pega_estados();
        }
        else {
            int sensor_entrada = numeroGpio(info.inputs, info.qntd_inputs, "contagem", 1);
            int sensor_saida = numeroGpio(info.inputs, info.qntd_inputs, "contagem", 2);

            estados.estado_entrada = read_sensor_value(sensor_entrada);
            estados.estado_saida = read_sensor_value(sensor_saida);
        }
        if (compareStatus(estados, estados_anteriores))
            atualizaJson(estados);

        estados_anteriores = estados;
        cont++;
        usleep(50000);
    }
}

void* temperatura(void* args) {
    dht22Data dht22;
    tempAnt = 0;
    umidadeAnt = 0;
    int sensor_dht22 = numeroGpio(info.sensores, 1, "dht22", 1);
    while (1) {
        dht22 = get_dht_data(sensor_dht22);
        temp = dht22.temp;
        umidade = dht22.umi;
    }

}

int main(int argc, char** argv) {

    //leitura do json de configuracao
    if (argc != 2) {
        printf("make run FILE=<arquivo_entrada>\n");
        return 0;
    }

    if (parse(argv[1]) == -1)
        return 0;

    signal(SIGINT, ajusteSinal);

    info = getJSONData();
    initWiringPi();

    StateSensor args;
    cJSON* json;
    JSONMessage solicitacao;


    sendStatus();

    pthread_create(&(threadCliente), NULL, &verificaSensores, &args);
    pthread_create(&(threadDht22), NULL, &temperatura, &args);
    startListening(info.porta_servidor_distribuido);

    while (1) {
        json = getMenssage();
        solicitacao = parseMessage(json);
        int gpio = numeroGpio(info.outputs, info.qntd_outputs, solicitacao.sensor, solicitacao.numero);

        if (gpio != -1)
            write_sensor_value(gpio, solicitacao.comand);
        else
            altera_todos_sensore(solicitacao.comand);
    }
    finishListening();
    return 0;
}