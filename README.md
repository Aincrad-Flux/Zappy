# Zappy

Zappy est un projet de simulation multi-joueurs en réseau développé en C/C++ et Python. Le jeu met en scène des équipes d'intelligences artificielles qui évoluent dans un monde 2D, collectent des ressources et tentent de progresser à travers des rituels d'élévation pour atteindre le niveau maximum.

## 🎯 Objectif du jeu

Le but de Zappy est d'être la première équipe à avoir 6 joueurs atteignant le niveau maximum (niveau 8). Pour cela, les joueurs doivent :

- **Survivre** en collectant de la nourriture
- **Rassembler des ressources** (pierres précieuses) nécessaires aux rituels
- **Collaborer** pour effectuer des incantations d'élévation
- **Se reproduire** pour augmenter les effectifs de l'équipe
- **Communiquer** entre membres de l'équipe via des messages

## 🏗️ Architecture du projet

Le projet Zappy est composé de trois composants principaux :

```
zappy/
├── SERVER/          # Serveur de jeu (C)
├── GUI/             # Interface graphique (C++/Raylib)
├── AI/              # Intelligence artificielle (Python)
├── zappy_launcher/  # Lanceur graphique (Python/Tkinter)
└── zappy_ref-v3.0.1/ # Version de référence
```

### 🖥️ Serveur (`zappy_server`)

Le serveur est le cœur du système. Il gère :
- La simulation du monde et ses ressources
- Les connexions des clients (IA et interface graphique)
- Les règles du jeu et la logique métier
- La communication entre tous les composants

**Technologies** : C, sockets TCP, architecture modulaire

### 🎮 Interface Graphique (`zappy_gui`)

L'interface graphique permet de visualiser le jeu en temps réel avec :
- Rendu 2D et 3D du monde
- Affichage des joueurs, ressources et équipes
- Panneaux d'information détaillés
- Interface utilisateur intuitive

**Technologies** : C++17, Raylib, OpenGL

### 🤖 Intelligence Artificielle (`zappy_ai`)

L'IA contrôle les joueurs automatiquement avec :
- Stratégies de survie et de collecte
- Coordination pour les rituels d'élévation
- Communication inter-équipe
- Reproduction automatique

**Technologies** : Python 3.6+, architecture modulaire

### 🚀 Lanceur (`zappy_launcher`)

Le lanceur simplifie le démarrage et la configuration avec :
- Interface graphique de configuration
- Compilation automatique
- Gestion des processus
- Logs en temps réel

**Technologies** : Python 3, Tkinter

## 📋 Prérequis

### Système
- **OS** : Linux (testé sur Ubuntu/Debian)
- **Compilateurs** : GCC, G++ (support C++17)
- **Python** : Version 3.6 ou supérieure
- **Make** : Pour la compilation

### Dépendances
- **Raylib** : Pour l'interface graphique
- **Sockets TCP** : Support réseau (inclus dans le système)

### Installation des dépendances

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential python3 python3-pip libraylib-dev
```

#### Arch Linux
```bash
sudo pacman -S base-devel python python-pip raylib
```

## 🔧 Compilation

### Compilation complète
```bash
make
```
Cette commande compile automatiquement les trois composants principaux.

### Compilation individuelle
```bash
# Serveur uniquement
make zappy_server

# Interface graphique uniquement
make zappy_gui

# IA uniquement
make zappy_ai
```

### Nettoyage
```bash
# Nettoyer les objets
make clean

# Nettoyer complètement
make fclean

# Recompiler entièrement
make re
```

## 🎮 Utilisation

### Méthode 1 : Lanceur graphique (Recommandé)

1. **Lancer le lanceur**
   ```bash
   cd zappy_launcher
   python3 zappy_launcher.py
   ```

2. **Configurer la partie**
   - Définir les paramètres du serveur (port, taille de carte, équipes)
   - Choisir le nombre d'IA par équipe
   - Sélectionner le mode d'affichage

3. **Démarrer automatiquement**
   - Le lanceur compile, démarre le serveur, l'interface et les IA

### Méthode 2 : Lancement manuel

1. **Démarrer le serveur**
   ```bash
   ./zappy_server -p 4242 -x 20 -y 20 -n team1 team2 team3 -c 6 -f 100
   ```

2. **Lancer l'interface graphique**
   ```bash
   ./zappy_gui -p 4242 -h localhost
   ```

3. **Connecter les IA**
   ```bash
   ./zappy_ai -p 4242 -n team1
   ./zappy_ai -p 4242 -n team2
   # ... autant d'IA que souhaité
   ```

## 📖 Documentation détaillée

### Paramètres du serveur

| Paramètre | Description | Exemple |
|-----------|-------------|---------|
| `-p port` | Port d'écoute | `-p 4242` |
| `-x width` | Largeur du monde | `-x 20` |
| `-y height` | Hauteur du monde | `-y 20` |
| `-n teams` | Noms des équipes | `-n team1 team2` |
| `-c clients` | Clients max/équipe | `-c 6` |
| `-f freq` | Fréquence serveur | `-f 100` |

### Commandes de l'IA

#### Mouvement
- `Forward` : Avancer
- `Right` : Tourner à droite
- `Left` : Tourner à gauche

#### Information
- `Look` : Observer l'environnement
- `Inventory` : Afficher l'inventaire
- `Connect_nbr` : Places libres équipe

#### Actions
- `Take <resource>` : Prendre ressource
- `Set <resource>` : Poser ressource
- `Eject` : Expulser joueurs
- `Broadcast <msg>` : Diffuser message
- `Incantation` : Rituel d'élévation
- `Fork` : Reproduction

### Ressources du jeu

| Ressource | Usage |
|-----------|-------|
| `food` | Survie (consommée automatiquement) |
| `linemate` | Incantations niveau 1-2 |
| `deraumere` | Incantations niveau 2-3 |
| `sibur` | Incantations niveau 3-4 |
| `mendiane` | Incantations niveau 4-5 |
| `phiras` | Incantations niveau 5-6 |
| `thystame` | Incantations niveau 6-8 |

### Conditions d'élévation

Chaque niveau nécessite un nombre spécifique de joueurs et de ressources sur la même case pour l'incantation.

## 🎯 Stratégies de jeu

### Survie
- Toujours maintenir un stock de nourriture suffisant
- La nourriture se consomme automatiquement avec le temps

### Collecte
- Prioriser les ressources selon le niveau actuel
- Optimiser les déplacements pour l'efficacité

### Coordination
- Utiliser `Broadcast` pour coordonner les équipes
- Rassembler les joueurs pour les incantations

### Reproduction
- Utiliser `Fork` pour augmenter les effectifs
- Plus de joueurs = plus de chances de réussir

## 🔧 Configuration avancée

### Personnalisation de l'IA

L'IA est modulaire et peut être étendue en modifiant :
- `ai_core.py` : Logique principale
- `network_client.py` : Communication réseau
- `terminal_ui.py` : Interface utilisateur (optionnelle)

### Interface graphique

L'interface supporte :
- Mode 2D classique (`./zappy_gui -p 4242 -2d`)
- Mode 3D par défaut (`./zappy_gui -p 4242`)
- Connexion à distance (`./zappy_gui -p 4242 -h <ip>`)

## 🐛 Dépannage

### Problèmes courants

**Compilation échoue**
```bash
# Vérifier les dépendances
sudo apt install build-essential libraylib-dev

# Nettoyer et recompiler
make fclean && make
```

**Connexion refusée**
```bash
# Vérifier que le serveur est démarré
ps aux | grep zappy_server

# Vérifier le port
netstat -tlnp | grep 4242
```

**IA ne se connecte pas**
```bash
# Vérifier le nom d'équipe
./zappy_ai -p 4242 -n team_name_exact

# Vérifier les logs serveur
```

### Logs et debug

- **Serveur** : Affiche les connexions et événements
- **IA** : Option `-v` pour mode verbeux
- **GUI** : Logs intégrés dans l'interface

## 📚 Structure des fichiers

```
zappy/
├── Makefile                 # Compilation globale
├── README.md               # Ce fichier
├── zappy_server            # Exécutable serveur
├── zappy_gui              # Exécutable interface
├── zappy_ai               # Exécutable IA
│
├── SERVER/                # Code serveur
│   ├── src/              # Sources C
│   ├── include/          # Headers
│   ├── Makefile          # Compilation serveur
│   └── README.md         # Doc serveur
│
├── GUI/                  # Interface graphique
│   ├── src/             # Sources C++
│   ├── include/         # Headers
│   ├── Makefile         # Compilation GUI
│   └── README           # Doc GUI
│
├── AI/                  # Intelligence artificielle
│   ├── *.py            # Modules Python
│   ├── Makefile        # Script de compilation
│   └── README.md       # Doc IA
│
└── zappy_launcher/     # Lanceur graphique
    └── zappy_launcher.py
```

## 🤝 Contribution

### Standards de code
- **C/C++** : Normes EPITECH, documentation des fonctions
- **Python** : PEP 8, docstrings pour les modules
- **Git** : Messages de commit descriptifs

### Tests
- Tester les trois composants ensemble
- Vérifier la compatibilité réseau
- Valider les règles du jeu


## 🎉 Crédits

Développé dans le cadre du projet Zappy à EPITECH.

---

**Pour commencer rapidement :** Utilisez le lanceur graphique avec `python3 zappy_launcher/zappy_launcher.py` !
