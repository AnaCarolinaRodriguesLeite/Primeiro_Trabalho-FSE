#include "alarme.h"

int securityAlarm = 0;
int smokeAlarm = 0;

void shoot() {
    if (securityAlarm || smokeAlarm) {
        attron(COLOR_PAIR(1));
        if (smokeAlarm && securityAlarm){
            mvprintw(0, 30, "Os alarmes de incêndio e segurança foram disparados!");
        }
        else if (securityAlarm){
            mvprintw(0, 30, "O alarme de segurança foi disparado!");
        }
        else{
            mvprintw(0, 30, "O alarme de incêndio foi disparado!");
        }
        beep();
    }
}

int controlSmokeAlarm(int statusSmoke) {
    smokeAlarm = statusSmoke;
    return smokeAlarm;
}

void controlSecurityAlarm(int command) {
    securityAlarm = command;
}

int statusAlarm(int type) {
    if (type == FUMACA)
        return smokeAlarm;
    return securityAlarm;
}