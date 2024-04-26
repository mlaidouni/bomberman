#ifndef PARTIE_H
#define PARTIE_H

#include "server.h"

int start_game(partie_t partie);
partie_t create_partie(client_t client, msg_join_ready_t params);

#endif