# Rapport SAÉ 2048

## 1. Difficultés
- **Les pointeurs et la mémoire** : Passer d'une simple variable à un tableau dynamique (avec malloc) et utiliser la mémoire partagée (shmget/shmat) nous a causé pleins d'erreurs "Segmentation fault (core dumped)". On a mis du temps à comprendre comment bien relier les parties aux joueurs.
- **Affichage multi-joueurs** : La grille de tous les joueurs s'affichait dans le terminal du serveur. On a réglé ça en récupérant le chemin du terminal du joueur avec ttyname() et en passant ce chemin au processus d'affichage, qui redirige sa sortie avec freopen().
- **La synchronisation** : On a eu du mal à comprendre où mettre exactement les mutex et les sémaphores pour éviter que le programme ne se bloque complètement quand les threads tournent en même temps.
- **Erreurs de lecture interrompue** : Les signaux interrompaient parfois le read() du pipe et faisaient planter le serveur. On a géré ça avec un test if (errno == EINTR) continue;.

## 2. Répartition des tâches

- **Alexis HELLICH** : Architecture globale (processus, fork, pipes, signaux)
- **Emre SEN** : Algorithme de jeu (mouvements, fusions, score, détection victoire/défaite)
- **Fabio GIORGI** : Affichage console et synchronisation des threads

## 3. Fonctionnalités

- Multi-joueurs : N parties en parallèle, N choisi au lancement (./game N)
- Un seul pipe nommé pour tous les joueurs, chaque joueur est identifié par son PID
- Mémoire partagée (shmget/shmat) protégée par mutex et sémaphores
- Chaque joueur voit sa propre grille dans son propre terminal
- Connexion refusée si le serveur est plein (message d'erreur dans le terminal du joueur)

## 4. Ce qui ne marche pas

- Quand la partie est finie (victoire ou défaite), le client ./main ne le sait pas et reste bloqué à attendre une touche. Il faut taper a pour quitter.

## 5. Comment jouer

1. Compiler : make
2. Lancer le serveur dans un terminal : ./game N (remplacer N par le nombre de joueurs max)
3. Chaque joueur ouvre un autre terminal et tape : ./main
4. Appuyer sur Entrée pour rejoindre la partie
5. Touches : z (Haut), s (Bas), q (Gauche), d (Droite), a (Abandonner)
