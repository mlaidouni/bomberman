#include "server.h"
#include "../lib/message.h"
#include "partie.h"

// DEFINES
#define SIZE_MESS 100

// VARIABLES
server_t srv = {0};

int main(int argc, char **args) {
  // NOTE: Le port devra être passé en argument
  const int port = 8080;
  // On initialise le port de la connexion TCP du serveur
  srv.tcp_port = port;
  // FIXME: On dit que la liste est toujours mallocée
  srv.parties.parties = malloc(sizeof(partie_t));
  // On dit que la liste des clients est toujours mallocée
  srv.clients = malloc(sizeof(client_t));

  // Création de la connexion TCP
  if (create_TCP_connection(port) < 0)
    exit(EXIT_FAILURE);

  while (1) {
    // A chaque connexion, on crée et ajoute un nouveau client
    if (accept_client(&srv.clients[srv.nb_clients]))
      exit(EXIT_FAILURE);
    else
      // Le client a été ajouté, on incrémente le nombre de clients
      srv.nb_clients++;

    // TODO: Gérer la recep des msg TCP, et la détection du client qui l'a
    // envoyé Pour codereq de 1 à 4 -> uint16 Pour les autres c'est chat
    // uint16_t message; recv(&message);
    // client_t client;

    // TODO: Gérer le type de la partie (lire le message)
    // msg_join_ready_t demande = mg_join(message),

    // TODO: Gérer la création de la partie
    // if (demande.type is not in srv.parties) {

    /* NOTE: A chaque fois qu'on a un type de partie qui n'est pas dans la
     * liste, on realloc la liste et on ajoute la nouvelle partie. */

    // partie_t partie = create_partie(client, demande);
    // Puis on lance start_game, et on la laisse gérer le lancement de la partie
    // start_game(partie);
    // } else {
    // partie_t partie = find_partie(demande.type);
    // add_joueur(partie, client);
    // }
  }
  /* TEST TO DELETE: On va supposer que c'est le premier client qui a créé la
   * partie */
  msg_join_ready_t demande = {0};
  demande.game_type = 1;
  uint16_t message = ms_join(demande);
  msg_join_ready_t params = mg_join(message);

  int finded_partie = find_partie(params.game_type);
  partie_t partie;
  // On cherche une partie de ce type
  if (finded_partie == -1)
    // Si on n'en trouve pas, on crée une nouvelle partie
    partie = create_partie(srv.clients[0], params);
  else {
    // Sinon, on ajoute le joueur à la partie trouvée
    partie = srv.parties.parties[finded_partie];
    add_joueur(partie, srv.clients[0]);
  }

  // On lance le jeu
  start_game(partie);

  // TEST TO DELETE: fin

  // Fermeture des sockets client et serveur
  close(srv.tcp_sock);

  return 0;
}

/* ********** Fonctions server ********** */

// Affiche les informations de connexion du client.
void affiche_connexion(struct sockaddr_in6 adrclient) {
  char adr_buf[INET6_ADDRSTRLEN];
  memset(adr_buf, 0, sizeof(adr_buf));

  inet_ntop(AF_INET6, &(adrclient.sin6_addr), adr_buf, sizeof(adr_buf));
  printf("adresse client : IP: %s port: %d\n", adr_buf,
         ntohs(adrclient.sin6_port));
}

/**
 * Crée une connexion TCP sur le port donné.
 * @param port Le port sur lequel le serveur doit écouter.
 * @param srv La structure server_t où stocker les informations de connexion.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int create_TCP_connection(int port) {
  // Création de la socket serveur
  int sock_srv = socket(PF_INET6, SOCK_STREAM, 0);
  if (sock_srv < 0) {
    perror("server.c: socket(): création de la socket échouée");
    return -1;
  }

  // Création de l'adresse du destinataire (serveur)
  struct sockaddr_in6 addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons(port);
  addr.sin6_addr = in6addr_any;

  // On lie la socket au port
  int r = bind(sock_srv, (struct sockaddr *)&addr, sizeof(addr));
  // Gestions des erreurs
  if (r < 0) {
    perror("server.c: bind(): bind échoué");
    return -1;
  }

  // Le serveur est prêt à écouter les connexions sur le port
  r = listen(sock_srv, 0);
  // Gestions des erreurs
  if (r < 0) {
    perror("server.c: listen(): listen échoué");
    return -1;
  }

  // Si tout s'est bien passé, on stocke les informations dans le serveur
  srv.tcp_sock = sock_srv;
  srv.tcp_port = port;
  srv.adr = addr;

  return 0;
}

/**
 * Accepte un client sur la socket TCP du serveur.
 * @param client La structure client_t où stocker les informations du client.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int accept_client(client_t *client) {
  /* Le serveur accepte une connexion et crée la socket de communication avec le
   * client */
  memset(&client->adr, 0, sizeof(client->adr));
  client->size = sizeof(client->adr);

  // TODO: Gérer le fait que accept() est bloquant
  // On accepte la connexion
  int sock_client =
      accept(srv.tcp_sock, (struct sockaddr *)&client->adr, &client->size);

  // Gestions des erreurs
  if (sock_client == -1) {
    perror("server.c: accept(): problème avec la socket client");
    return -1;
  }

  // Si tout s'est bien passé, on stocke la socket du client
  client->sock = sock_client;

  // On affiche les informations de connexion
  affiche_connexion(client->adr);

  return 0;
}

/**
 * Reçoit une requête sur la socket donnée.
 * @param sock La socket sur laquelle recevoir la requête.
 * @return La requête reçue.
 */
int receive_request() {
  // On crée un buffer pour stocker les octets reçus
  uint16_t *buffer = malloc(sizeof(uint16_t));
  // Le nombre d'octets reçus
  ssize_t bytes_received = 0;

  // On boucle tant qu'on n'a pas reçu les 2 octets
  while (bytes_received < sizeof(buffer)) {
    // On reçoit les octets
    ssize_t received = recv(srv.tcp_sock, buffer + bytes_received,
                            sizeof(uint16_t) - bytes_received, 0);

    // Gérer l'erreur
    if (received == -1)
      perror("server.c: receive_request(): recv");
    else if (!received) {
      fprintf(stderr, "server.c: receive_request(): connexion fermée\n");
      return -1;
    } else
      bytes_received += received;
  }

  // Convertir les octets reçus en Little Endian
  *buffer = ntohs(*buffer);

  // On sauvegarde une copie du buffer
  // uint16_t buffer_copie = *buffer; // WARNING: NE PAS TOUCHER CE BUFFER

  // On lit le code de requête (13 premiers bits)
  int codereq = *buffer >> 3;

  // TODOFIXME: Comment gérer les messages, que doit renvoyer cette fonction
  // ???? Si le codereq vaut 1 ou 2, c'est une requête de type 'join'
  if (codereq == 1 || codereq == 2) {
    // msg_join_ready_t msg = mg_join(buffer_copie);
  } else if (codereq == 3 || codereq == 4) {
    // Si le codereq vaut 3 ou 4, c'est une requête de type 'ready'

    // msg_ready_t msg = mg_ready(buffer_copie);
  } else {
    // Sinon, c'est un message de tchat
    /* TODO: lire les octets restants si c'est un message de tchat en bouclant
     * de nouveaux sur le recv */
  }

  // On libère le buffer
  free(buffer);

  return 0;
}
