#ifndef DISTRIBUIDO_H
#define DISTRIBUIDO_H

#include "cJSON.h"

void processclient(int socketCliente);
void startListening(unsigned short servidorPorta);
cJSON* getMenssage();
void finishListening();
void encerraServidor();
#endif