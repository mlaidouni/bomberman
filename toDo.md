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
