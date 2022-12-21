#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "central.h"
#include "cliente.h"
#include "cJSON.h"
#include "cJSON_Parse.h"
#include "alarme.h"
#include "interface.h"

#define ARQUIVO "entradas.csv"
#define MAX_ANDAR 3

cJSON* json;
JSONData* estados_sensores;
pthread_t t1, thread_menu;

char** andares;
int* qntd_pessoas;
int sala = -1;
int pessoas_predio = 0;
int qntd_andares = 0;
int andar_atual = 0;
int alarme = 0;

void log_information(char* comando) {
    FILE* file;
    struct tm* data_hora;
    time_t segundos;

    time(&segundos);
    data_hora = localtime(&segundos);

    file = fopen(ARQUIVO, "a");

    if (file == NULL)
        return;

    fprintf(file, "%d/%d/%d,", data_hora->tm_mday, data_hora->tm_mon + 1, data_hora->tm_year + 1900);
    fprintf(file, "%d:%d:%d,", data_hora->tm_hour, data_hora->tm_min, data_hora->tm_sec);
    fprintf(file, "%s\n", comando);

    fclose(file);
}

int novaConexao(JSONData json_data) {
    int nova = 1;
    for (int i = 0; i < qntd_andares; i++) {
        if (estados_sensores[i].distribuido_porta == json_data.distribuido_porta) {
            return 0;
        }
    }
    return nova;
}

void atualizaAndar(JSONData json_data) {
    for (int i = 0; i < qntd_andares; i++) {
        if (estados_sensores[i].distribuido_porta == json_data.distribuido_porta) {
            estados_sensores[i] = json_data;
            return;
        }
    }
}

int  checaAndar(JSONData json_data) {
    for (int i = 0; i < qntd_andares; i++) {
        if (estados_sensores[i].distribuido_porta == json_data.distribuido_porta) {
            return i;
        }
    }
    return -1;
}

void concertaRetorno(JSONMessage mensagem) {
    if (strcmp("erro", mensagem.sensor) == 0) {
        attron(COLOR_PAIR(RED));
        mvprintw(0, 30, "Não foi possível comunicar com o servidor distribuído!");
        attroff(COLOR_PAIR(RED));
    }
    else {
        attron(COLOR_PAIR(GREEN));
        char msg_arquivo[150];
        char numero_sensor[5];
        int eh_alarme = strcmp(mensagem.sensor, "alarme") == 0;
        if (mensagem.comand == 1) {
            if (!eh_alarme)
                mvprintw(0, 30, "ligando %s . . .", mensagem.sensor);
            strcpy(msg_arquivo, "Liga ");
        }
        else {
            if (!eh_alarme)
                mvprintw(0, 30, "desligando %s . . .", mensagem.sensor);
            strcpy(msg_arquivo, "Desliga ");
        }
        strcat(msg_arquivo, mensagem.sensor);
        if (mensagem.numero > 1) {
            sprintf(numero_sensor, " %d", mensagem.numero);
            strcat(msg_arquivo, numero_sensor);
        }
        if (eh_alarme)
            strcat(msg_arquivo, " Ativado o alarme de incêndio!");

        else {
            strcat(msg_arquivo, " pelo ");
            strcat(msg_arquivo, andares[andar_atual]);
        }
        log_information(msg_arquivo);
        attroff(COLOR_PAIR(GREEN));
    }
}

void sendMensage(char* mensagem, unsigned short porta) {
    JSONMessage mensagem_json;
    char* mensgaem_retorno;
    mensgaem_retorno = envia("164.41.98.15", porta, mensagem);
    mensagem_json = parseMessage(mensgaem_retorno);
    concertaRetorno(mensagem_json);
}

void controlePredioTodo(int comand) {
    char* message = buildMessage("todos do prédio", 1, comand);
    for (int i = 0; i < qntd_andares; i++) {
        sendMensage(message, estados_sensores[i].distribuido_porta);
    }
}

void verificaAlarme(JSONData estados) {
    int estado_anterior_alarme = statusAlarm(SEGURANCA);
    if (alarme) {
        for (int i = 0;i < qntd_andares; i++) {
            if (estados_sensores[i].janela || estados_sensores[i].presenca || estados_sensores[i].porta) {
                if (estado_anterior_alarme == 0) {
                    log_information("Alarme de segurança foi disparado!");
                    controlSecurityAlarm(LIGA);
                }
                return;
            }
        }
    }
}

void trocaAlarme() {
    if (alarme) {
        alarme = DESLIGA;
        controlSecurityAlarm(DESLIGA);
        log_information("Alarme de segurança está desativado!");
    }
    else {
        int cond = 1;
        for (int i = 0; i < qntd_andares; i++) {
            if (estados_sensores[i].janela || estados_sensores[i].porta == 1 || estados_sensores[i].presenca) {
                cond = 0;
                break;
            }
        }
        if (cond) {
            alarme = LIGA;
            log_information("Alarme de segurança ACIONADO!!!!!");
        }
        else {
            attron(COLOR_PAIR(RED));
            mvprintw(0, 30, "Não foi possível acionar o alarme!");
            attroff(COLOR_PAIR(RED));

        }
    }
}

int liga_desliga(int estado) {
    if (estado == 1)
        return 0;
    return 1;
}


void* listeningServer(void* args) {
    unsigned short porta = *(unsigned short*)args;
    JSONData info;

    int estado_anterior_entrada = 0;
    int estado_anterior_saida = 0;
    int andar_json;

    estados_sensores = malloc(MAX_ANDAR * sizeof(JSONData));
    andares = malloc(MAX_ANDAR * sizeof(andares));
    qntd_pessoas = malloc(MAX_ANDAR * sizeof(int));

    escutando(porta);

    while (1) {
        json = getMenssage();
        info = parseJson(json);
        if (qntd_andares == 0 || novaConexao(info)) {
            cJSON* json_nome = getMenssage();
            char* nome_andar = getFloorName(json_nome);
            if (strcmp("Sala 01", nome_andar) == 0)
                sala = qntd_andares;

            andares[qntd_andares] = nome_andar;
            qntd_pessoas[qntd_andares] = 0;
            estados_sensores[qntd_andares] = info;

            qntd_andares++;
        }
        andar_json = checaAndar(info);

        estado_anterior_entrada = estados_sensores[andar_json].estado_entrada;
        estado_anterior_saida = estados_sensores[andar_json].estado_saida;

        if (strcmp(andares[andar_json], "Sala 01") == 0) {
            if (info.estado_entrada == 1 && estado_anterior_entrada == 0)
                pessoas_predio++;
            if (info.estado_saida == 1 && estado_anterior_saida == 0)
                pessoas_predio--;
        }
        else {
            if (info.estado_entrada == 1 && estado_anterior_entrada == 0)
                qntd_pessoas[andar_json]++;
            if (info.estado_saida == 1 && estado_anterior_saida == 0)
                qntd_pessoas[andar_json]--;
        }

        int soma = 0;
        for (int i = 0; i < qntd_andares; i++) {
            if (strcmp(andares[i], "Sala 01") != 0)
                soma += qntd_pessoas[i];
        }
        qntd_pessoas[sala] = pessoas_predio - soma;
        verificaAlarme(info);
        atualizaAndar(info);
    }

    finalizaEscuta();
}

void* comandoUsuario(void* args) {
    int* exit = (int*)args;
    char* message;
    int altera_andar = andar_atual;

    while (1) {
        char entrada_usuario = getch();
        message = NULL;
        switch (entrada_usuario) {
        case '1':
            message = buildMessage("lampada", 1, liga_desliga(estados_sensores[andar_atual].lampada1));
            break;
        case '2':
            message = buildMessage("lampada", 2, liga_desliga(estados_sensores[andar_atual].lampada2));
            break;
        case '3':
            message = buildMessage("projetor", 3, liga_desliga(estados_sensores[andar_atual].projetor));
            break;
        case '4':
            message = buildMessage("ar-condicionado", 1, liga_desliga(estados_sensores[andar_atual].ar_cond));
            break;
        case '5':
            if (andar_atual == sala)
                trocaAlarme();
            break;

        case 'S':
            *exit = 1;
            break;

        case 'A':
            message = buildMessage("todos do andar", 1, LIGA);
            break;

        case 'B':
            message = buildMessage("todos do andar", 1, DESLIGA);
            break;
        case 'C':
            controlePredioTodo(LIGA);
            break;
        case 'D':
            controlePredioTodo(DESLIGA);
            break;

        case ' ':
            altera_andar++;
            if (altera_andar == qntd_andares)
                altera_andar = 0;
            andar_atual = altera_andar;
            break;

        }
        if (message != NULL) {
            sendMensage(message, estados_sensores[andar_atual].distribuido_porta);
        }
    }
}
void init_file() {
    FILE* file;
    file = fopen(ARQUIVO, "w+");
    fprintf(file, "Data, Hora, Evento\n");
    fclose(file);
}

void trata_sinal(int sinal) {
    encerraServidor();
    free(estados_sensores);
    free(andares);
    pthread_cancel(thread_menu);
    pthread_cancel(t1);
    endwin();
    exit(0);
}

int main(void) {
    unsigned short porta = 10903;
    int exit = 0;

    signal(SIGINT, trata_sinal);
    init_file();
    initscr();
    curs_set(0);
    noecho();
    start_color();

    pthread_create(&t1, NULL, listeningServer, &porta);
    pthread_create(&thread_menu, NULL, comandoUsuario, &exit);

    while (qntd_andares == 0) {
        mvprintw(1, 5, "Aguardando a mensagem do servidor distribuído!");
        refresh();
        sleep(1);
    }

    while (1) {
        clear();
        menu(estados_sensores, andares, qntd_pessoas, qntd_andares, pessoas_predio, andar_atual, alarme);
        shoot();
        refresh();
        sleep(1);
        if (exit)
            break;;
    }
    trata_sinal(exit);
    return 0;
}