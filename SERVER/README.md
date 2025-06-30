# Zappy Server

Le serveur Zappy est le composant central du jeu Zappy. Il gère la simulation du monde, les équipes de joueurs, les ressources, et communique avec les clients IA et l'interface graphique.

## Table des matières

- [Compilation](#compilation)
- [Utilisation](#utilisation)
- [Architecture](#architecture)
- [Protocole de communication](#protocole-de-communication)
- [Commandes des joueurs IA](#commandes-des-joueurs-ia)
- [Commandes de l'interface graphique](#commandes-de-linterface-graphique)
- [Système de ressources](#système-de-ressources)
- [Système de niveaux](#système-de-niveaux)
- [Gestion du temps](#gestion-du-temps)

## Compilation

Pour compiler le serveur, utilisez le Makefile fourni :

```bash
make
```

Cela génère l'exécutable `zappy_server`.

Pour nettoyer les fichiers objets :
```bash
make clean
```

Pour nettoyer complètement (y compris l'exécutable) :
```bash
make fclean
```

Pour recompiler entièrement :
```bash
make re
```

## Utilisation

### Syntaxe

```bash
./zappy_server -p port -x width -y height -n name1 name2 ... -c clientsNb -f freq
```

### Paramètres obligatoires

- `-p port` : Port d'écoute du serveur
- `-x width` : Largeur du monde (en cases)
- `-y height` : Hauteur du monde (en cases)
- `-n name1 name2 ...` : Noms des équipes (au moins 2)
- `-c clientsNb` : Nombre maximum de clients autorisés par équipe au début
- `-f freq` : Fréquence du serveur (inverse de l'unité de temps pour l'exécution des actions)

### Exemple

```bash
./zappy_server -p 4242 -x 20 -y 20 -n team1 team2 team3 -c 6 -f 100
```

Cet exemple :
- Lance le serveur sur le port 4242
- Crée un monde de 20x20 cases
- Définit 3 équipes : team1, team2, team3
- Autorise 6 clients maximum par équipe au début
- Définit la fréquence à 100 (une unité de temps = 1/100 seconde)

## Architecture

### Structure du serveur

Le serveur est organisé en plusieurs modules :

- **Core** : Gestion principale du serveur (`server_main.c`, `server_init.c`)
- **Network** : Gestion des connexions clients (`client_handling.c`)
- **Commands** : Traitement des commandes
  - `command_server/` : Commandes des joueurs IA
  - `command_gui/` : Commandes de l'interface graphique
- **Map** : Gestion de la carte et des ressources (`map/`)
- **Time** : Gestion du temps et des ticks (`time/`)
- **Players** : Gestion des joueurs (`player.c`)
- **Teams** : Gestion des équipes (`team.c`)

### Types de clients

Le serveur gère deux types de clients :
1. **Clients IA** : Joueurs contrôlés par intelligence artificielle
2. **Client graphique** : Interface graphique pour visualiser le jeu

## Protocole de communication

### Connexion des clients IA

1. Le client se connecte au serveur
2. Le serveur envoie "WELCOME\n"
3. Le client envoie le nom de son équipe
4. Le serveur répond avec les informations de connexion :
   ```
   client_num
   X Y
   ```
   Où :
   - `client_num` : Nombre de places restantes dans l'équipe
   - `X Y` : Dimensions du monde

### Connexion du client graphique

1. Le client se connecte au serveur
2. Le serveur envoie "WELCOME\n"
3. Le client envoie "GRAPHIC"
4. Le serveur confirme la connexion graphique

## Commandes des joueurs IA

Toutes les commandes des joueurs IA ont une durée d'exécution en unités de temps.

### Commandes de mouvement

| Commande | Durée | Description | Réponse |
|----------|-------|-------------|---------|
| `Forward` | 7 | Avance d'une case dans la direction courante | `ok` |
| `Right` | 7 | Tourne de 90° vers la droite | `ok` |
| `Left` | 7 | Tourne de 90° vers la gauche | `ok` |

### Commandes d'information

| Commande | Durée | Description | Réponse |
|----------|-------|-------------|---------|
| `Look` | 7 | Regarde autour du joueur | `[ case1, case2, ... ]` |
| `Inventory` | 1 | Affiche l'inventaire | `[ food n, linemate n, ... ]` |
| `Connect_nbr` | 0 | Nombre de places libres dans l'équipe | `n` |

#### Format de la réponse Look

La commande `Look` retourne le contenu des cases visibles sous forme de liste. La vision s'étend selon le niveau du joueur :
- Niveau 1 : 3 cases (case courante + 1 devant)
- Niveau 2 : 8 cases
- etc.

Chaque case contient une liste d'éléments séparés par des espaces :
```
[ player food linemate, food, linemate deraumere, ... ]
```

#### Format de la réponse Inventory

```
[ food 10, linemate 3, deraumere 1, sibur 0, mendiane 2, phiras 1, thystame 0 ]
```

### Commandes d'action

| Commande | Durée | Description | Réponse |
|----------|-------|-------------|---------|
| `Take <resource>` | 7 | Prend une ressource sur la case | `ok` / `ko` |
| `Set <resource>` | 7 | Pose une ressource sur la case | `ok` / `ko` |
| `Eject` | 7 | Expulse tous les joueurs de la case | `ok` |
| `Broadcast <message>` | 7 | Diffuse un message | `ok` |
| `Incantation` | 300 | Commence une incantation | `ok` / `ko` |
| `Fork` | 42 | Pond un œuf | `ok` |

#### Ressources disponibles

- `food` : Nourriture (nécessaire pour survivre)
- `linemate` : Pierre précieuse niveau 1
- `deraumere` : Pierre précieuse niveau 2
- `sibur` : Pierre précieuse niveau 3
- `mendiane` : Pierre précieuse niveau 4
- `phiras` : Pierre précieuse niveau 5
- `thystame` : Pierre précieuse niveau 6

#### Commande Broadcast

La commande `Broadcast` permet d'envoyer un message à tous les joueurs de toutes les équipes. Les autres joueurs reçoivent :
```
message K, <texte>
```
Où `K` est une valeur de 1 à 8 indiquant la direction approximative de l'émetteur.

#### Commande Incantation

L'incantation permet de monter de niveau. Elle nécessite :
1. Le bon nombre de joueurs du bon niveau sur la même case
2. Les bonnes ressources sur la case

Les conditions par niveau sont définies dans le code et suivent les règles du jeu Zappy.

## Commandes de l'interface graphique

L'interface graphique utilise un protocole spécifique pour obtenir des informations sur l'état du jeu.

### Commandes d'information sur la carte

| Commande | Description | Réponse |
|----------|-------------|---------|
| `msz` | Taille de la carte | `msz X Y` |
| `bct X Y` | Contenu d'une case | `bct X Y q0 q1 q2 q3 q4 q5 q6` |
| `mct` | Contenu de toute la carte | Série de réponses `bct` |
| `tna` | Noms des équipes | Série de réponses `tna <nom>` |

### Commandes d'information sur les joueurs

| Commande | Description | Réponse |
|----------|-------------|---------|
| `ppo #n` | Position d'un joueur | `ppo #n X Y O` |
| `plv #n` | Niveau d'un joueur | `plv #n L` |
| `pin #n` | Inventaire d'un joueur | `pin #n X Y q0 q1 q2 q3 q4 q5 q6` |

### Commandes de contrôle du serveur

| Commande | Description | Réponse |
|----------|-------------|---------|
| `sgt` | Fréquence du serveur | `sgt f` |
| `sst f` | Modifie la fréquence | `sst f` |

### Événements envoyés automatiquement

Le serveur envoie automatiquement ces événements à l'interface graphique :

#### Événements joueurs
- `pnw #n X Y O L <team>` : Nouveau joueur
- `ppo #n X Y O` : Position d'un joueur
- `plv #n L` : Niveau d'un joueur
- `pin #n X Y q0 q1 q2 q3 q4 q5 q6` : Inventaire d'un joueur
- `pex #n` : Expulsion d'un joueur
- `pbc #n <message>` : Broadcast d'un joueur
- `pic X Y L #n #n ...` : Début d'incantation
- `pie X Y result` : Fin d'incantation
- `pfk #n` : Ponte d'un œuf
- `pdr #n i` : Un joueur lâche une ressource
- `pgt #n i` : Un joueur prend une ressource
- `pdi #n` : Mort d'un joueur

#### Événements œufs
- `enw #e #n X Y` : Ponte d'un œuf
- `ebo #e` : Éclosion d'un œuf
- `edi #e` : Mort d'un œuf

#### Événements de fin
- `seg <team>` : Fin du jeu (équipe gagnante)
- `smg <message>` : Message du serveur

### Codes de réponse

- `suc` : Commande inconnue
- `sbp` : Mauvais paramètres

## Système de ressources

### Distribution initiale

Les ressources sont distribuées aléatoirement sur la carte au démarrage selon des densités prédéfinies.

### Réapparition

Les ressources réapparaissent périodiquement sur la carte pour maintenir l'équilibre du jeu.

### Types de ressources

1. **Food** : Nécessaire pour la survie (consommée automatiquement)
2. **Pierres précieuses** : Nécessaires pour les incantations (linemate, deraumere, sibur, mendiane, phiras, thystame)

## Système de niveaux

Les joueurs commencent au niveau 1 et peuvent monter jusqu'au niveau 8. Chaque niveau nécessite :
- Un nombre spécifique de joueurs du bon niveau
- Des ressources spécifiques sur la case
- Une incantation réussie

## Gestion du temps

### Fréquence

La fréquence (`-f` paramètre) détermine la vitesse du jeu :
- Fréquence = 100 → 1 unité de temps = 1/100 seconde
- Fréquence = 2 → 1 unité de temps = 0.5 seconde

### Durée des actions

Chaque action a une durée définie en unités de temps :
- Actions rapides (Inventory) : 1 unité
- Actions normales (mouvement, look, take, set) : 7 unités
- Incantation : 300 unités
- Fork : 42 unités

### Survie

Les joueurs consomment automatiquement de la nourriture pour survivre. Sans nourriture, ils meurent.

## Codes d'erreur et réponses

### Réponses standard
- `ok` : Commande exécutée avec succès
- `ko` : Échec de l'exécution de la commande
- `dead` : Le joueur est mort

### Gestion des erreurs
- Commandes invalides → `ko`
- Paramètres manquants → `ko`
- Actions impossibles → `ko`

## Notes de développement

### Architecture modulaire
Le code est organisé en modules pour faciliter la maintenance et l'extension.

### Gestion mémoire
Le serveur gère automatiquement la mémoire et nettoie les ressources à la fermeture.

### Performance
Le serveur utilise `select()` pour gérer efficacement plusieurs connexions simultanées.

### Logs
Le serveur affiche des informations de démarrage et peut être étendu pour inclure plus de logs.
