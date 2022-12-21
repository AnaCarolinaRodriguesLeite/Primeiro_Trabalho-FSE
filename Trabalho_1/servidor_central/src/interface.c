#include "interface.h"

char** floors;
int qtdFloors, buildingPeople, currentFloor, alarm;
int* qtdPeople;
JSONData* sensor_states;

void startColors() {
    init_pair(DEFAULT, COLOR_WHITE, COLOR_BLACK);
    init_pair(RED, COLOR_RED, COLOR_BLACK);
    init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);
}

void selectColor(int status) {
    if (status == 1) {
        attron(COLOR_PAIR(GREEN));
    }
    else
        attron(COLOR_PAIR(RED));
}

void generalInformation() {
    int row = 1;
    attron(COLOR_PAIR(DEFAULT));

    mvprintw(0, 0, "Quantidade de pessoas no prédio: %d", buildingPeople);
    for (int i = 0; i < qtdFloors; i++)
        mvprintw(i + row, 0, "Pessoas no %s: %d", floors[i], qtdPeople[i]);
    attroff(COLOR_PAIR(DEFAULT));
}

void informations() {
    int row_init = qtdFloors + 3;
    int ident_column = 30;
    attron(COLOR_PAIR(DEFAULT));
    mvprintw(row_init, 0, "---------------- Informações do %s ----------------", floors[currentFloor]);
    attroff(COLOR_PAIR(DEFAULT));

    attron(COLOR_PAIR(BLUE));
    mvprintw(row_init + 2, 0, "Quantidade de pessoas: %d", qtdPeople[currentFloor]);

    if (sensor_states[currentFloor].temp == 0) {
        attron(COLOR_PAIR(DEFAULT));
        mvprintw(row_init + 3, 0, "Obtendo temperatura");
        mvprintw(row_init + 4, 6, "e umidade ...");
        attroff(COLOR_PAIR(DEFAULT));
    }
    else {
        mvprintw(row_init + 4, 0, "Temperatura: %.1lf°C", sensor_states[currentFloor].temp);
        mvprintw(row_init + 3, 0, "Umidade: %.1lf%%", sensor_states[currentFloor].umidade);
    }
    attroff(COLOR_PAIR(BLUE));

    selectColor(sensor_states[currentFloor].presenca);
    mvprintw(row_init + 2, ident_column, "Sensor de presença");
    selectColor(sensor_states[currentFloor].fumaca);
    mvprintw(row_init + 3, ident_column, "Sensor de fumaça");
    selectColor(sensor_states[currentFloor].janela);
    mvprintw(row_init + 4, ident_column, "Sensor de Janela");
    selectColor(sensor_states[currentFloor].porta);
    mvprintw(row_init + 5, ident_column, "Sensor de Porta");
}

void comandos() {
    attroff(COLOR_PAIR(GREEN));
    int row_init = qtdFloors + 12;
    int column_init = 0;
    int column_ident = 30;
    mvprintw(row_init, column_init, "-------------------- INPUTS --------------------");
    selectColor(sensor_states[currentFloor].lampada1);
    mvprintw(row_init + 2, column_init, "[1] Lâmpada 01");
    selectColor(sensor_states[currentFloor].lampada2);
    mvprintw(row_init + 3, column_init, "[2] Lâmpada 02");
    selectColor(sensor_states[currentFloor].projetor);
    mvprintw(row_init + 4, column_init, "[3] Projetor Multimidia");
    selectColor(sensor_states[currentFloor].ar_cond);
    mvprintw(row_init + 2, column_ident, "[4] Ar-Condicionado");

    if (strcmp(floors[currentFloor], "Sala 01") == 0 || strcmp(floors[currentFloor], "Sala 02") == 0) {
        selectColor(alarm);
        mvprintw(row_init + 3, column_ident, "[5] Alarme\n");
    }

    attroff(COLOR_PAIR(GREEN));
    attroff(COLOR_PAIR(RED));

    row_init += 6;

    mvprintw(row_init, 0, "-------------- Outputs Gerais --------------");
    mvprintw(row_init + 2, column_init, "[A] Ligar todos os sensores de saída das salas");
    mvprintw(row_init + 3, column_init, "[B] Desligar todos os sensores de saída das salas");

    mvprintw(row_init + 6, column_init, "[S] Sair?");

    if (qtdFloors > 1)
        mvprintw(row_init + 10, 8, "Aperte a tecla espaço para mudar de sala");
}

void menu(JSONData* status, char** _floors, int* qtds, int qtdFloors, int peopleCount, int floor, int alarm) {
    startColors();
    sensor_states = status;
    floors = _floors;
    qtdPeople = qtds;
    qtdFloors = qtdFloors;
    buildingPeople = peopleCount;
    currentFloor = floor;
    alarm = alarm;
    generalInformation();
    informations();
    comandos();
}