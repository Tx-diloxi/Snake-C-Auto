/**
 * @file version3.c
 * @brief Jeu Snake version 3 avec obstacles pour le projet SAE 1.02.
 *
 * Ce fichier implémente la version 3 du Snake autonome avec des obstacles fixes (pavés).
 * Le serpent doit naviguer autour des obstacles tout en collectant les pommes.
 *
 * @author
 * LE SECH Marceau
 * CAMARA Kémo
 *
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

// ================================
// Constantes du jeu
// ================================
#define TAILLE 10                 // Taille initiale du serpent
#define LARGEUR_PLATEAU 80        // Largeur du plateau
#define HAUTEUR_PLATEAU 40        // Hauteur du plateau
#define X_DEPART_SERPENT 40       // Position de départ du serpent (X)
#define Y_DEPART_SERPENT 20       // Position de départ du serpent (Y)
#define NB_POMMES 10              // Nombre total de pommes à collecter
#define NB_PAVES 6                // Nombre de pavés d'obstacles
#define TAILLE_PAVE 5             // Dimension (carrée) des pavés
#define ATTENTE 100000            // Temps d'attente (microsecondes) entre deux mouvements
#define CORPS 'X'                 // Représente le corps du serpent
#define TETE 'O'                  // Représente la tête du serpent
#define BORDURE '#'               // Représente les bords du plateau
#define PAVE '#'                  // Représente un pavé d'obstacle
#define STOP 'a'                  // Touche pour arrêter le jeu
#define VIDE ' '                  // Case vide du plateau
#define POMME '6'                 // Représente une pomme
#define HAUT 'z'                  // Touches de direction
#define BAS 's'                   // Touches de direction
#define GAUCHE 'q'                // Touches de direction
#define DROITE 'd'                // Touches de direction
#define PARSECONDE 1000000.0      // Nombre de microsecondes par seconde

// ================================
// Structures de données principales
// ================================

// Représente les positions des pommes et des pavés sur le plateau
int lesPommesX[NB_POMMES] = {75, 75, 78, 2, 8, 78, 74, 2, 72, 5};
int lesPommesY[NB_POMMES] = {8, 39, 2, 2, 5, 39, 33, 38, 35, 2};
int lesPavesX[NB_PAVES] = {3, 74, 3, 74, 38, 38};
int lesPavesY[NB_PAVES] = {3, 3, 34, 34, 21, 15};

typedef char tPlateau[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1]; // Plateau de jeu
typedef int tChemins[5]; // Tableau des distances possibles vers une pomme

// Variables globales de suivi
typedef struct {
    int x; // Coordonnée X
    int y; // Coordonnée Y
} Portail;

const Portail TROU_HAUT = {40, 0}; // Portail en haut du plateau
const Portail TROU_BAS = {40, 40}; // Portail en bas
const Portail TROU_GAUCHE = {0, 20}; // Portail à gauche
const Portail TROU_DROITE = {80, 20}; // Portail à droite

int NbPommesSerpentManger = 0; // Compteur de pommes mangées
int nbDepUnitaires = 0;        // Compteur de déplacements unitaires

// ================================
// Prototypes de fonctions
// ================================
void afficher(int x, int y, char car); // Affiche un caractère à une position donnée.
void effacer(int x, int y); // Efface un caractère à une position donnée.
void dessinerSerpent(int lesX[], int lesY[]); // Dessine le serpent entier sur le plateau.
void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool *collision, bool *pomme); // Fait avancer le serpent dans une direction donnée.
void gotoxy(int x, int y); // Déplace le curseur à une position spécifique dans le terminal.
int kbhit(void); // Vérifie si une touche a été pressée.
void disable_echo(void); // Désactive l'écho des touches dans le terminal.
void enable_echo(void); // Réactive l'écho des touches dans le terminal.
bool PasserPortails(int lesX[], int lesY[]); // Gère la traversée des bords du plateau via les portails.
int minimunTableau(tChemins Tableau); // Retourne l'index de la plus petite distance dans un tableau de distances.
bool estSurCorpsSerpent(int x, int y, int lesX[], int lesY[]); // Vérifie si une position est occupée par le corps du serpent.
bool estSurPave(int x, int y, tPlateau plateau); // Vérifie si une position est occupée par un pavé.
bool directionEstSure(int x, int y, char direction, int lesX[], int lesY[], tPlateau plateau); // Vérifie si une direction est sans danger.
char trouverDirectionSure(int lesX[], int lesY[], char directionActuelle, tPlateau plateau); // Trouve une direction sûre pour le serpent.


int main() {
    // Variables de suivi du serpent
    int lesX[TAILLE], lesY[TAILLE];
    char touche, direction;
    tPlateau lePlateau;
    bool collision = false;
    bool gagne = false;
    bool pommeMangee = false;

    // Initialisation de la position initiale du serpent
    for (int i = 0; i < TAILLE; i++) {
        lesX[i] = X_DEPART_SERPENT - i; // Le serpent commence horizontalement vers la gauche
        lesY[i] = Y_DEPART_SERPENT;
    }

    // Initialisation du plateau
    initPlateau(lePlateau);
    system("clear"); // Nettoie l'écran
    ajouterPomme(lePlateau); // Ajoute une première pomme
    dessinerPlateau(lePlateau);
    dessinerSerpent(lesX, lesY);
    disable_echo(); // Désactive l'écho des touches dans le terminal
    direction = DROITE; // Direction initiale

    time_t debut = clock(); // Temps de départ

    do {
        // Calcul des distances vers la pomme et les portails
        int CheminDirectPomme = abs(lesX[0] - lesPommesX[NbPommesSerpentManger]) + abs(lesY[0] - lesPommesY[NbPommesSerpentManger]);
        int CheminPortailHaut = abs(lesX[0] - TROU_HAUT.x) + abs(lesY[0] - TROU_HAUT.y) + abs(lesPommesX[NbPommesSerpentManger] - TROU_BAS.x) + abs(lesPommesY[NbPommesSerpentManger] - TROU_BAS.y);
        int CheminPortailBas = abs(lesX[0] - TROU_BAS.x) + abs(lesY[0] - TROU_BAS.y) + abs(lesPommesX[NbPommesSerpentManger] - TROU_HAUT.x) + abs(lesPommesY[NbPommesSerpentManger] - TROU_HAUT.y);
        int CheminPortailGauche = abs(lesX[0] - TROU_GAUCHE.x) + abs(lesY[0] - TROU_GAUCHE.y) + abs(lesPommesX[NbPommesSerpentManger] - TROU_DROITE.x) + abs(lesPommesY[NbPommesSerpentManger] - TROU_DROITE.y);
        int CheminPortailDroite = abs(lesX[0] - TROU_DROITE.x) + abs(lesY[0] - TROU_DROITE.y) + abs(lesPommesX[NbPommesSerpentManger] - TROU_GAUCHE.x) + abs(lesPommesY[NbPommesSerpentManger] - TROU_GAUCHE.y);

        // Stockage des distances
        tChemins Tableau = {CheminDirectPomme, CheminPortailHaut, CheminPortailBas, CheminPortailGauche, CheminPortailDroite};
        int minimunDes5distances = minimunTableau(Tableau); // Trouve la distance minimale

        
        // Affichage pour moi, pour comprendre comment les distances fonctionnes
        gotoxy(2+LARGEUR_PLATEAU, 1);
        printf("Distance Pomme : %4d pas", CheminDirectPomme);
        gotoxy(2+LARGEUR_PLATEAU, 2);
        printf("Distance Portail Haut : %4d pas", CheminPortailHaut);
        gotoxy(2+LARGEUR_PLATEAU, 3);
        printf("Distance Portail Bas : %4d pas", CheminPortailBas);
        gotoxy(2+LARGEUR_PLATEAU, 4);
        printf("Distance Portail Gauche : %4d pas", CheminPortailGauche);
        gotoxy(2+LARGEUR_PLATEAU, 5);
        printf("Distance Portail Droit : %4d pas", CheminPortailDroite);

        int minimunDes5distances = minimunTableau(Tableau); // Calcule la minimun des distances pour atteindre la pomme.

        // Logique de choix de direction
        switch (minimunDes5distances)
        {
            case 0:
                // Cas 0 : Chemin direct vers la pomme
                // Si la pomme est plus proche sans utiliser de portail, on va vers la pomme.
                if ((lesPommesY[NbPommesSerpentManger] - lesY[0]) < 0) 
                {
                    direction = HAUT; // La pomme est située au-dessus de la tête du serpent.
                } 
                else if ((lesPommesY[NbPommesSerpentManger] - lesY[0]) > 0) 
                {
                    direction = BAS; // La pomme est située en-dessous de la tête du serpent.
                } 
                else if ((lesPommesX[NbPommesSerpentManger] - lesX[0]) < 0) 
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
                    if ((lesPommesY[NbPommesSerpentManger] - lesY[0]) < 0) 
                    {
                        direction = HAUT; // La pomme est au-dessus après téléportation.
                    } 
                    else if ((lesPommesY[NbPommesSerpentManger] - lesY[0]) > 0) 
                    {
                        direction = BAS; // La pomme est en-dessous après téléportation.
                    } 
                    else if ((lesPommesX[NbPommesSerpentManger] - lesX[0]) < 0) 
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
                    if ((lesPommesY[NbPommesSerpentManger] - lesY[0]) < 0)
                    {
                        direction = HAUT; // La pomme est au-dessus après téléportation.
                    } 
                    else if ((lesPommesY[NbPommesSerpentManger] - lesY[0]) > 0) 
                    {
                        direction = BAS; // La pomme est en-dessous après téléportation.
                    } 
                    else if ((lesPommesX[NbPommesSerpentManger] - lesX[0]) < 0) 
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
                    if ((lesPommesY[NbPommesSerpentManger] - lesY[0]) < 0) 
                    {
                        direction = HAUT; // La pomme est au-dessus après téléportation.
                    } 
                    else if ((lesPommesY[NbPommesSerpentManger] - lesY[0]) > 0) 
                    {
                        direction = BAS; // La pomme est en-dessous après téléportation.
                    } 
                    else if ((lesPommesX[NbPommesSerpentManger] - lesX[0]) < 0) 
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
                    if ((lesPommesY[NbPommesSerpentManger] - lesY[0]) < 0) 
                    {
                        direction = HAUT; // La pomme est au-dessus après téléportation.
                    } 
                    else if ((lesPommesY[NbPommesSerpentManger] - lesY[0]) > 0) 
                    {
                        direction = BAS; // La pomme est en-dessous après téléportation.
                    } 
                    else if ((lesPommesX[NbPommesSerpentManger] - lesX[0]) < 0) 
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
        // Mise à jour du serpent
        progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee);
        PasserPortails(lesX, lesY);

       	if (pommeMangee) // Ajoute une pomme au compteur de pommes quand elle est mangée et arrête le jeu si le score atteint 10.
		{
            NbPommesSerpentManger++; // Incrémente le nombre de pommes mangées.
			gagne = (NbPommesSerpentManger==NB_POMMES); // Vérifie si toutes les pommes ont été mangées.
			if (!gagne)
			{
				ajouterPomme(lePlateau);// Ajoute une nouvelle pomme sur le plateau.
				pommeMangee = false; // Réinitialise l'indicateur de pomme mangée.
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
	} while ( (touche != STOP) && !collision && !gagne); // La boucle continue tant que l'utilisateur n'appuie pas sur STOP, qu'il n'y a pas de collision et que toutes les pommes ne sont pas mangées.
    

    enable_echo();
    if (gagne) {
        clock_t fin = clock();
        gotoxy(2, HAUTEUR_PLATEAU+1);
        printf("Compteur de pas : %d\n", nbDepUnitaires);
        gotoxy(2, HAUTEUR_PLATEAU+2);
        printf("Temps écoulé : %f en seconde\n", difftime(fin, debut) / PARSECONDE);
    }

    return EXIT_SUCCESS;
}