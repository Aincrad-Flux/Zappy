# Stratégie améliorée de l'IA Zappy

Ce document explique la stratégie améliorée implémentée dans le code de l'IA pour le jeu Zappy.

## Objectif de la stratégie

La nouvelle stratégie est axée sur deux objectifs principaux :
1. Collecter un maximum de nourriture pour chaque IA
2. Rassembler toutes les ressources nécessaires pour enchaîner les élévations et atteindre le niveau 8 d'un coup

## Organisation de la collecte

La stratégie utilise une approche hybride pour la collecte des ressources :

1. **Collecte décentralisée avec coordination** :
   - Chaque IA collecte des ressources individuellement
   - Toutes les IAs partagent les informations sur leurs inventaires via des messages diffusés
   - L'IA maintient un inventaire virtuel total de l'équipe (`team_backpack["total"]`)

2. **Priorisation des ressources** :
   - Toutes les IAs ciblent en priorité les ressources manquantes au niveau global de l'équipe
   - Les IAs avec des niveaux plus élevés ou désignées comme leaders se concentrent sur les ressources rares

3. **Centralisation pour l'élévation** :
   - Lors de la phase de préparation à l'élévation, les ressources sont placées sur une même tuile
   - Le leader du rituel coordonne le placement des ressources pour l'incantation

## Paramètres de la stratégie

La stratégie utilise plusieurs seuils configurables :

- `FOOD_MIN_THRESHOLD = 30` : Priorité absolue sur la nourriture en dessous de ce seuil
- `FOOD_COMFORT_THRESHOLD = 60` : Niveau confortable pour se concentrer sur les ressources
- `FOOD_ELEVATION_THRESHOLD = 100` : Réserve nécessaire avant d'envisager l'élévation
- `RESOURCE_MULTIPLIER = 1.5` : Multiplicateur pour assurer des quantités suffisantes de ressources

## Phases de la stratégie

L'IA fonctionne selon trois phases distinctes :

1. **FOOD_GATHERING** : Lorsque les réserves de nourriture sont faibles (<30), l'IA se concentre exclusivement sur la recherche de nourriture.

2. **RESOURCE_GATHERING** : Avec un niveau de nourriture confortable (>60), l'IA collecte stratégiquement les ressources nécessaires pour l'élévation, en priorisant celles dont elle a le plus besoin.

3. **ELEVATION_PREPARATION** : Lorsque suffisamment de ressources sont collectées et que les réserves de nourriture sont élevées (>100), l'IA se prépare à l'élévation et coordonne avec les autres joueurs.

## Améliorations de l'algorithme

### 1. Calcul des besoins en ressources
L'IA calcule la quantité totale de ressources nécessaires pour atteindre le niveau 8, avec une marge de sécurité de 50% pour garantir une progression rapide.

```python
def calculate_total_resources_needed(self):
    """
    Calcule les ressources totales nécessaires pour tous les niveaux jusqu'à 8.
    Applique un multiplicateur pour assurer que nous collectons plus que le minimum requis.
    """
    total_resources = {"linemate": 0, "deraumere": 0, "sibur": 0,
                     "mendiane": 0, "phiras": 0, "thystame": 0}

    for level, requirements in LEVEL_REQUIREMENTS.items():
        for resource, amount in requirements.items():
            total_resources[resource] += int(amount * RESOURCE_MULTIPLIER)

    return total_resources
```

### 2. Priorisation des ressources
Au lieu de chercher aléatoirement les ressources manquantes pour le niveau actuel, l'IA priorise les ressources en fonction de l'écart entre les quantités actuelles et les objectifs totaux.

```python
def find_priority_resource(self):
    # Logique pour déterminer quelle ressource prioriser
```

### 3. Analyse de vision améliorée
L'IA utilise une analyse de vision plus sophistiquée qui peut identifier les ressources alternatives si la cible n'est pas visible, optimisant ainsi les mouvements.

```python
def analyze_vision_enhanced(self, data: str, resource: str):
    # Analyse améliorée du champ de vision
```

### 4. Coordination d'équipe
L'IA diffuse régulièrement son inventaire pour maintenir une vision globale des ressources de l'équipe, ce qui permet de mieux répartir les efforts de collecte.

## Avantages de cette stratégie

- **Efficacité énergétique** : Maintien de niveaux élevés de nourriture pour éviter la famine
- **Collecte optimisée** : Concentration sur les ressources réellement nécessaires
- **Progression rapide** : Accumulation des ressources pour enchaîner les élévations
- **Résilience** : Adaptation aux conditions variables du jeu et aux disponibilités de ressources

## Comment l'utiliser

La stratégie est intégrée directement dans la classe `AICore` et s'active automatiquement lors de l'initialisation sans nécessiter de configuration supplémentaire.
