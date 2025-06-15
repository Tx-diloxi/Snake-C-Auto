# 🐍 Snake-C-Auto – Jeu Snake automatique en C (Terminal)

**Snake-C-Auto** est une adaptation du célèbre jeu **Snake**, programmée en **C** et jouable directement dans le **terminal**.  
Le serpent se déplace **automatiquement** en utilisant des algorithmes de recherche de chemin pour attraper les pommes de manière optimale.  
Le projet est composé de plusieurs versions, chacune apportant de nouvelles fonctionnalités et défis.

---

# Détails des versions

> ### Version 1 – Plateau classique avec bordures
> <details> <summary>Détails</summary>
>
> - Le serpent évolue sur un plateau entouré de **bordures fixes**.
> - Aucun contrôle clavier nécessaire : le déplacement est entièrement automatisé.
>
> </details>

---

> ### Version 2 – Téléportation par les murs
> <details> <summary>Détails</summary>
>
> - Apparition de **passages dans les bords** permettant au serpent de **traverser** de l'autre côté du terrain.
> - Modification de l’algorithme pour gérer cette nouvelle mécanique.
>
> </details>

---

> ### Version 3 – Obstacles sur le plateau
> <details> <summary>Détails</summary>
>
> - Introduction d’**obstacles fixes** que le serpent doit éviter.
> - Adaptation de l’algorithme pour inclure ces contraintes supplémentaires.
>
> </details>

---

> ### Version 4 – Mode compétitif à deux serpents
> <details> <summary>Détails</summary>
>
> - Deux serpents se déplacent automatiquement et **se disputent les pommes**.
> - Chacun applique sa propre stratégie d’optimisation.
> - Le joueur ayant le meilleur score remporte la partie.
>
> </details>

---

## Compilation et exécution

```sh
gcc -o snake main.c
./snake
```

## Auteurs

- [Ton nom ou pseudo]

## Licence

Ce projet est distribué sous licence MIT.
