# Todo

## Client: `client.c`

- Connexion en mode TCP au serveur.
- Envoi du mode de jeu au serveur
- `recv_game_data` : Attend la réception des données de la partie
- `abonnement` : Abonnement à l'adresse de multicast (connexion UDP)
- `ready` : Dire au serveur que le client est prêt
- `tchat_send` : Envoi d'un message au tchat
- `send_move` : Envoi d'un coup au serveur, en UDP

## Server: `server.c`

- `init_server` : Initialisation du serveur
  - Création d'une connexion TCP
- `recv_mode` : Reception du mode de jeu du client
  - Lecture du message
- attribution d'un identifiant unique et d'un numéro d'équipe au client
- check si une partie est en attente
  - Si oui, ajout du client à la partie
  - `create_game` : Création de la partie
    - adresse ipv6 et numéro de port de multicast
    - numéro de port de communication (msg UDP des joueurs de la partie)
- `send_game_data` : envoie des données au joueurs (cf. partie (c))
- `loading_players` : Attend que 4 joueurs soient inscrits à la partie
  - si 4 joueurs sont inscrits, attend que tous les joueurs soient prêts (e.g 2 min)
    - sinon, annule la partie
- `start_game` : Lance la partie
  - multidiffusion de la grille de jeu initiale, avec les 4 joueurs disposés aux 4 coins de la grille
- `game` : Gestion des coups des joueurs etc.
  - diffusion de la grille complète toutes les secondes
  - diffusion des cases modifiers toutes les `freq` ms
  - Vérification du nombre de joueurs $\to$ `end_game`
- `end_game` : Gestion de fin de jeu
  - Envoie l'identifiant du gagnant à tous les joueurs (en TCP)
  - Ferme les connexions TCP et UDP
- `tchat` : Gestion du tchat
  - Écoute sur son port les messages des joueurs
  - Envoie, en TCP, les messages aux joueurs concernés

## Structs, et fonctions nécessaires/annexes

- fonction de convertion des messages (pour récupérer les données ou les envoyer):
  - client: demande d'intégration d'une partie et s'annoncer comme prêt
  - client: coups joués
  - client: messages tchat
  - serveur: intégrer une partie
  - serveur: grille de jeu
  - serveur: grille modifiée
  - serveur: message tchat
  - serveur: fin de partie

Chaque partie est identifiée par son adresse de multidiffusion:

- une adresse ipv6 + un numéro de port de multidiffusion, sur lequel le serveur envoie des messages aux joueurs
- un numéro de port de communication (UDP), sur lequel le serveur attend les messages des joueurs de la partie
- un numéro de port (TCP) pour les messages de tchat et de fin de partie

Le serveur doit, de façon simultanée:

- écouter les messages sur le port de communication TCP pour le tchat
- examiner les requêtes des joueurs toutes les `freq` ms: comment sont stockées les différentes requêtes d'un joueur ?
- écouter les messages sur le port de communication UDP pour les coups des joueurs, toutes les `freq` ms

---

## Pour une partie

### Côté client

$\to$ D'abord, on init une **socket** (`sock_client`) avec laquelle on va recevoir et envoyer des messages en UDP au serveur.  
$\to$ Ensuite, on init une **adresse de réception** (`sockaddr_in6 r_adr`), avec son port `port_sc` d'écoute (stocké dans `r_adr.sin6_port`).  
$\to$ Nous on veut pouvoir écouter les messages qui arrivent sur le port `port_sc`. Pour cela, on lie notre `sock_client` à cette adresse (`bind(sock_client, r_adr)`). Ainsi, notre socket peut écouter sur le port `port_sc`, et on peut recevoir les paquets UDP à destination de n'importe quelle adresse ipv6 locale.  
$\to$ Mais nous, on ne veut recevoir que les messages de l'adresse d'abonnement du serveur. Pour cela, on init l'**adresse d'abonnement** (`struct ipv6_mreq g_adr`), qui contient l'adresse ipv6 d'abonnement (`adr_g`) et l'interface de multicast. On utilise `inet_pton(..., adr_g)`.  
$\to$ Puis, on s'inscris/s'abonne à cette adresse d'abonnement (`setsockopt(..., IPV6_JOIN_GROUP, ...)`).  
$\to$ Une fois qu'on s'est inscris, on peut recevoir des messages du serveur de ce "groupe" (`read(sock_client, buf, ...) ou recvfrom()` pour récupérer l'adresse de l'expéditeur).  
$\to$ On peut aussi envoyer des msg UDP au serveur, en utilisant la même socket mais le port `port_cs`

```c
// On init l'adresse de réception des messages du server
struct sockaddr_in6 r_adr;
// On met tout à 0
memset(&r_adr, 0, sizeof(r_adr));
// Socket en ipv6
r_adr.sin6_family = AF_INET6;
// On recoit des messages de l'adresse choisit par le serveur
inet_pton(AF_INET6, adr_g, &g_adr.sin6_addr);
// On initialise le port sur lequel on reçoit les paquets UDP
r_adr.sin6_port = htons(port_sc);

// Puis on lie la socket pour pouvoir recevoir les messages envoyés sur ce port
bind(sock_srv, (struct sockaddr *)&r_adr, sizeof(r_adr))

// On init l'adresse d'abonnement
struct ipv6_mreq g_adr;
// On met tout à 0
memset(&g_adr, 0, sizeof(g_adr));
// On initialise l'adresse d'abonnement
inet_pton(AF_INET6, adr_g, &g_adr.ipv6mr_multiaddr);
// On initialise l'interface de multicast
g_adr.ipv6mr_interface = 0; // 0 pour interface par défaut

// Puis on s'abonne à l'adresse d'abonnement
setsockopt(sock_srv, IPPROTO_IPV6, IPV6_JOIN_GROUP, &g_adr, sizeof(g_adr));

// On init l'adresse d'envoie des messages au serveur
struct sockaddr_in6 s_adr;
// On met tout à 0
memset(&s_adr, 0, sizeof(s_adr));
// Socket en ipv6
s_adr.sin6_family = AF_INET6;
// On initialise l'adresse ipv6 du serveur
inet_pton(AF_INET6, adr_s, &s_adr.sin6_addr);
// On initialise le port sur lequel on envoie les paquets UDP
s_adr.sin6_port = htons(port_cs);

/* ********** */
// On peut envoyer des messages au serveur
sendto(sock_srv, buf, strlen(buf), 0, (struct sockaddr *)&s_adr, sizeof(s_adr));

// On peut recevoir des messages des clients
recv(sock_srv, buf, strlen(buf), 0,);
```

### Côté serveur

$\to$ D'abord, on init une **socket** (`socket_srv`) avec laquelle on va recevoir et envoyer des msg en UDP au groupe de clients.  
$\to$ Ensuite, on init une **adresse multicast de groupe** (`sockaddr_in6 g_adr`), avec son port `port_sc` d'écoute (sotcké dans `g_adr.sin6_port`) et son adresse de multicast (`adr_g`).  
$\to$ Puis, on définit l'interface locale par laquelle partiront les paquets. (`g_adr.sin6_scope_id`).  
$\to$ On a aussi besoin d'une **adresse de réception**, pour réceptionner les msg des clients. Pour cela, on init l'adresse de réception (`sockaddr_in6 r_adr`), avec son port `port_cs`.  
$\to$ On lie notre socket à cette adresse d'écoute (`bind(sock_srv, r_adr)`).  
$\to$ On peut mtn envoyer et recevoir des msg UDP des clients du groupe:

```c
// On init l'adresse multicast de groupe
struct sockaddr_in6 g_adr;
// On met tout à 0
memset(&g_adr, 0, sizeof(g_adr));
// Socket en ipv6
g_adr.sin6_family = AF_INET6;
// On initialise l'adresse ipv6 de l'adresse de groupe
inet_pton(AF_INET6, adr_g, &g_adr.sin6_addr);
// On initialise le port sur lequel on envoie les paquets UDP
g_adr.sin6_port = htons(port_sc);
// On initialise l'interface locale de multicast
g_adr.sin6_scope_id = 0; // 0 pour interface par défaut

// On init l'adresse de réception des messages des clients du groupe
struct sockaddr_in6 r_adr;
// On met tout à 0
memset(&r_adr, 0, sizeof(r_adr));
// Socket en ipv6
r_adr.sin6_family = AF_INET6;
// On recoit des messages de n'importe quelle adresse locale
r_adr.sin6_addr = in6addr_any;
// On initialise le port sur lequel on reçoit les paquets UDP
r_adr.sin6_port = htons(port_cs);

// Puis on lit la socket pour pouvoir recevoir les messages envoyés sur ce port
bind(sock_srv, (struct sockaddr *)&r_adr, sizeof(r_adr))

/**********/

// On peut envoyer des messages aux clients du groupe
sendto(sock_srv, buf, strlen(buf), 0, (struct sockaddr *)&g_adr, sizeof(g_adr))

// On peut recevoir des messages des clients
recv(sock_srv, buf, strlen(buf), 0,);
```
