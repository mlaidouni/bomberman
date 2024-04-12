#include <stdint.h>

uint16_t m_join(int game_type)
{
    /* CODEREQ = 1 si la partie est en mode 4 joueurs, 2 si elle est en mode équipe.
    ID et EQ sont ignorés */
    uint16_t message = (game_type << 3) | (0 << 1) | 0;
    return message;
}

uint16_t m_ready(int game_type, int player_id, int team_id)
{
    /* CODEREQ = 1 si la partie est en mode 4 joueurs, 2 si elle est en mode équipe.
    ID et EQ sont ignorés */
    uint16_t message = (game_type << 3) | (player_id << 1) | team_id;
    return message;
}