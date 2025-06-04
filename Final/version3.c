/**
* @file version3.c
* @brief Jeu Snake version 3 avec obstacles pour le projet SAE 1.02.
*
* Ce programme implémente une version du jeu Snake avec des obstacles. 
* Le serpent se déplace automatiquement vers des pommes tout en évitant les pavés et les bords du plateau.
* Des portails permettent au serpent de traverser les bords du plateau pour réapparaître de l'autre côté.
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

// Constantes du jeu
#define TAILLE 10  // Taille du serpent.
#define LARGEUR_PLATEAU 80  // Largeur du plateau de jeu.
#define HAUTEUR_PLATEAU 40  // Hauteur du plateau de jeu.
#define X_DEPART_SERPENT 40  // Position en X du serpent au départ (au centre du plateau).
#define Y_DEPART_SERPENT 20  // Position en Y du serpent au départ (au centre du plateau).
#define NB_POMMES 10  // Nombre de pommes présentes sur le plateau.
#define ATTENTE 200000  // Temps d'attente entre chaque déplacement du serpent (en microsecondes).
#define CORPS 'X'  // Caractère utilisé pour dessiner le corps du serpent.
#define TETE 'O'  // Caractère utilisé pour dessiner la tête du serpent.
#define BORDURE '#'  // Caractère utilisé pour dessiner les bords du plateau.
#define STOP 'a'  // Touche pour arrêter le jeu.
#define VIDE ' '  // Caractère pour un espace vide sur le plateau.
#define POMME '6'  // Caractère pour une pomme.
#define HAUT 'z'  // Touche pour diriger le serpent vers le haut.
#define BAS 's'  // Touche pour diriger le serpent vers le bas.
#define GAUCHE 'q'  // Touche pour diriger le serpent vers la gauche.
#define DROITE 'd'  // Touche pour diriger le serpent vers la droite.
#define PARSECONDE 1000000.0 // Nombre de microsecondes par seconde
#define PAVE '#' // Représente un pavé d'obstacle
#define NB_PAVES 6 // Nombre de pavés d'obstacles
#define TAILLE_PAVE 5 // Dimension (carrée) des pavés

// Positions des pommes et des pavés
int lesPommesX[NB_POMMES] = {75, 75, 78, 2, 8, 78, 74, 2, 72, 5}; // Positions en X des pommes.
int lesPommesY[NB_POMMES] = {8, 39, 2, 2, 5, 39, 33, 38, 35, 2}; // Positions en Y des pommes.
int lesPavesX[NB_PAVES] = {3, 74, 3, 74, 38, 38}; // Positions en X des pavés.
int lesPavesY[NB_PAVES] = {3, 3, 34, 34, 21, 15}; // Positions en Y des pavés.

typedef char tPlateau[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1]; // Initialiser le plateau de jeu.

typedef int tChemins[5]; // Initialiser le tableau avec les 5 chemins possibles.

int NbPommesSerpentManger = 0; // Compteur du nombre de déplacements effectués par le serpent.
int nbDepUnitaires = 0; // Compteur du nombre de pommes mangées par le serpent.

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
void ajouterPomme(tPlateau plateau); // Ajoute une pomme à une position définie.
void placerPaves(tPlateau plateau); // Ajoute les pavés à une position définie.
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
    // 2 tableaux contenant les positions des éléments qui constituent le serpent
    int lesX[TAILLE];  // Tableau pour les positions en X des différentes parties du serpent.
    int lesY[TAILLE];  // Tableau pour les positions en Y des différentes parties du serpent.

    // Représente la touche frappée par l'utilisateur : touche de direction ou pour l'arrêt
    char touche;

    // Direction courante du serpent (HAUT, BAS, GAUCHE ou DROITE)
    char direction;

    // Le plateau de jeu
    tPlateau lePlateau;

    bool collision = false;  // Variable pour détecter si le serpent a heurter avec quelque chose (mur ou lui-même).
    bool gagne = false;  // Indicateur de victoire (si toutes les pommes sont mangées).
    bool pommeMangee = false;  // Indicateur pour savoir si une pomme a été mangée pendant le tour.

    // Initialisation de la position du serpent : positionnement de la tête en (X_DEPART_SERPENT, Y_DEPART_SERPENT), puis des anneaux à sa gauche.
    for (int i = 0; i < TAILLE; i++) {
        lesX[i] = X_DEPART_SERPENT - i;  // Position X de chaque partie du serpent.
        lesY[i] = Y_DEPART_SERPENT;  // Position Y de chaque partie du serpent.
    }

    // Mise en place du plateau
    initPlateau(lePlateau);  // Initialisation du plateau de jeu.
    system("clear");  // Effacement de l'écran .
    ajouterPomme(lePlateau);  // Ajoute une pomme sur le plateau.
    dessinerPlateau(lePlateau);  // Dessine le plateau à l'écran.

    // Initialisation : le serpent se dirige vers la droite
    dessinerSerpent(lesX, lesY);  // Dessine le serpent au début.
    disable_echo();  // Désactive l'affichage des touches.
    direction = DROITE;  // Initialisation de la direction du serpent vers la droite.

    time_t debut = clock(); // Enregistre l'heure de début pour calculer le temps d'exécution du programme.
    
    // Boucle de jeu. Le jeu continue tant que l'utilisateur n'appuie pas sur la touche STOP ou qu'il n'y a pas de collision ou que toutes les pommes ne sont pas mangées.
    do {
		int CheminDirectPomme = abs(lesX[0] - lesPommesX[NbPommesSerpentManger]) + abs(lesY[0] - lesPommesY[NbPommesSerpentManger]); // Calcul de la distance directe entre la tête du serpent et la pomme

        // Calcul des distances en passant par différents portails (haut, bas, gauche, droite)
        // Chaque chemin nécessite de passer par un portail et d'en sortir de l'autre côté avant d'atteindre la pomme.
        int CheminPortailHaut = abs(lesX[0] - TROU_HAUT.x) + abs(lesY[0] - TROU_HAUT.y) + abs(lesPommesX[NbPommesSerpentManger] - TROU_BAS.x) + abs(lesPommesY[NbPommesSerpentManger] - TROU_BAS.y);
        int CheminPortailBas = abs(lesX[0] - TROU_BAS.x) + abs(lesY[0] - TROU_BAS.y) + abs(lesPommesX[NbPommesSerpentManger] - TROU_HAUT.x) + abs(lesPommesY[NbPommesSerpentManger] - TROU_HAUT.y);
        int CheminPortailGauche = abs(lesX[0] - TROU_GAUCHE.x) + abs(lesY[0] - TROU_GAUCHE.y) + abs(lesPommesX[NbPommesSerpentManger] - TROU_DROITE.x) + abs(lesPommesY[NbPommesSerpentManger] - TROU_DROITE.y);
        int CheminPortailDroite = abs(lesX[0] - TROU_DROITE.x) + abs(lesY[0] - TROU_DROITE.y) + abs(lesPommesX[NbPommesSerpentManger] - TROU_GAUCHE.x) + abs(lesPommesY[NbPommesSerpentManger] - TROU_GAUCHE.y);

        // Tableau contenant les distances calculées pour chaque chemin
        tChemins Tableau = {CheminDirectPomme, CheminPortailHaut, CheminPortailBas, CheminPortailGauche, CheminPortailDroite};
        
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

        int minimunDes5distances = minimunTableau(Tableau); // Calcule la minimun des distances pour atteindre la pomme.

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
        
		progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee); // Le serpent progresse dans la direction déterminée.
        PasserPortails(lesX, lesY); //Vérifie si le serpent est passé par un portail.

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
    

    enable_echo(); // Réactive l'affichage des touches.
	gotoxy(LARGEUR_PLATEAU+1, 1); // Déplace le curseur en dehors du plateau de jeu.
	if (gagne)
	{
		clock_t fin = clock(); // Enregistre l'heure de fin pour calculer le temps d'exécution.
		enable_echo();
		gotoxy(2, HAUTEUR_PLATEAU+1);
		printf("Compteur de pas : %d\n", nbDepUnitaires);// Affiche les performances du programme.
        gotoxy(2,HAUTEUR_PLATEAU+2);
		printf("Temps écoulé : %f en seconde\n", difftime(fin, debut) / PARSECONDE );// Affiche les performances du programme.
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

void ajouterPomme(tPlateau plateau)
{
    // Génère la position de la pomme à partir des tableaux de positions
    plateau[lesPommesX[NbPommesSerpentManger]][lesPommesY[NbPommesSerpentManger]] = POMME;  // Place la pomme sur le plateau
    afficher(lesPommesX[NbPommesSerpentManger], lesPommesY[NbPommesSerpentManger], POMME);  // Affiche la pomme à l'écran
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


void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool *collision, bool *pomme) {   // Met à jour la position du serpent
    direction = trouverDirectionSure(lesX, lesY, direction, plateau); // Vérifie ou trouve une direction sûre
    effacer(lesX[TAILLE - 1], lesY[TAILLE - 1]); // Efface le dernier segment du serpent

    for (int i = TAILLE - 1; i > 0; i--)// Déplace chaque segment vers l'avant 
    { 
        lesX[i] = lesX[i - 1]; // Copie la position x du segment précédent
        lesY[i] = lesY[i - 1]; // Copie la position y du segment précédent
    }
    switch (direction) // Met à jour la position de la tête selon la direction
    { 
        case HAUT: 
            lesY[0]--; // Monte
            break;                                                                       
        case BAS: 
            lesY[0]++; // Descend
            break;                                                                        
        case DROITE: 
            lesX[0]++; // Va à droite
            break;                                                                     
        case GAUCHE: 
            lesX[0]--; // Va à gauche 
            break;                                                                     
    }

    if (lesX[0] <= 0)
    {
        lesX[0] = LARGEUR_PLATEAU;// Gère la sortie à gauche
    }                                                          
    else if (lesX[0] > LARGEUR_PLATEAU)
    {
        lesX[0] = 1; // Gère la sortie à droite
    } 

    if (lesY[0] <= 0) 
    {
        lesY[0] = HAUTEUR_PLATEAU;// Gère la sortie en haut
    }                                                          
    else if (lesY[0] > HAUTEUR_PLATEAU)
    {
        lesY[0] = 1; // Gère la sortie en bas 
    }
    
    *pomme = (plateau[lesX[0]][lesY[0]] == POMME); // Vérifie si une pomme est mangée
    if (*pomme) 
    {
        plateau[lesX[0]][lesY[0]] = VIDE;   // Supprime la pomme si mangée
    }
    else if (plateau[lesX[0]][lesY[0]] == BORDURE) 
    {
        *collision = true; // Vérifie si une bordure est touchée
    }

    if (estSurCorpsSerpent(lesX[0], lesY[0], lesX, lesY)) 
    {
        *collision = true;// Vérifie la collision avec le corps  
    }                         
    dessinerSerpent(lesX, lesY); // Redessine le serpent
    nbDepUnitaires++; // Incrémente le compteur de mouvements
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


char trouverDirectionSure(int lesX[], int lesY[], char directionActuelle, tPlateau plateau) { // Cherche une direction sûre pour éviter les collisions
    if (directionEstSure(lesX[0], lesY[0], directionActuelle, lesX, lesY, plateau)) // Vérifie si la direction actuelle est sûre
    { 
        return directionActuelle; // Si oui, la conserve
    }
    if (directionEstSure(lesX[0], lesY[0], GAUCHE, lesX, lesY, plateau)) // Vérifie si GAUCHE est sûre
    { 
        return GAUCHE; // Retourne GAUCHE si sûr
    }
    if (directionEstSure(lesX[0], lesY[0], DROITE, lesX, lesY, plateau))  // Vérifie si DROITE est sûre
    {
        return DROITE; // Retourne DROITE si sûr
    }
    if (directionEstSure(lesX[0], lesY[0], HAUT, lesX, lesY, plateau)) // Vérifie si HAUT est sûr
    { 
        return HAUT; // Retourne HAUT si sûr
    }
    if (directionEstSure(lesX[0], lesY[0], BAS, lesX, lesY, plateau))// Vérifie si BAS est sûr
    { 
        return BAS; // Retourne BAS si sûr
    }
    return directionActuelle; // Sinon, retourne la direction actuelle (risquée)
}


bool PasserPortails(int lesX[], int lesY[]){ //Vérifie si le serpent est passé par un portail.
    bool teleporter = false ;
    if (lesX[0] <= 0)  // Si la tête du serpent dépasse la bordure gauche
    {
        teleporter = true;
    }
    else if (lesX[0] > LARGEUR_PLATEAU) // Si la tête dépasse la bordure droite
    {
        teleporter = true;
    }

    if (lesY[0] <= 0)  // Si la tête dépasse la bordure du haut
    {
        teleporter = true;
    }
    else if (lesY[0] > HAUTEUR_PLATEAU) // Si la tête dépasse la bordure du bas
    {
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


bool directionEstSure(int x, int y, char direction, int lesX[], int lesY[], tPlateau plateau) {   // Vérifie si une direction est sûre
    bool estSur; // Initialise la variable pour le résultat

    switch (direction) // Met à jour les coordonnées en fonction de la direction
    { 
        case HAUT: 
            y--; // Déplace vers le haut
            break;  
        case BAS: 
            y++; // Déplace vers le bas
            break;                                                                   
        case GAUCHE: 
            x--; // Déplace vers la gauche
            break;                                                                
        case DROITE: 
            x++; // Déplace vers la droite
            break;                                                                
    }

    x = (x + LARGEUR_PLATEAU - 1) % LARGEUR_PLATEAU + 1; // Gère les sorties horizontales du plateau
    y = (y + HAUTEUR_PLATEAU - 1) % HAUTEUR_PLATEAU + 1; // Gère les sorties verticales du plateau

    estSur = !estSurCorpsSerpent(x, y, lesX, lesY) && !estSurPave(x, y, plateau) &&  plateau[x][y] != BORDURE; // Vérifie que la position n'est pas sur le corps du serpent et vérifie que la position n'est pas sur un pavé vérifie que la position n'est pas une bordure

    return estSur; // Retourne vrai si la direction est sûre, sinon faux
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