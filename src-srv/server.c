#include "server.h"

#define SIZE_MESS 100

int compteur = 0;

void affiche_connexion(struct sockaddr_in6 adrclient) {
  char adr_buf[INET6_ADDRSTRLEN];
  memset(adr_buf, 0, sizeof(adr_buf));

  inet_ntop(AF_INET6, &(adrclient.sin6_addr), adr_buf, sizeof(adr_buf));
  printf("adresse client : IP: %s port: %d\n", adr_buf,
         ntohs(adrclient.sin6_port));
}

int main(int argc, char **args) {
  // NOTE: Le port devra être passé en argument
  const int port = 8080;

  // Création de la socket serveur
  int sock_srv = socket(PF_INET6, SOCK_STREAM, 0);
  if (sock_srv < 0) {
    perror("server.c: socket(): création de la socket échouée");
    exit(1);
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
    exit(1);
  }

  // Le serveur est prêt à écouter les connexions sur le port
  r = listen(sock_srv, 0);
  // Gestions des erreurs
  if (r < 0) {
    perror("server.c: listen(): listen échoué");
    exit(1);
  }

  /* Le serveur accepte une connexion et crée la socket de communication avec le
   * client */
  struct sockaddr_in6 adrclient;
  memset(&adrclient, 0, sizeof(adrclient));
  socklen_t size = sizeof(adrclient);
  int sockclient = accept(sock_srv, (struct sockaddr *)&adrclient, &size);
  // Gestions des erreurs
  if (sockclient == -1) {
    perror("server.c: accept(): problème avec la socket client");
    exit(1);
  }

  // Création d'un struct joueur
  struct joueur j;
  j.id = compteur; compteur++;
  j.adr = adrclient;
  tab_joueur[0] = j;

  bool end = false;

  void start_game(struct joueur joueurs[]) {
      while(!end){
          // Si un client se connecte au server et que le nombre de joueur est inférieur à 4, on l'ajoute au tableau de joueur
          if (accept(sock_srv, (struct sockaddr *)&adrclient, &size) > 0 && (compteur < 4)){
            // Création d'un nouveau joueur
            struct joueur j;
            j.id = compteur; compteur++;
            j.adr = adrclient;
            tab_joueur[compteur] = j;
          }
      }
  }

  // Fermeture des sockets client et serveur
  close(sockclient);
  close(sock_srv);

  return 0;
}
