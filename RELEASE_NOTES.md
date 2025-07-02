# 🎉 Zappy v1.0.0 - First Release

**Date de release** : 2 juillet 2025

## 🚀 Qu'est-ce que Zappy ?

Zappy est un projet de simulation multi-joueurs en réseau où des équipes d'intelligences artificielles s'affrontent dans un monde 2D. Le but est d'être la première équipe à avoir 6 joueurs atteignant le niveau maximum (niveau 8).

## ✨ Nouveautés de cette release

### 🎮 Expérience de jeu complète
- **Simulation multi-joueurs** temps réel avec gestion d'équipes
- **Monde dynamique** avec 7 types de ressources
- **Système d'élévation** avec rituels d'incantation
- **Communication d'équipe** avec broadcasting directionnel
- **Reproduction stratégique** pour augmenter les effectifs

### 🛠️ Stack technique robuste
- **Serveur** en C avec sockets TCP pour la performance
- **Interface graphique** en C++17 avec Raylib pour le rendu 3D
- **IA** en Python avec algorithmes avancés
- **Lanceur** graphique pour une utilisation simplifiée

## 📦 Composants inclus

| Composant | Description | Langage |
|-----------|-------------|---------|
| `zappy_server` | Serveur de simulation | C |
| `zappy_gui` | Interface graphique 3D | C++17 |
| `zappy_ai` | Intelligence artificielle | Python 3.6+ |
| `zappy_launcher` | Lanceur graphique | Python + Tkinter |

## 🚀 Installation rapide

### Prérequis
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential python3 python3-pip libraylib-dev

# Arch Linux  
sudo pacman -S base-devel python python-pip raylib
```

### Compilation
```bash
# Clone le repository
git clone <repository-url>
cd B-YEP-400-LIL-4-1-zappy-thibault.pouch

# Compilation complète
make

# Ou compilation individuelle
make server    # Serveur uniquement
make gui       # Interface graphique uniquement  
make ai        # IA uniquement
```

## 🎯 Utilisation

### Méthode 1 : Lanceur graphique (Recommandé)
```bash
python3 zappy_launcher/zappy_launcher.py
```

### Méthode 2 : Lancement manuel

1. **Démarrer le serveur** :
```bash
./zappy_server -p 4242 -x 10 -y 10 -n team1 team2 -c 5 -f 100
```

2. **Lancer l'interface graphique** :
```bash
./zappy_gui -p 4242 -h localhost
```

3. **Démarrer les IA** :
```bash
./zappy_ai -p 4242 -n team1 -h localhost  # Équipe 1
./zappy_ai -p 4242 -n team2 -h localhost  # Équipe 2
```

## 🎮 Règles du jeu

### Objectif
Première équipe à avoir **6 joueurs au niveau 8** gagne !

### Survie
- Chaque joueur a besoin de **nourriture** pour survivre
- 1 nourriture = 126 unités de temps
- Mort par famine si plus de nourriture

### Élévation
8 niveaux d'élévation avec des conditions spécifiques :
- **Niveau 1→2** : 1 joueur, 1 linemate
- **Niveau 2→3** : 1 joueur, 1 linemate, 1 deraumere, 1 sibur
- **Niveau 3→4** : 2 joueurs, 2 linemate, 1 sibur, 1 phiras
- Et ainsi de suite...

### Ressources
7 types de ressources dans le monde :
- **Nourriture** : Survie
- **Linemate, Deraumere, Sibur, Mendiane, Phiras, Thystame** : Élévation

## 🔧 Configuration avancée

### Paramètres du serveur
```bash
./zappy_server [OPTIONS]
  -p port          # Port d'écoute
  -x width         # Largeur de la carte
  -y height        # Hauteur de la carte  
  -n team1 team2   # Noms des équipes
  -c clients       # Nombre de clients max par équipe
  -f freq          # Fréquence des actions (1/seconde)
```

### Paramètres de l'IA
```bash
./zappy_ai [OPTIONS]
  -p port          # Port du serveur
  -n team          # Nom de l'équipe
  -h host          # Adresse du serveur
```

## 🐛 Problèmes connus

- Sur certains systèmes, Raylib peut nécessiter une installation manuelle
- L'IA peut parfois avoir des comportements non optimaux dans les coins de carte
- La GUI peut ralentir avec plus de 20 joueurs simultanés

## 🤝 Contribution

Ce projet a été développé dans le cadre du cursus Epitech. 

### Équipe de développement
- Développement serveur (C)
- Développement interface graphique (C++/Raylib)  
- Développement IA (Python)
- Intégration et tests

## 📄 Licence

Ce projet est développé dans le cadre éducatif d'Epitech.

## 🔗 Liens utiles

- [Documentation complète](./README.md)
- [Changelog détaillé](./CHANGELOG.md)
- [Issues GitHub](../../issues)

---

**Téléchargement** : [Release v1.0.0](../../releases/tag/v1.0.0)  
**Support** : [Issues GitHub](../../issues)  
**Documentation** : [README.md](./README.md)

Bonne partie ! 🎮✨
