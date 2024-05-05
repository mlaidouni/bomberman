#include "rules.h"

#include <assert.h>

int init_board(board board, TYPE type) {
	board.type = type;
	for(int i = 0; i < WIDTH * HEIGHT; i++) {
		board.grid[i] = EMPTY;
	}
	board.players[0] = (player) {0, ALIVE, {0, 0}};
	board.players[1] = (player) {0, ALIVE, {WIDTH - 1, HEIGHT - 1}};
	board.players[2] = (player) {1, ALIVE, {0, HEIGHT - 1}};
	board.players[3] = (player) {1, ALIVE, {WIDTH - 1, 0}};
	return 0;
}

int valid_pos(board board, pos p) {
	return p.x >= 0 && p.x < WIDTH && p.y >= 0 && p.y < HEIGHT;
}

int action_player(board board, int player, ACTION action) {
	// Rappel: x <= BOMB signifie x est EMPTY ou x est BOMB

	assert(player >= 0 && player < NB_PLAYERS);

	pos p = board.players[player].pos;
	switch (action) {
		case A_UP:
			if (p.y > 0) {
				if(board.grid[p.x + (p.y - 1) * WIDTH] <= BOMB) {
					p.y--;
					break;
				}
			}
			return -1;
		case A_DOWN:
			if (p.y < HEIGHT - 1) {
				if(board.grid[p.x + (p.y + 1) * WIDTH] <= BOMB) {
					p.y++;
					break;
				}
			}
			return -1;
		case A_LEFT:
			if (p.x > 0) {
				if(board.grid[(p.x - 1) + p.y * WIDTH] <= BOMB) {
					p.x--;
					break;
				}
			}
			return -1;
		case A_RIGHT:
			if (p.x < WIDTH - 1) {
				if(board.grid[(p.x + 1) + p.y * WIDTH] <= BOMB){
					p.x++;
					break;
				}
			}
			return -1;
		case A_BOMB:
			if(valid_pos(board, p) && board.grid[p.x + p.y * WIDTH] == EMPTY) {
				board.grid[p.x + p.y * WIDTH] = 'B';
				// TODO: timer pour faire peter la bombe
				// Attention: si la bombe se fait peter par une autre
				// bombe, il faut desactiver le timer
			}
			else
				return -1;
			break;
		default:
			return -1;
	}
	board.players[player].pos = p;
	return 0;
}

int damage_pos(board board, pos p) {
	if(!valid_pos(board, p)) {
		return -1;
	}
	if(board.grid[p.x + p.y * WIDTH] == BOMB) {
		explode_bomb(board, p);
		return 0;
	}
	if(board.grid[p.x + p.y * WIDTH] == DEST_WALL) {
		board.grid[p.x + p.y * WIDTH] = EMPTY;
		return 1;
	}
	if(board.grid[p.x + p.y * WIDTH] == INDEST_WALL) {
		return 2;
	}
	for(int i = 0; i < NB_PLAYERS; i++) {
		if(board.players[i].pos.x == p.x && board.players[i].pos.y == p.y) {
			board.players[i].status = DEAD;
		}
	}
	return 0;
}

int explode_bomb(board board, pos bomb_pos) {
	if(!valid_pos(board, bomb_pos) || board.grid[bomb_pos.x + bomb_pos.y * WIDTH] != BOMB)
		return -1;
	board.grid[bomb_pos.x + bomb_pos.y * WIDTH] = EMPTY;

	if(!damage_pos(board, (pos) {bomb_pos.x - 1, bomb_pos.y})) {
		damage_pos(board, (pos) {bomb_pos.x - 2, bomb_pos.y});
	}

	if(!damage_pos(board, (pos) {bomb_pos.x + 1, bomb_pos.y})) {
		damage_pos(board, (pos) {bomb_pos.x + 2, bomb_pos.y});
	}

	if(!damage_pos(board, (pos) {bomb_pos.x, bomb_pos.y - 1})) {
		damage_pos(board, (pos) {bomb_pos.x, bomb_pos.y - 2});
	}

	if(!damage_pos(board, (pos) {bomb_pos.x, bomb_pos.y + 1})) {
		damage_pos(board, (pos) {bomb_pos.x, bomb_pos.y + 2});
	}

	damage_pos(board, (pos) {bomb_pos.x - 1, bomb_pos.y - 1});
	damage_pos(board, (pos) {bomb_pos.x + 1, bomb_pos.y - 1});
	damage_pos(board, (pos) {bomb_pos.x - 1, bomb_pos.y + 1});
	damage_pos(board, (pos) {bomb_pos.x + 1, bomb_pos.y + 1});

	return 0;
}


/*
	Renvoie -1 si la partie n'est pas terminée
	Renvoie le numéro de l'équipe gagnante sinon
*/
int winner(board board) {
	int nb_alive = 0;
	int last_team = -1;
	for(int i = 0; i < NB_PLAYERS; i++) {
		if(board.players[i].status == ALIVE) {
			nb_alive++;
			if(last_team == -1) {
				last_team = board.players[i].team;
			} else if(last_team != board.players[i].team) {
				return -1;
			}
		}
	}
	if(nb_alive >= 1) {
		return last_team;
	}
	return -1;
}
