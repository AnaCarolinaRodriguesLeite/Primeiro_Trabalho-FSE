#ifndef ALARME_H
#define ALARME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "cJSON_Parse.h"

#define LIGA 1
#define DESLIGA 0
#define FUMACA 0
#define SEGURANCA 1

void changeAlarmState(char* nome, int estado);
void shoot();
void controlSecurityAlarm(int command);
int controlSmokeAlarm(int statusSmoke);
int statusAlarm(int type);

#endif