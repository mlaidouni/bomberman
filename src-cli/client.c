#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "../lib/message.h"

/**
 * @brief Crée une socket client et se connecte à un serveur
 * @param port Le port du serveur
 * @return Le descripteur de la socket client
 */
int connect_to_server(int port)
{
    // Création de la socket client
    int sock_client = socket(PF_INET6, SOCK_STREAM, 0);
    if (sock_client == -1)
    {
        perror("src-cli/client.c: socket");
        exit(EXIT_FAILURE);
    }

    // Préparation de l'adresse du destinataire (le serveur)
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port);
    addr.sin6_addr = in6addr_any;

    // Demande de connexion au serveur
    int r = connect(sock_client, (struct sockaddr *)&addr, sizeof(addr));
    if (r)
    {
        perror("src-cli/client.c: connect()");
        close(sock_client);
        return -1;
    }
    return sock_client;
}

// Demande d'intégration d'une partie
int join_game(int sock_client, int game_type)
{
    // Création du message
    uint16_t message = m_join(game_type);
    // Envoi du message
    int r = send(sock_client, &message, sizeof(message), 0);
    if (r == -1)
    {
        perror("src-cli/client.c: join_game: send message failed");
        close(sock_client);
        return -1;
    }
    return 0;
}

// Annoncer que l'on est prêt
int ready(int sock_client, int game_type, int player_id, int team_id)
{
    // Création du message
    uint16_t message = m_ready(game_type, player_id, team_id);
    // Envoi du message
    int r = send(sock_client, &message, sizeof(message), 0);
    if (r == -1)
    {
        perror("src-cli/client.c: ready: send message failed");
        close(sock_client);
        return -1;
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    const int port = 7878;
    // Connexion en mode TCP au serveur
    int sock_client = connect_to_server(port);
    printf("Connected to server\n");

    // Envoi du mode de jeu au serveur

    // On attend la réception des données de la partie

    // On s'abonne à l'adresse de multicast (connexion UDP)

    // On s'annonce prêt au serveur

    // ...

    // Fermeture de la socket client: à la fin de la partie seulement
    close(sock_client);
    return 0;
}
