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

  while(1){
  // A chaque connexion, on crée et ajoute un nouveau client
  if (accept_client(&srv.clients[srv.nb_clients]))
    exit(EXIT_FAILURE);
  else
    // Le client a été ajouté, on incrémente le nombre de clients
    srv.nb_clients++;

  // TODO: Gérer la recep des msg TCP, et la détection du client qui l'a envoyé
  // Pour codereq de 1 à 4 -> uint16
  // Pour les autres c'est chat
  // uint16_t message; recv(&message);
  // client_t client;

  // TODO: Gérer le type de la partie (lire le message)
  // msg_join_ready_t demande = mg_join(message),

  // TODO: Gérer la création de la partie
  // if (demande.type is not in srv.parties) {

  /* NOTE: A chaque fois qu'on a un type de partie qui n'est pas dans la liste,
   * on realloc la liste et on ajoute la nouvelle partie. */

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

  // On crée une partie
  partie_t partie = create_partie(srv.clients[0], params);

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
 * Lance le jeu.
 * @param partie La partie à lancer.
 * @ret
 */


/**
 * Crée une partie.
 * @param client Le client qui a demandé la partie.
 * @param params Les paramètres de la partie.
 * @return La partie créée.
 */


// FIXME: Fonction générée par copilot
int add_joueur(partie_t partie, client_t client) {
  // On crée un joueur
  joueur_t j = {0};
  j.client = client;
  j.id = partie.nb_joueurs;

  // On ajoute le joueur à la partie
  partie.joueurs[partie.nb_joueurs] = j;

  // On incrémente le nombre de joueurs
  partie.nb_joueurs++;

  // Si la partie est en mode 2 équipes, on répartit les joueurs
  if (partie.type == 1)
    /* NOTE: On choisit de les répartir en fonction de leur ordre
     * d'arrivée (i.e, de ordre de connexion) */
    j.team = partie.nb_joueurs % 2;

  return 0;
}

void receive_request(int sock) {
    uint16_t buffer;
    ssize_t bytes_received = 0;
    while (bytes_received < sizeof(buffer)) {
        ssize_t result = recv(sock, ((char*)&buffer) + bytes_received, sizeof(buffer) - bytes_received, 0);
        if (result == -1) {
            perror("recv");
            // Gérer l'erreur
        } else if (result == 0) {
            printf("La connexion a été fermée par le serveur.\n");
            // Gérer la fermeture de la connexion
        } else {
            bytes_received += result;
        }
    }

    // Convertir les octets reçus en big endian en valeurs host
    buffer = ntohs(buffer);
    uint16_t copie = buffer;
    // Extraire les bits
    int req = buffer >> 3; // Les 13 premiers bits
    if (req == 1 || req ==2){
      msg_join_ready_t msg = mg_join(copie);
    }
    else if (req == 3 || req == 4){
      msg_game_t msg = mg_game(copie);
    }
    else{
      // TODO tchat
    }
}
