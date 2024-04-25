#include "partie.h"

int start_game(partie_t partie) {
  // TODO: Gérer le type de partie et les parties en attente

  while (partie.nb_joueurs < 4)
    // Si le nombre de joueurs est inférieur à 4, on attend des connexions
    ;

  // Si le nombre de joueurs est égal à 4, on lance et gère le jeu
  // Tant que la partie n'est pas terminée
  while (partie.end) {

    // TODO: Le jeu
    // ...

    // TODO: Penser à gérer la fermeture des sockets des clients
    // ...
  }

  return 0;
}

partie_t create_partie(client_t client, msg_join_ready_t params) {
  // Création de la structure partie
  partie_t partie = {0};
  partie.end = 1;
  partie.type = params.game_type;
  partie.nb_joueurs = 1;

  // Création d'un joueur
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

  // On ajoute la partie à la liste des parties
  srv.parties.parties[srv.parties.nb_parties] = partie;
  srv.parties.nb_parties++;
  return partie;
}