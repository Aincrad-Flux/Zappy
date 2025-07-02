# Changelog

Toutes les modifications notables de ce projet seront documentées dans ce fichier.

Le format est basé sur [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
et ce projet adhère à [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-07-02

### 🎉 First Release

Cette version marque la première release stable du projet Zappy, un jeu de simulation multi-joueurs en réseau.

### ✅ Features

#### 🖥️ Serveur (`zappy_server`)
- **Gestion multi-joueurs** : Support complet pour plusieurs équipes simultanées
- **Simulation du monde** : Gestion d'une carte 2D avec ressources dynamiques
- **Protocole réseau** : Communication TCP robuste avec clients IA et GUI
- **Système de règles** : Implémentation complète des règles de jeu Zappy
- **Gestion des ressources** : 7 types de ressources (nourriture + 6 pierres précieuses)
- **Système d'élévation** : Rituels d'incantation avec conditions strictes
- **Broadcasting** : Communication inter-équipes via messages directionnels
- **Reproduction** : Système de fork pour augmenter les effectifs

#### 🎮 Interface Graphique (`zappy_gui`)
- **Rendu 2D/3D** : Visualisation complète du monde de jeu avec Raylib
- **Interface temps réel** : Mise à jour en direct de l'état du jeu
- **Panneaux d'information** : Détails sur joueurs, équipes, ressources
- **Contrôles intuitifs** : Navigation fluide dans l'environnement 3D
- **Affichage des stats** : Niveau, inventaire et statut des joueurs
- **Vue d'ensemble** : Carte miniature et informations globales

#### 🤖 Intelligence Artificielle (`zappy_ai`)
- **Stratégies avancées** : IA capable de survivre et évoluer
- **Coordination d'équipe** : Communication et collaboration entre bots
- **Gestion des ressources** : Collecte optimisée et partage intelligent
- **Rituels d'élévation** : Organisation automatique des incantations
- **Reproduction stratégique** : Augmentation des effectifs au bon moment
- **Communication cryptée** : Système de hashage pour les messages d'équipe

#### 🚀 Lanceur (`zappy_launcher`)
- **Interface graphique** : Configuration simple via Tkinter
- **Compilation automatique** : Build automatique de tous les composants
- **Gestion des processus** : Lancement et supervision des programmes
- **Logs en temps réel** : Suivi des activités de tous les composants
- **Configuration flexible** : Paramétrage avancé du serveur et des clients

### 🛠️ Technical Stack

- **Server** : C avec architecture modulaire et sockets TCP
- **GUI** : C++17 avec Raylib pour le rendu graphique
- **AI** : Python 3.6+ avec logging avancé et architecture OOP
- **Launcher** : Python 3 avec Tkinter pour l'interface utilisateur

### 📦 Components Architecture

```
zappy/
├── SERVER/          # Serveur de jeu (C)
│   ├── src/         # Code source
│   ├── include/     # Headers
│   └── Makefile     # Compilation
├── GUI/             # Interface graphique (C++/Raylib)
│   ├── src/         # Code source
│   ├── include/     # Headers
│   └── Makefile     # Compilation
├── AI/              # Intelligence artificielle (Python)
│   ├── ai_core.py   # Logique principale
│   ├── network_client.py # Communication réseau
│   └── terminal_ui.py    # Interface debug
├── zappy_launcher/  # Lanceur graphique
│   └── zappy_launcher.py # Interface de configuration
└── Makefile         # Compilation globale
```

### 🎯 Game Rules Implemented

- **Objectif** : Première équipe avec 6 joueurs au niveau 8 gagne
- **Survie** : Gestion de la nourriture (126 unités de temps par aliment)
- **Niveaux** : 8 niveaux d'élévation avec conditions spécifiques
- **Ressources** : 7 types (linemate, deraumere, sibur, mendiane, phiras, thystame, nourriture)
- **Communication** : Broadcasting directionnel entre joueurs
- **Reproduction** : Fork avec œufs et éclosion automatique

### 🚀 Getting Started

1. **Compilation** :
   ```bash
   make
   ```

2. **Lancement du serveur** :
   ```bash
   ./zappy_server -p 4242 -x 10 -y 10 -n team1 team2 -c 5 -f 100
   ```

3. **Lancement de l'IA** :
   ```bash
   ./zappy_ai -p 4242 -n team1 -h localhost
   ```

4. **Lancement de la GUI** :
   ```bash
   ./zappy_gui -p 4242 -h localhost
   ```

5. **Ou utiliser le lanceur** :
   ```bash
   python3 zappy_launcher/zappy_launcher.py
   ```

### 📋 System Requirements

- **OS** : Linux (testé sur Ubuntu/Debian)
- **Compilateurs** : GCC, G++ (support C++17)
- **Python** : Version 3.6 ou supérieure
- **Dépendances** : Raylib, Make
- **Réseau** : Support sockets TCP

### 🔧 Installation

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential python3 python3-pip libraylib-dev

# Compilation
make

# Test
make test
```

### 📊 Performance & Stability

- **Serveur** : Gestion stable de 10+ clients simultanés
- **IA** : Algorithmes optimisés pour la survie long terme
- **GUI** : Rendu fluide à 60 FPS même avec beaucoup de joueurs 
- **Réseau** : Communication robuste avec gestion des déconnexions

### 🧪 Testing

- Tests unitaires pour les composants critiques
- Tests d'intégration serveur-client
- Tests de performance sous charge
- Tests de stabilité long terme

### 📝 Documentation

- README complet avec instructions détaillées
- Commentaires de code en français/anglais
- Architecture documentée
- Protocoles de communication spécifiés

---

**Date de release** : 2 juillet 2025  
**Contributeurs** : Équipe Epitech B-YEP-400  
**Version** : 1.0.0 (Première release stable)
