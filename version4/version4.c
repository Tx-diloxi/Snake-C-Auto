/**
* @file version4.c
* @brief Jeu Snake version 3 avec obstacles pour le projet SAE 1.02.
*
* Ce programme implémente une version du jeu Snake avec des obstacles. 
* Le serpent se déplace automatiquement vers des pommes tout en évitant les pavés et les bords du plateau.
* Des portails permettent au serpent de traverser les bords du plateau pour réapparaître de l'autre côté.
*
* @author
* LE SECH Marceau
* CAMARA Kemo
* @version 3.0
* @date 08/12/2024
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

// Constantes du jeu
#define TAILLE 10  // Taille du serpent.
#define LARGEUR_PLATEAU 80  // Largeur du plateau de jeu.
#define HAUTEUR_PLATEAU 40  // Hauteur du plateau de jeu.
#define X_DEPART_SERPENT 40  // Position en X du serpent 1 au départ (au centre du plateau).
#define Y_DEPART_SERPENT 13  // Position en Y du serpent 1 au départ (au centre du plateau).
#define X_DEPART_SERPENT_2 40  // Position en X du serpent 2 au départ (au centre du plateau).
#define Y_DEPART_SERPENT_2 27  // Position en Y du serpent 2 au départ (au centre du plateau).
#define NB_POMMES 10  // Nombre de pommes présentes sur le plateau.
#define ATTENTE 20000  // Temps d'attente entre chaque déplacement du serpent (en microsecondes).
#define CORPS 'X'  // Caractère utilisé pour dessiner le corps du serpent.
#define TETE '1'  // Caractère utilisé pour dessiner la tête du serpent.
#define TETE2 '2'  // Caractère utilisé pour dessiner la tête du serpent.
#define BORDURE '#'  // Caractère utilisé pour dessiner les bords du plateau.
#define STOP 'a'  // Touche pour arrêter le jeu.
#define VIDE ' '  // Caractère pour un espace vide sur le plateau.
#define POMME '6'  // Caractère pour une pomme.
#define HAUT 'z'  // Touche pour diriger le serpent vers le haut.
#define BAS 's'  // Touche pour diriger le serpent vers le bas.
#define GAUCHE 'q'  // Touche pour diriger le serpent vers la gauche.
#define DROITE 'd'  // Touche pour diriger le serpent vers la droite.
#define PAVE '#' // Représente un pavé d'obstacle
#define NB_PAVES 6 // Nombre de pavés d'obstacles
#define TAILLE_PAVE 5 // Dimension (carrée) des pavés

// Positions des pommes et des pavés
int lesPommesX[NB_POMMES] = {40, 75, 78, 2, 9, 78, 74, 2, 72, 5}; // Positions en X des pommes.
int lesPommesY[NB_POMMES] = {20, 38, 2, 2, 5, 38, 32, 38, 32, 2}; // Positions en Y des pommes.
int lesPavesX[NB_PAVES] = { 4, 73, 4, 73, 38, 38}; // Positions en X des pavés.
int lesPavesY[NB_PAVES] = { 4, 4, 33, 33, 14, 22}; // Positions en Y des pavés.

typedef char tPlateau[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1]; // Initialiser le plateau de jeu.

typedef int tChemins[5]; // Initialiser le tableau avec les 5 chemins possibles.

int NbPommesSerpentManger = 0; // Compteur du nombre de déplacements effectués par le serpent.
int NbPommesSerpentManger2 = 0; 
int nbDepUnitaires = 0; // Compteur du nombre de pommes mangées par le serpent.
int nbDepUnitaires2 = 0;

typedef struct {
    int x; // Coordonnée X du Portail.
    int y; // Coordonnée X du Portail.
} Portail;

const Portail TROU_HAUT = {40, 0};  // Portail en haut du plateau (Portail de coordonnées (40, 0)).
const Portail TROU_BAS = {40, 40};  // Portail en bas du plateau (Portail de coordonnées (40, 40)).
const Portail TROU_GAUCHE = {0, 20};  // Portail à gauche du plateau (Portail de coordonnées (0, 20)).
const Portail TROU_DROITE = {80, 20};  // Portail à droite du plateau (Portail de coordonnées (80, 20)).

// Prototypes des fonctions
void initPlateau(tPlateau plateau); // Initialise le plateau avec des bordures et des espaces vides.
void dessinerPlateau(tPlateau plateau); // Affiche le plateau à l'écran.
void ajouterPomme(tPlateau plateau, int Pomme);
void placerPaves(tPlateau plateau); // Ajoute les pavés à une position définie.
void afficher(int x, int y, char car); // Affiche un caractère à une position donnée.
void effacer(int x, int y); // Efface un caractère à une position donnée.
void dessinerSerpent(int lesX[], int lesY[]); // Dessine le serpent entier sur le plateau.
void dessinerSerpent2(int lesX2[], int lesY2[]); // Dessine le serpent entier sur le plateau.
void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool *collision, bool *pomme, int autreX[], int autreY[]);
void progresser2(int lesX2[], int lesY2[], char direction2, tPlateau plateau, bool *collision, bool *pomme, int autreX[], int autreY[]);
void gotoxy(int x, int y); // Déplace le curseur à une position spécifique dans le terminal.
int kbhit(void); // Vérifie si une touche a été pressée.
void disable_echo(void); // Désactive l'écho des touches dans le terminal.
void enable_echo(void); // Réactive l'écho des touches dans le terminal.
bool PasserPortails(int lesX[], int lesY[]); // Gère la traversée des bords du plateau via les portails.
int minimunTableau(tChemins Tableau); // Retourne l'index de la plus petite distance dans un tableau de distances.
bool estSurCorpsSerpent(int x, int y, int lesX[], int lesY[]); // Vérifie si une position est occupée par le corps du serpent.
bool estSurPave(int x, int y, tPlateau plateau); // Vérifie si une position est occupée par un pavé.
bool directionEstSure(int x, int y, char direction, int lesX[], int lesY[], int autreX[], int autreY[], tPlateau plateau) ;
char trouverDirectionSure(int lesX[], int lesY[], char directionActuelle, tPlateau plateau, int autreX[], int autreY[]);
bool estSurCorpsAutreSerpent(int x, int y, int autreX[], int autreY[]);

int main() {
    // 2 tableaux contenant les positions des éléments qui constituent le serpent
    int lesX[TAILLE];  // Tableau pour les positions en X des différentes parties du serpent.
    int lesY[TAILLE];  // Tableau pour les positions en Y des différentes parties du serpent.
    
    int lesX2[TAILLE];  // Tableau pour les positions en X des différentes parties du serpent.
    int lesY2[TAILLE];  // Tableau pour les positions en Y des différentes parties du serpent.
    
    // Représente la touche frappée par l'utilisateur : touche de direction ou pour l'arrêt
    char touche;

    // Direction courante du serpent (HAUT, BAS, GAUCHE ou DROITE)
    char direction;
    char direction2;

    // Le plateau de jeu
    tPlateau lePlateau;

    bool collision = false;  // Variable pour détecter si le serpent a heurter avec quelque chose (mur ou lui-même).
    bool gagne = false;  // Indicateur de victoire (si toutes les pommes sont mangées).
    bool pommeMangee = false;  // Indicateur pour savoir si une pomme a été mangée pendant le tour.
    bool pommeMangee2 = false;  // Indicateur pour savoir si une pomme a été mangée pendant le tour.

    // Initialisation de la position du serpent : positionnement de la tête en (X_DEPART_SERPENT, Y_DEPART_SERPENT), puis des anneaux à sa gauche.
    for (int i = 0; i < TAILLE; i++) {
        lesX[i] = X_DEPART_SERPENT - i;  // Position X de chaque partie du serpent.
        lesY[i] = Y_DEPART_SERPENT;  // Position Y de chaque partie du serpent.
    }

    for (int i = 0; i < TAILLE; i++) {
        lesX2[i] = X_DEPART_SERPENT_2 + i;  // Position X de chaque partie du serpent.
        lesY2[i] = Y_DEPART_SERPENT_2;  // Position Y de chaque partie du serpent.
    }

    // Mise en place du plateau
    initPlateau(lePlateau);  // Initialisation du plateau de jeu.
    system("clear");  // Effacement de l'écran .
    ajouterPomme(lePlateau, (NbPommesSerpentManger + NbPommesSerpentManger2));  // Ajoute une pomme sur le plateau.
    dessinerPlateau(lePlateau);  // Dessine le plateau à l'écran.

    // Initialisation : le serpent se dirige vers la droite
    dessinerSerpent(lesX, lesY);  // Dessine le serpent au début.
    dessinerSerpent2(lesX2, lesY2);  // Dessine le serpent au début.
    disable_echo();  // Désactive l'affichage des touches.
    direction = DROITE;  // Initialisation de la direction du serpent vers la droite.
    direction2 = GAUCHE;
    
    // Boucle de jeu. Le jeu continue tant que l'utilisateur n'appuie pas sur la touche STOP ou qu'il n'y a pas de collision ou que toutes les pommes ne sont pas mangées.
    do {
		int CheminDirectPomme = abs(lesX[0] - lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)]) + abs(lesY[0] - lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)]); // Calcul de la distance directe entre la tête du serpent et la pomme

        // Calcul des distances en passant par différents portails (haut, bas, gauche, droite)
        // Chaque chemin nécessite de passer par un portail et d'en sortir de l'autre côté avant d'atteindre la pomme.
        int CheminPortailHaut = abs(lesX[0] - TROU_HAUT.x) + abs(lesY[0] - TROU_HAUT.y) + abs(lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - TROU_BAS.x) + abs(lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - TROU_BAS.y);
        int CheminPortailBas = abs(lesX[0] - TROU_BAS.x) + abs(lesY[0] - TROU_BAS.y) + abs(lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - TROU_HAUT.x) + abs(lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - TROU_HAUT.y);
        int CheminPortailGauche = abs(lesX[0] - TROU_GAUCHE.x) + abs(lesY[0] - TROU_GAUCHE.y) + abs(lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - TROU_DROITE.x) + abs(lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - TROU_DROITE.y);
        int CheminPortailDroite = abs(lesX[0] - TROU_DROITE.x) + abs(lesY[0] - TROU_DROITE.y) + abs(lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - TROU_GAUCHE.x) + abs(lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - TROU_GAUCHE.y);


        int CheminDirectPomme2 = abs(lesX2[0] - lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)]) + abs(lesY2[0] - lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)]); // Calcul de la distance directe entre la tête du serpent et la pomme
        int CheminPortailHaut2 = abs(lesX2[0] - TROU_HAUT.x) + abs(lesY2[0] - TROU_HAUT.y) + abs(lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - TROU_BAS.x) + abs(lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - TROU_BAS.y);
        int CheminPortailBas2 = abs(lesX2[0] - TROU_BAS.x) + abs(lesY2[0] - TROU_BAS.y) + abs(lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - TROU_HAUT.x) + abs(lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - TROU_HAUT.y);
        int CheminPortailGauche2 = abs(lesX2[0] - TROU_GAUCHE.x) + abs(lesY2[0] - TROU_GAUCHE.y) + abs(lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - TROU_DROITE.x) + abs(lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - TROU_DROITE.y);
        int CheminPortailDroite2 = abs(lesX2[0] - TROU_DROITE.x) + abs(lesY2[0] - TROU_DROITE.y) + abs(lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - TROU_GAUCHE.x) + abs(lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - TROU_GAUCHE.y);



        // Tableau contenant les distances calculées pour chaque chemin
        tChemins Tableau = {CheminDirectPomme, CheminPortailHaut, CheminPortailBas, CheminPortailGauche, CheminPortailDroite};
        tChemins Tableau2 = {CheminDirectPomme2, CheminPortailHaut2, CheminPortailBas2, CheminPortailGauche2, CheminPortailDroite2};
        
        // Affichage pour moi, pour comprendre comment les distances fonctionnent.
        gotoxy(2+LARGEUR_PLATEAU, 1);
        printf("Distance Pomme : %4d pas", CheminDirectPomme);
        gotoxy(2+LARGEUR_PLATEAU, 2);
        printf("Distance Portail Haut + Pomme : %4d pas", CheminPortailHaut);
        gotoxy(2+LARGEUR_PLATEAU, 3);
        printf("Distance Portail Bas + Pomme : %4d pas", CheminPortailBas);
        gotoxy(2+LARGEUR_PLATEAU, 4);
        printf("Distance Portail Gauche + Pomme : %4d pas", CheminPortailGauche);
        gotoxy(2+LARGEUR_PLATEAU, 5);
        printf("Distance Portail Droit + Pomme : %4d pas", CheminPortailDroite);

        gotoxy(2+LARGEUR_PLATEAU, 7);
        printf("Distance 2 Pomme : %4d pas", CheminDirectPomme2);
        gotoxy(2+LARGEUR_PLATEAU, 8);
        printf("Distance 2 Portail Haut + Pomme : %4d pas", CheminPortailHaut2);
        gotoxy(2+LARGEUR_PLATEAU, 9);
        printf("Distance 2 Portail Bas + Pomme : %4d pas", CheminPortailBas2);
        gotoxy(2+LARGEUR_PLATEAU, 10);
        printf("Distance 2 Portail Gauche + Pomme : %4d pas", CheminPortailGauche2);
        gotoxy(2+LARGEUR_PLATEAU, 11);
        printf("Distance 2 Portail Droit + Pomme : %4d pas", CheminPortailDroite2);

        int minimunDes5distances = minimunTableau(Tableau); // Calcule la minimun des distances pour atteindre la pomme.
        int minimunDes5distances2 = minimunTableau(Tableau2); // Calcule la minimun des distances pour atteindre la pomme.



        switch (minimunDes5distances)
        {
            case 0:
                // Cas 0 : Chemin direct vers la pomme
                // Si la pomme est plus proche sans utiliser de portail, on va vers la pomme.
                if ((lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesY[0]) < 0) 
                {
                    direction = HAUT; // La pomme est située au-dessus de la tête du serpent.
                } 
                else if ((lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesY[0]) > 0) 
                {
                    direction = BAS; // La pomme est située en-dessous de la tête du serpent.
                } 
                else if ((lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesX[0]) < 0) 
                {
                    direction = GAUCHE; // La pomme est à gauche de la tête du serpent.
                } 
                else 
                {
                    direction = DROITE; // La pomme est à droite de la tête du serpent.
                }
                break;

            case 1:
                // Cas 1 : Chemin via le portail haut
                if (PasserPortails(lesX, lesY) == false)
                {
                    // Si le serpent n'est pas encore au portail haut, il se dirige vers le portail.
                    if ((TROU_HAUT.y - lesY[0]) < 0) 
                    {
                        direction = HAUT; // Le portail haut est au-dessus de la tête du serpent.
                    } 
                    else if ((TROU_HAUT.y - lesY[0]) > 0)
                    {
                        direction = BAS; // Le portail haut est en-dessous de la tête.
                    }
                    else if ((TROU_HAUT.x - lesX[0]) < 0) 
                    {
                        direction = GAUCHE; // Le portail haut est à gauche de la tête.
                    } else 
                    {
                        direction = DROITE; // Le portail haut est à droite de la tête.
                    }
                } else {
                    // Une fois passé par le portail, on calcule la direction directe vers la pomme.
                    if ((lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesY[0]) < 0) 
                    {
                        direction = HAUT; // La pomme est au-dessus après téléportation.
                    } 
                    else if ((lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesY[0]) > 0) 
                    {
                        direction = BAS; // La pomme est en-dessous après téléportation.
                    } 
                    else if ((lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesX[0]) < 0) 
                    {
                        direction = GAUCHE; // La pomme est à gauche après téléportation.
                    } 
                    else 
                    {
                        direction = DROITE; // La pomme est à droite après téléportation.
                    }
                }
                break;

            case 2:
                // Cas 2 : Chemin via le portail bas
                if (PasserPortails(lesX, lesY) == false) 
                {
                    // Si le serpent n'est pas encore au portail bas, il se dirige vers le portail.
                    if ((TROU_BAS.y - lesY[0]) < 0) 
                    {
                        direction = HAUT; // Le portail bas est au-dessus de la tête du serpent.
                    } 
                    else if ((TROU_BAS.y - lesY[0]) > 0) 
                    {
                        direction = BAS; // Le portail bas est en-dessous de la tête.
                    } 
                    else if ((TROU_BAS.x - lesX[0]) < 0) 
                    {
                        direction = GAUCHE; // Le portail bas est à gauche de la tête.
                    } 
                    else 
                    {
                        direction = DROITE; // Le portail bas est à droite de la tête.
                    }
                } else {
                    // Une fois passé par le portail, on calcule la direction directe vers la pomme.
                    if ((lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesY[0]) < 0)
                    {
                        direction = HAUT; // La pomme est au-dessus après téléportation.
                    } 
                    else if ((lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesY[0]) > 0) 
                    {
                        direction = BAS; // La pomme est en-dessous après téléportation.
                    } 
                    else if ((lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesX[0]) < 0) 
                    {
                        direction = GAUCHE; // La pomme est à gauche après téléportation.
                    } 
                    else 
                    {
                        direction = DROITE; // La pomme est à droite après téléportation.
                    }
                }
                break;

            case 3:
                // Cas 3 : Chemin via le portail gauche
                if (PasserPortails(lesX, lesY) == false) 
                {
                    // Si le serpent n'est pas encore au portail gauche, il se dirige vers le portail.
                    if ((TROU_GAUCHE.y - lesY[0]) < 0) 
                    {
                        direction = HAUT; // Le portail gauche est au-dessus de la tête du serpent.
                    } 
                    else if ((TROU_GAUCHE.y - lesY[0]) > 0) 
                    {
                        direction = BAS; // Le portail gauche est en-dessous de la tête.
                    } 
                    else if ((TROU_GAUCHE.x - lesX[0]) < 0) 
                    {
                        direction = GAUCHE; // Le portail gauche est à gauche de la tête.
                    } 
                    else 
                    {
                        direction = DROITE; // Le portail gauche est à droite de la tête.
                    }
                } 
                else 
                {
                    // Une fois passé par le portail, on calcule la direction directe vers la pomme.
                    if ((lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesY[0]) < 0) 
                    {
                        direction = HAUT; // La pomme est au-dessus après téléportation.
                    } 
                    else if ((lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesY[0]) > 0) 
                    {
                        direction = BAS; // La pomme est en-dessous après téléportation.
                    } 
                    else if ((lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesX[0]) < 0) 
                    {
                        direction = GAUCHE; // La pomme est à gauche après téléportation.
                    } 
                    else 
                    {
                        direction = DROITE; // La pomme est à droite après téléportation.
                    }
                }
                break;

            case 4:
                // Cas 4 : Chemin via le portail droit
                if (PasserPortails(lesX, lesY) == false)
                {
                    // Si le serpent n'est pas encore au portail droit, il se dirige vers le portail.
                    if ((TROU_DROITE.y - lesY[0]) < 0) 
                    {
                        direction = HAUT; // Le portail droit est au-dessus de la tête du serpent.
                    } 
                    else if ((TROU_DROITE.y - lesY[0]) > 0) 
                    {
                        direction = BAS; // Le portail droit est en-dessous de la tête.
                    } 
                    else if ((TROU_DROITE.x - lesX[0]) < 0) 
                    {
                        direction = GAUCHE; // Le portail droit est à gauche de la tête.
                    } 
                    else
                    {
                        direction = DROITE; // Le portail droit est à droite de la tête.
                    }
                } 
                else 
                {
                    // Une fois passé par le portail, on calcule la direction directe vers la pomme.
                    if ((lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesY[0]) < 0) 
                    {
                        direction = HAUT; // La pomme est au-dessus après téléportation.
                    } 
                    else if ((lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesY[0]) > 0) 
                    {
                        direction = BAS; // La pomme est en-dessous après téléportation.
                    } 
                    else if ((lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesX[0]) < 0) 
                    {
                        direction = GAUCHE; // La pomme est à gauche après téléportation.
                    } 
                    else 
                    {
                        direction = DROITE; // La pomme est à droite après téléportation.
                    }
                }
                break;
        }


        switch (minimunDes5distances2)
        {
            case 0:
                // Cas 0 : Chemin direct vers la pomme
                // Si la pomme est plus proche sans utiliser de portail, on va vers la pomme.
                if ((lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesX2[0]) < 0) 
                {
                    direction2 = GAUCHE; // La pomme est à gauche de la tête du serpent.
                } 
                else if ((lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesX2[0]) > 0) 
                {
                    direction2 = DROITE; // La pomme est à droite de la tête du serpent.
                } 
                else if ((lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesY2[0]) < 0) 
                {
                    direction2 = HAUT; // La pomme est située au-dessus de la tête du serpent.
                } 
                else 
                {
                    direction2 = BAS; // La pomme est située en-dessous de la tête du serpent.
                }
                break;

            case 1:
                // Cas 1 : Chemin via le portail haut
                if (PasserPortails(lesX2, lesY2) == false)
                {
                    // Si le serpent n'est pas encore au portail haut, il se dirige vers le portail.
                    if ((TROU_HAUT.x - lesX2[0]) < 0) 
                    {
                        direction2 = GAUCHE; // Le portail haut est à gauche de la tête du serpent.
                    } 
                    else if ((TROU_HAUT.x - lesX2[0]) > 0)
                    {
                        direction2 = DROITE; // Le portail haut est à droite de la tête.
                    }
                    else if ((TROU_HAUT.y - lesY2[0]) < 0) 
                    {
                        direction2 = HAUT; // Le portail haut est au-dessus de la tête du serpent.
                    } 
                    else 
                    {
                        direction2 = BAS; // Le portail haut est en-dessous de la tête.
                    }
                } 
                else 
                {
                    // Une fois passé par le portail, on calcule la direction2 directe vers la pomme.
                    if ((lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesX2[0]) < 0) 
                    {
                        direction2 = GAUCHE; // La pomme est à gauche après téléportation.
                    } 
                    else if ((lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesX2[0]) > 0) 
                    {
                        direction2 = DROITE; // La pomme est à droite après téléportation.
                    } 
                    else if ((lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesY2[0]) < 0) 
                    {
                        direction2 = HAUT; // La pomme est au-dessus après téléportation.
                    } 
                    else 
                    {
                        direction2 = BAS; // La pomme est en-dessous après téléportation.
                    }
                }
                break;

            case 2:
                // Cas 2 : Chemin via le portail bas
                if (PasserPortails(lesX2, lesY2) == false) 
                {
                    // Si le serpent n'est pas encore au portail bas, il se dirige vers le portail.
                    if ((TROU_BAS.x - lesX2[0]) < 0) 
                    {
                        direction2 = GAUCHE; // Le portail bas est à gauche de la tête du serpent.
                    } 
                    else if ((TROU_BAS.x - lesX2[0]) > 0) 
                    {
                        direction2 = DROITE; // Le portail bas est à droite de la tête.
                    } 
                    else if ((TROU_BAS.y - lesY2[0]) < 0) 
                    {
                        direction2 = HAUT; // Le portail bas est au-dessus de la tête.
                    } 
                    else 
                    {
                        direction2 = BAS; // Le portail bas est en-dessous de la tête.
                    }
                } 
                else 
                {
                    // Une fois passé par le portail, on calcule la direction2 directe vers la pomme.
                    if ((lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesX2[0]) < 0) 
                    {
                        direction2 = GAUCHE; // La pomme est à gauche après téléportation.
                    } 
                    else if ((lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesX2[0]) > 0) 
                    {
                        direction2 = DROITE; // La pomme est à droite après téléportation.
                    } 
                    else if ((lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesY2[0]) < 0) 
                    {
                        direction2 = HAUT; // La pomme est au-dessus après téléportation.
                    } 
                    else 
                    {
                        direction2 = BAS; // La pomme est en-dessous après téléportation.
                    }
                }
                break;

            case 3:
                // Cas 3 : Chemin via le portail gauche
                if (PasserPortails(lesX2, lesY2) == false) 
                {
                    // Si le serpent n'est pas encore au portail gauche, il se dirige vers le portail.
                    if ((TROU_GAUCHE.x - lesX2[0]) < 0) 
                    {
                        direction2 = GAUCHE; // Le portail gauche est à gauche de la tête du serpent.
                    } 
                    else if ((TROU_GAUCHE.x - lesX2[0]) > 0) 
                    {
                        direction2 = DROITE; // Le portail gauche est à droite de la tête.
                    } 
                    else if ((TROU_GAUCHE.y - lesY2[0]) < 0) 
                    {
                        direction2 = HAUT; // Le portail gauche est au-dessus de la tête.
                    } 
                    else 
                    {
                        direction2 = BAS; // Le portail gauche est en-dessous de la tête.
                    }
                } 
                else 
                {
                    // Une fois passé par le portail, on calcule la direction2 directe vers la pomme.
                    if ((lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesX2[0]) < 0) 
                    {
                        direction2 = GAUCHE; // La pomme est à gauche après téléportation.
                    } 
                    else if ((lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesX2[0]) > 0) 
                    {
                        direction2 = DROITE; // La pomme est à droite après téléportation.
                    } 
                    else if ((lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesY2[0]) < 0) 
                    {
                        direction2 = HAUT; // La pomme est au-dessus après téléportation.
                    } 
                    else 
                    {
                        direction2 = BAS; // La pomme est en-dessous après téléportation.
                    }
                }
                break;

            case 4:
                // Cas 4 : Chemin via le portail droit
                if (PasserPortails(lesX2, lesY2) == false)
                {
                    // Si le serpent n'est pas encore au portail droit, il se dirige vers le portail.
                    if ((TROU_DROITE.x - lesX2[0]) < 0) 
                    {
                        direction2 = GAUCHE; // Le portail droit est à gauche de la tête du serpent.
                    } 
                    else if ((TROU_DROITE.x - lesX2[0]) > 0) 
                    {
                        direction2 = DROITE; // Le portail droit est à droite de la tête.
                    } 
                    else if ((TROU_DROITE.y - lesY2[0]) < 0) 
                    {
                        direction2 = HAUT; // Le portail droit est au-dessus de la tête.
                    } 
                    else 
                    {
                        direction2 = BAS; // Le portail droit est en-dessous de la tête.
                    }
                } 
                else 
                {
                    // Une fois passé par le portail, on calcule la direction2 directe vers la pomme.
                    if ((lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesX2[0]) < 0) 
                    {
                        direction2 = GAUCHE; // La pomme est à gauche après téléportation.
                    } 
                    else if ((lesPommesX[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesX2[0]) > 0) 
                    {
                        direction2 = DROITE; // La pomme est à droite après téléportation.
                    } 
                    else if ((lesPommesY[(NbPommesSerpentManger + NbPommesSerpentManger2)] - lesY2[0]) < 0) 
                    {
                        direction2 = HAUT; // La pomme est au-dessus après téléportation.
                    } 
                    else 
                    {
                        direction2 = BAS; // La pomme est en-dessous après téléportation.
                    }
                }
                break;
        }
        
		progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee, lesX2, lesY2);
        progresser2(lesX2, lesY2, direction2, lePlateau, &collision, &pommeMangee2, lesX, lesY);
        
        PasserPortails(lesX, lesY); //Vérifie si le serpent est passé par un portail.
        PasserPortails(lesX2, lesY2);

		if (pommeMangee) // Ajoute une pomme au compteur de pommes quand elle est mangée et arrête le jeu si le score atteint 10.
		{
            if (pommeMangee){
                NbPommesSerpentManger++;
            }
            else {
                NbPommesSerpentManger2++;
            }
            pommeMangee2 = pommeMangee ;

			gagne = ((NbPommesSerpentManger + NbPommesSerpentManger2)== NB_POMMES); // Vérifie si toutes les pommes ont été mangées.
			if (!gagne)
			{
				ajouterPomme(lePlateau, (NbPommesSerpentManger + NbPommesSerpentManger2));// Ajoute une nouvelle pomme sur le plateau.
				pommeMangee = false; // Réinitialise l'indicateur de pomme mangée.
                pommeMangee2 = false;
			}	
			
		}

        else if (pommeMangee2) // Ajoute une pomme au compteur de pommes quand elle est mangée et arrête le jeu si le score atteint 10.
		{
            NbPommesSerpentManger2++;

			gagne = ((NbPommesSerpentManger + NbPommesSerpentManger2)== NB_POMMES); // Vérifie si toutes les pommes ont été mangées.
			if (!gagne)
			{
				ajouterPomme(lePlateau, (NbPommesSerpentManger + NbPommesSerpentManger2));// Ajoute une nouvelle pomme sur le plateau.
				pommeMangee = false; // Réinitialise l'indicateur de pomme mangée.
                pommeMangee2 = false;
			}	
			
		}

		if (!gagne) // Continue à faire avancer le serpent si le jeu n'est pas terminé.
		{
			if (!collision)
            {
                usleep(ATTENTE);  // Attends un certain temps avant de redessiner le plateau.
                if (kbhit() == 1)  // Si une touche a été pressée.
                {
                    touche = getchar();  // Lit la touche pressée.
                }
            }
		}

        gotoxy(2+LARGEUR_PLATEAU, 13);
		printf("Nombre de pommes mangée Serpent 1 : %d\n", NbPommesSerpentManger );
        gotoxy(2+LARGEUR_PLATEAU, 14);
		printf("Nombre de pommes mangée Serpent 2 : %d\n", NbPommesSerpentManger2);
	} while ( (touche != STOP) && !collision && !gagne); // La boucle continue tant que l'utilisateur n'appuie pas sur STOP, qu'il n'y a pas de collision et que toutes les pommes ne sont pas mangées.
    

    enable_echo(); // Réactive l'affichage des touches.
	gotoxy(LARGEUR_PLATEAU+1, 1); // Déplace le curseur en dehors du plateau de jeu.
	if (gagne)
	{
		enable_echo();
		gotoxy(2, HAUTEUR_PLATEAU+1);
		printf("Compteur de pas pour le Serpent 1 : %d et le nombre de pommes mangée %d \n", nbDepUnitaires, NbPommesSerpentManger);// Affiche les performances du programme.
        gotoxy(2,HAUTEUR_PLATEAU+2);
		printf("Compteur de pas pour le Serpent 2 : %d et le nombre de pommes mangée %d \n", nbDepUnitaires2, NbPommesSerpentManger2);// Affiche les performances du programme.

	}
	return EXIT_SUCCESS;
}

/************************************************/
/*		FONCTIONS ET PROCEDURES DU JEU 			*/
/************************************************/

void initPlateau(tPlateau plateau)
{
    int i, j;

    // Initialisation du plateau avec des espaces vides
    for (i = 1 ; i <= LARGEUR_PLATEAU ; i++)  // Pour chaque colonne
    {
        for (int j = 1 ; j <= HAUTEUR_PLATEAU ; j++)  // Pour chaque ligne
        {
            plateau[i][j] = VIDE;  // Remplissage de la case avec un espace vide
        }
    }

    // Mise en place de la bordure autour du plateau
    // Première ligne : en haut
    for (i = 1 ; i <= LARGEUR_PLATEAU ; i++)
    {
        plateau[i][1] = BORDURE;  // Bordure sur la première ligne (haut)
    }

    // Lignes intermédiaires : les deux bords verticaux
    for (j = 1 ; j <= HAUTEUR_PLATEAU ; j++)
    {
        plateau[1][j] = BORDURE;  // Bordure sur le côté gauche
        plateau[LARGEUR_PLATEAU][j] = BORDURE;  // Bordure sur le côté droit
    }

    // Dernière ligne : en bas
    for (i = 1; i <= LARGEUR_PLATEAU ; i++)
    {
        plateau[i][HAUTEUR_PLATEAU] = BORDURE;  // Bordure sur la dernière ligne (bas)
    }

    // Suppression de certaines bordures pour créer des "trous" (portails)
    plateau[LARGEUR_PLATEAU][HAUTEUR_PLATEAU / 2] = VIDE;  // Trou en bas à droite
    plateau[LARGEUR_PLATEAU / 2][HAUTEUR_PLATEAU] = VIDE;  // Trou en bas au centre
    plateau[LARGEUR_PLATEAU / 2][1] = VIDE;  // Trou en haut au centre
    plateau[1][HAUTEUR_PLATEAU / 2] = VIDE;  // Trou en haut à droite

    placerPaves(plateau); // Placement des pavés
}


void dessinerPlateau(tPlateau plateau)
{
    for (int i = 1; i <= LARGEUR_PLATEAU; i++)  // Parcourt les colonnes
    {
        for (int j = 1; j <= HAUTEUR_PLATEAU; j++)  // Parcourt les lignes
        {
            afficher(i, j, plateau[i][j]);  // Appelle la fonction afficher pour chaque case
        }
    }
}

void ajouterPomme(tPlateau plateau, int Pomme)
{
    // Génère la position de la pomme à partir des tableaux de positions
    plateau[lesPommesX[Pomme]][lesPommesY[Pomme]] = POMME;  // Place la pomme sur le plateau
    afficher(lesPommesX[Pomme], lesPommesY[Pomme], POMME);  // Affiche la pomme à l'écran
}

void afficher(int x, int y, char car)
{
    gotoxy(x, y);  // Déplace le curseur à la position (x, y)
    printf("%c", car);  // Affiche le caractère à cette position
    gotoxy(1, 1);  // Remet le curseur en haut à gauche de l'écran
}


void effacer(int x, int y)
{
    gotoxy(x, y);  // Déplace le curseur à la position (x, y)
    printf(" ");  // Efface la case en affichant un espace vide
    gotoxy(1, 1);  // Remet le curseur en haut à gauche de l'écran
}


void dessinerSerpent(int lesX[], int lesY[])
{
    // Affiche les anneaux du serpent
    for (int i = 1; i < TAILLE; i++)  // Parcourt le serpent 
    {
        afficher(lesX[i], lesY[i], CORPS);  // Affiche un segment du serpent (corps)
    }
    afficher(lesX[0], lesY[0], TETE);  // Affiche la tête du serpent
}

void dessinerSerpent2(int lesX[], int lesY[])
{
    // Affiche les anneaux du serpent
    for (int i = 1; i < TAILLE; i++)  // Parcourt le serpent 
    {
        afficher(lesX[i], lesY[i], CORPS);  // Affiche un segment du serpent (corps)
    }
    afficher(lesX[0], lesY[0], TETE2);  // Affiche la tête du serpent
}


// Modification de la fonction progresser pour inclure la détection de collision avec l'autre serpent
void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool *collision, bool *pomme, int autreX[], int autreY[]) {   
    direction = trouverDirectionSure(lesX, lesY, direction, plateau, autreX, autreY); // Ajout des paramètres autreX et autreY
    effacer(lesX[TAILLE - 1], lesY[TAILLE - 1]);

    for (int i = TAILLE - 1; i > 0; i--) 
    { 
        lesX[i] = lesX[i - 1];
        lesY[i] = lesY[i - 1];
    }
    switch (direction) 
    { 
        case HAUT: 
            lesY[0]--;
            break;                                                                       
        case BAS: 
            lesY[0]++;
            break;                                                                        
        case DROITE: 
            lesX[0]++;
            break;                                                                     
        case GAUCHE: 
            lesX[0]--;
            break;                                                                     
    }

    if (lesX[0] <= 0)
    {
        lesX[0] = LARGEUR_PLATEAU;
    }                                                          
    else if (lesX[0] > LARGEUR_PLATEAU)
    {
        lesX[0] = 1;
    } 

    if (lesY[0] <= 0) 
    {
        lesY[0] = HAUTEUR_PLATEAU;
    }                                                          
    else if (lesY[0] > HAUTEUR_PLATEAU)
    {
        lesY[0] = 1;
    }
    
    *pomme = (plateau[lesX[0]][lesY[0]] == POMME);
    if (*pomme) 
    {
        plateau[lesX[0]][lesY[0]] = VIDE;
    }
    else if (plateau[lesX[0]][lesY[0]] == BORDURE) 
    {
        *collision = true;
    }

    // Ajout de la vérification de collision avec l'autre serpent
    if (estSurCorpsSerpent(lesX[0], lesY[0], lesX, lesY) || estSurCorpsAutreSerpent(lesX[0], lesY[0], autreX, autreY)) 
    {
        *collision = true;
    }  

    if (*collision || estSurPave(lesX[0], lesY[0], plateau)) 
    {
        *collision = true;
    }

    dessinerSerpent(lesX, lesY);
    nbDepUnitaires++;
}

// Modification similaire pour progresser2
void progresser2(int lesX2[], int lesY2[], char direction2, tPlateau plateau, bool *collision, bool *pomme, int autreX[], int autreY[]) {   
    direction2 = trouverDirectionSure(lesX2, lesY2, direction2, plateau, autreX, autreY);
    effacer(lesX2[TAILLE - 1], lesY2[TAILLE - 1]);

    for (int i = TAILLE - 1; i > 0; i--) 
    { 
        lesX2[i] = lesX2[i - 1];
        lesY2[i] = lesY2[i - 1];
    }
    switch (direction2) 
    { 
        case HAUT: 
            lesY2[0]--;
            break;                                                                       
        case BAS: 
            lesY2[0]++;
            break;                                                                        
        case DROITE: 
            lesX2[0]++;
            break;                                                                     
        case GAUCHE: 
            lesX2[0]--;
            break;                                                                     
    }

    if (lesX2[0] <= 0)
    {
        lesX2[0] = LARGEUR_PLATEAU;
    }                                                          
    else if (lesX2[0] > LARGEUR_PLATEAU)
    {
        lesX2[0] = 1;
    } 

    if (lesY2[0] <= 0) 
    {
        lesY2[0] = HAUTEUR_PLATEAU;
    }                                                          
    else if (lesY2[0] > HAUTEUR_PLATEAU)
    {
        lesY2[0] = 1;
    }
    
    *pomme = (plateau[lesX2[0]][lesY2[0]] == POMME);
    if (*pomme) 
    {
        plateau[lesX2[0]][lesY2[0]] = VIDE;
    }
    else if (plateau[lesX2[0]][lesY2[0]] == BORDURE) 
    {
        *collision = true;
    }

    if (estSurCorpsSerpent(lesX2[0], lesY2[0], lesX2, lesY2) || estSurCorpsAutreSerpent(lesX2[0], lesY2[0], autreX, autreY)) 
    {
        *collision = true;
    }

    if (*collision || estSurPave(lesX2[0], lesY2[0], plateau)) 
    {
        *collision = true;
    }


    dessinerSerpent2(lesX2, lesY2);
    nbDepUnitaires2++;
}

// Modification de la fonction trouverDirectionSure pour inclure la détection de l'autre serpent
char trouverDirectionSure(int lesX[], int lesY[], char directionActuelle, tPlateau plateau, int autreX[], int autreY[]) {
    if (directionEstSure(lesX[0], lesY[0], directionActuelle, lesX, lesY, autreX, autreY, plateau)) 
    { 
        return directionActuelle;
    }
    if (directionEstSure(lesX[0], lesY[0], GAUCHE, lesX, lesY, autreX, autreY, plateau)) 
    { 
        return GAUCHE;
    }
    if (directionEstSure(lesX[0], lesY[0], DROITE, lesX, lesY, autreX, autreY, plateau))
    {
        return DROITE;
    }
    if (directionEstSure(lesX[0], lesY[0], HAUT, lesX, lesY, autreX, autreY, plateau)) 
    { 
        return HAUT;
    }
    if (directionEstSure(lesX[0], lesY[0], BAS, lesX, lesY, autreX, autreY, plateau)) 
    { 
        return BAS;
    }
    return directionActuelle;
}

bool estSurCorpsSerpent(int x, int y, int lesX[], int lesY[]) {   // Vérifie si une position est occupée par le corps du serpent
    bool collision = false; // Initialise la variable de collision
    for (int i = 1; i < TAILLE; i++) // Parcourt les parties du serpent, sauf la tête
    { 
        if (lesX[i] == x && lesY[i] == y) // Compare les coordonnées fournies avec celles du serpent
        { 
            collision = true; // Si une partie correspond, une collision est détectée
        }
    }
    return collision; // Retourne vrai si une collision est détectée, sinon faux
}




bool PasserPortails(int lesX[], int lesY[]) {
    bool teleporter = false ;
    if (lesX[0] <= 0) {
        lesX[0] = LARGEUR_PLATEAU;  // Réapparaît à droite
        teleporter = true;
    } else if (lesX[0] > LARGEUR_PLATEAU) {
        lesX[0] = 1;  // Réapparaît à gauche
        teleporter = true;
    }

    if (lesY[0] <= 0) {
        lesY[0] = HAUTEUR_PLATEAU;  // Réapparaît en bas
        teleporter = true;
    } else if (lesY[0] > HAUTEUR_PLATEAU) {
        lesY[0] = 1;  // Réapparaît en haut
        teleporter = true;
    }
    return teleporter;
}



int minimunTableau(tChemins Tableau){
    int minDistance = Tableau[0];
    int minDistanceI = 0;
    
    for (int i = 0; i < 5; i++)
    {
        if (minDistance > Tableau[i]) // Si une distance plus courte est trouvée
        {
            minDistance = Tableau[i]; // Met à jour la distance minimale
            minDistanceI = i;     // Met à jour l'index du chemin le plus court
        }
    }
    return minDistanceI;
}


bool estSurPave(int x, int y, tPlateau plateau) { // Vérifie si une position est occupée par un pavé
    return plateau[x][y] == PAVE; // Retourne vrai si la position correspond à un pavé
}


void placerPaves(tPlateau plateau) { // Place les pavés sur le plateau
    for (int p = 0; p < NB_PAVES; p++) { // Parcourt chaque pavé à placer
        for (int i = 0; i < TAILLE_PAVE; i++) { // Parcourt les lignes du pavé
            for (int j = 0; j < TAILLE_PAVE; j++) { // Parcourt les colonnes du pavé
                plateau[lesPavesX[p] + i][lesPavesY[p] + j] = PAVE; // Marque la position comme pavé sur le plateau
            }
        }
    }
}


bool estSurCorpsAutreSerpent(int x, int y, int autreX[], int autreY[]) {
    // Vérifie si la tête d'un serpent est sur le corps de l'autre
    for (int i = 0; i < TAILLE; i++) {
        if (x == autreX[i] && y == autreY[i]) {
            return true;
        }
    }
    return false;
}

bool directionEstSure(int x, int y, char direction, int lesX[], int lesY[], int autreX[], int autreY[], tPlateau plateau) {
    // Met à jour les coordonnées en fonction de la direction
    switch (direction) {
        case HAUT: 
            y--; 
            break;
        case BAS: 
            y++; 
            break;
        case GAUCHE:
            x--; 
            break;
        case DROITE: 
            x++; 
            break;
    }

    // Gère les sorties du plateau pour les rendre cycliques
    x = (x + LARGEUR_PLATEAU - 1) % LARGEUR_PLATEAU + 1;
    y = (y + HAUTEUR_PLATEAU - 1) % HAUTEUR_PLATEAU + 1;

    // Vérifie si la position est sûre
    bool estSur = !estSurCorpsSerpent(x, y, lesX, lesY) && 
                  !estSurCorpsAutreSerpent(x, y, autreX, autreY) && 
                  !estSurPave(x, y, plateau) && 
                  plateau[x][y] != BORDURE;

    return estSur;
}


/************************************************/
/*				 FONCTIONS UTILITAIRES 			*/
/************************************************/
void gotoxy(int x, int y)
{ 
    printf("\033[%d;%df", y, x);
}

int kbhit()
{
	// la fonction retourne :
	// 1 si un caractere est present
	// 0 si pas de caractere présent
	int unCaractere = 0;
	struct termios oldt, newt;
	int ch;
	int oldf;

	// mettre le terminal en mode non bloquant
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
	ch = getchar();

	// restaurer le mode du terminal
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
 
	if(ch != EOF)
	{
		ungetc(ch, stdin);
		unCaractere = 1;
	} 
	return unCaractere;
}

// Fonction pour désactiver l'echo
void disable_echo()
{
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1) 
	{
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Désactiver le flag ECHO
    tty.c_lflag &= ~ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1)
	{
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

// Fonction pour réactiver l'echo
void enable_echo()
{
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1)
	{
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Réactiver le flag ECHO
    tty.c_lflag |= ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1)
	{
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}