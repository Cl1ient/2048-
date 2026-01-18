# Rapport SAÉ 2048


## 1. Difficultés
- **Algorithme de fusion** : Au début, on voulait faire un gros switch case avec 4 fonctions différentes pour chaque direction pour la fusion. Mais on n'était pas trop d'accord car ça faisait beaucoup de répétition de code. Au final, on a trouvé comment faire avec une seule fonction process_line qui marche pour toute les directions en changeant juste l'ordre des cases avant de l'appeler
- **Synchronisation** : On a eu quelques problèmes pour synchroniser les threads et les processus avec les signaux SIGUSR1 ça bloquait l'affichage parfois
- **Appels système** : On a dû gérer les erreurs EINTR car les signaux interrompaient parfois la lecture des pipes


## 2. Répartition des tâches
- **Alexis HELLICH** : Structure système (processus, pipes, fork) et de l'architecture globale du projet
- **Emre SEN** : Algorithme (mouvements, fusions, score)
- **Fabio GIORGI** : Affichage console et aide sur la synchronisation des threads


## 3. Fonctionnalités
Tout est implémenté (normalement)


## 4. Comment jouer
1. Taper `make` dans le terminal pour compiler.
2. Lancer `./main` dans un terminal pour les commandes.
3. Lancer `./game` dans un autre terminal pour l'affichage.
   - Touches : `z` (Haut), `s` (Bas), `q` (Gauche), `d` (Droite).
   - Quitter : `a` (Abandon).
