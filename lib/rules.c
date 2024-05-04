#include "rules.h"

#include <assert.h>

int init_board(board board, TYPE type) {
	board.type = type;
	for(int i = 0; i < WIDTH * HEIGHT; i++) {
		board.grid[i] = EMPTY;
	}
	board.players[0] = {0, {0, 0}};
	board.players[1] = {0, {WIDTH - 1, HEIGHT - 1}};
	board.players[2] = {1, {0, HEIGHT - 1}};
	board.players[3] = {1, {WIDTH - 1, 0}};
	return 0;
}

int valid_pos(board board, pos p) {
	return p.x >= 0 && p.x < WIDTH && p.y >= 0 && p.y < HEIGHT;
}

int action_player(board board, int player, ACTION action) {
	// Rappel: x <= BOMB signifie x est EMPTY ou x est BOMB

	assert(player >= 0 && player < board.nb_players);

	pos p = board.players[player].pos;
	switch (action) {
		case UP:
			if (p.y > 0) {
				if(board.grid[p.x + (p.y - 1) * WIDTH] <= BOMB) {
					p.y--;
					break;
				}
			}
			return -1;
		case DOWN:
			if (p.y < HEIGHT - 1) {
				if(board.grid[p.x + (p.y + 1) * WIDTH] <= BOMB) {
					p.y++;
					break;
				}
			}
			return -1;
		case LEFT:
			if (p.x > 0) {
				if(board.grid[(p.x - 1) + p.y * WIDTH] <= BOMB) {
					p.x--;
					break;
				}
			}
			return -1;
		case RIGHT:
			if (p.x < WIDTH - 1) {
				if(board.grid[(p.x + 1) + p.y * WIDTH] <= BOMB){
					p.x++;
					break;
				}
			}
			return -1;
		case BOMB:
			if(valid_pos(board, p) && board.grid[p.x + p.y * WIDTH] == EMPTY)
				board.grid[p.x + p.y * WIDTH] = 'B';
			else
				return -1;
			break;
		default:
			return -1;
	}
	board.players[player].pos = p;
	return 0;
}

int explode_bomb(board board, pos bomb_pos) {
	if(!valid_pos(board, bomb_pos) || board.grid[bomb_pos.x + bomb_pos.y * WIDTH] != BOMB)
		return -1;
	board.grid[bomb_pos.x + bomb_pos.y * WIDTH] = EMPTY;
	// TODO: Explode the bomb


	return 0;
}
