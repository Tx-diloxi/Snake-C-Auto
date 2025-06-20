/**
 * @file version4.c
 * @brief Automatisation d'un jeu snake codé en C lors de la SAE 1.01
 * @author Chauvel Arthur, Le Chevère Yannis
 * @version 4.0
 * @date 09/01/2025
 *
 * Cette version initialise le plateau de jeu avec les dimensions données en constantes,
 * elle initialise deux serpent à la position donnée en constante,
 * elle permet également de manger des pommes dont les positions sont fixes.
 * La touche d'arrêt du jeu est fonctionnelle et la condition de victoire aussi.
 * Les conditions avec les bordures sont activées
 * et les collisions entre la tête et le corps du serpent sont activées aussi.
 * Des issues sont créées dans le plateau au milieu de chaque bordure,
 * si les serpents emprunte une de ces issues, il se téléporte à celle opposée.
 * Des pavés sont placés à des coordonnées bien précises, leur taille est donnée en constantes.
 * Le serpent qui gagne est le serpent qui a manger les 10 pomme le plus rapidement.
 * En cas de victoire, le nombre de déplacements unitaire réalisé par chacun des serpents est afficher.
 *
 */


/* Fichiers inclus */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>


// taille du serpent
#define TAILLE 10
// dimensions du plateau
#define LARGEUR_PLATEAU 80	
#define HAUTEUR_PLATEAU 40
// position initiale de la tête des serpent
#define X_INITIAL 40
#define Y_INITIAL_SERPENT_1 13
#define Y_INITIAL_SERPENT_2 27
// nombre de pommes à manger pour gagner
#define NB_POMMES 10
// caractéristiques constantes des pavés (nombre et taille)
#define NB_PAVES 6
#define TAILLE_PAVES 5
// temporisation entre deux déplacements des serpents, 200 000 microseconces
#define ATTENTE 2000
// caractères pour représenter les serpents
#define CORPS 'X'
#define TETE_SERPENT_1 '1'
#define TETE_SERPENT_2 '2'
// touches de direction ou d'arrêt du jeu
#define HAUT 1
#define BAS 2
#define GAUCHE 3
#define DROITE 4
#define CHEMIN_POMME 5
#define STOP 'a'
// caractères pour les éléments du plateau
#define BORDURE '#'
#define VIDE ' '
#define POMME '6'
// coordonnées X et Y des issues
#define ISSUE_HAUT_X 40
#define ISSUE_HAUT_Y 0
#define ISSUE_BAS_X 40
#define ISSUE_BAS_Y 40
#define ISSUE_GAUCHE_X 0
#define ISSUE_GAUCHE_Y 20
#define ISSUE_DROITE_X 80
#define ISSUE_DROITE_Y 20

// définition des positions X et Y des pommes dans un tableau
// et des positions des coins supérieurs gauches des pavés dans un tableau
int lesPommesX[NB_POMMES] = {40, 75, 78, 2, 9, 78, 74, 2, 72, 5};
int lesPommesY[NB_POMMES] = {20, 38, 2, 2, 5, 38, 32, 38, 32, 2};
int lesPavesX[NB_PAVES] = { 4, 73, 4, 73, 38, 38};
int lesPavesY[NB_PAVES] = { 4, 4, 33, 33, 14, 22};


// définition d'un type pour le plateau
// Attention, pour que les indices du tableau 2D (qui commencent à 0) coincident
// avec les coordonées à l'écran (qui commencent à 1), on ajoute 1 aux dimensions
// et on neutralise la ligne 0 et la colonne 0 du tableau 2D (elles ne sont jamais
// utilisées)
typedef char tPlateau[LARGEUR_PLATEAU + 1][HAUTEUR_PLATEAU + 1];

// déclaration des compteurs de pommes pour le serpent 1, pour le serpent 2
int nbPommes1 = 0;
int nbPommes2 = 0;

// Total des déplacements des 2 serpents
int deplacements1 = 0;
int deplacements2 = 0;

/* Déclaration des fonctions et procédures*/
// Fonctions plateau, pommes et pavés
void initPlateau(tPlateau plateau, int lesX[], int lesY[]);
void dessinerPlateau(tPlateau plateau);
void ajouterPomme(tPlateau plateau, int iPomme);
void placerPaves(tPlateau plateau);
void afficher(int, int, char);
void effacer(int x, int y);
// Fonctions relatives au serpent 1
void dessinerSerpent1(int lesX1[], int lesY1[]);
void directionSerpent1(int lesX1[], int lesY1[], tPlateau plateau, char *direction1, int x, int y, int lesX2[], int lesY2[], char directionSerpent2);
bool verifierCollisionProchainDeplacement1(int lesX1[], int lesY1[], tPlateau plateau, char prochaineDirection1, int lesX2[], int lesY2[], char directionSerpent2);
int calculDistance1(int lesX1[], int lesY1[], int pommeX, int pommeY);
void progresser1(int lesX1[], int lesY1[], char direction1, tPlateau plateau, bool *collision1, bool *pommeMangee1, bool *utiliserIssue1);
// Fonctions relatives au serpent 2
void dessinerSerpent2(int lesX2[], int lesY2[]);
void directionSerpent2(int lesX2[], int lesY2[], tPlateau plateau, char *direction2, int objectifX, int objectifY, int lesX1[], int lesY1[], char directionSerpent2);
bool verifierCollisionProchainDeplacement2(int lesX2[], int lesY2[], tPlateau plateau, char prochaineDirection2, int lesX1[], int lesY1[], char directionSerpent1);
int calculDistance2(int lesX2[], int lesY2[], int pommeX, int pommeY);
void progresser2(int lesX2[], int lesY2[], char direction2, tPlateau plateau, bool *collision2, bool *pommeMangee2, bool *utiliserIssue2);
// Fonctions boites noires
void gotoxy(int x, int y);
int kbhit();
void disable_echo();
void enable_echo();

// Main du programme
int main()
{
	// tableaux contentant les coordonnées X et Y des éléments des 2 serpents
	int lesX1[TAILLE];
	int lesY1[TAILLE];
	int lesX2[TAILLE];
	int lesY2[TAILLE];

	// représente la touche frappée par l'utilisateur attend la possible entrée de la touche arrêt
	char touche;

	// direction courante des serpents (HAUT, BAS, GAUCHE ou DROITE)
	char direction1;
	char direction2;

	// le plateau de jeu
	tPlateau lePlateau;

	//variables utiles aux interactions des serpents avec le pateau
	bool collision1 = false;
	bool collision2 = false;
	bool gagne = false;
	bool pommeMangee1 = false;
	bool pommeMangee2 = false;
	bool utiliserIssue1 = false;
	bool utiliserIssue2 = false;

	// initialisation de la position du serpent 1: positionnement de la
	// tête en (X_INITIAL, Y_INITIAL_SERPENT_1), puis des anneaux à sa gauche
	for (int i = 0 ; i < TAILLE ; i++)
	{
		lesX1[i] = X_INITIAL - i;
		lesY1[i] = Y_INITIAL_SERPENT_1;
	}
	// initialisation de la position du serpent 2: positionnement de la
	// tête en (X_INITIAL, Y_INITIAL_SERPENT_2), puis des anneaux à sa droite
	for (int i = 0 ; i < TAILLE ; i++)
	{
		lesX2[i] = X_INITIAL + i;
		lesY2[i] = Y_INITIAL_SERPENT_2;
	}

	// mise en place du plateau
	initPlateau(lePlateau, lesX1, lesY1);
	system("clear");
	dessinerPlateau(lePlateau);
	srand(time(NULL));
	ajouterPomme(lePlateau, (nbPommes1+nbPommes2));

	// initialisation : le serpent 1 se dirige vers la DROITE
	// initialisation : le serpent 2 se dirige vers la GAUCHE
	dessinerSerpent1(lesX1, lesY1);
	dessinerSerpent2(lesX2, lesY2);
	disable_echo();
	direction1 = DROITE;
	direction2 = GAUCHE;

	// calcul la meilleur distance à l'initialisation
	int meilleurDistance1 = calculDistance1(lesX1, lesY1, lesPommesX[(nbPommes1+nbPommes2)], lesPommesY[(nbPommes1+nbPommes2)]);
	int meilleurDistance2 = calculDistance2(lesX2, lesY2, lesPommesX[(nbPommes1+nbPommes2)], lesPommesY[(nbPommes1+nbPommes2)]);

	// boucle de jeu du serpent 1. Arret si touche STOP, si collision avec une bordure ou
	// si toutes les pommes sont mangées
	do
	{
		/* déplacements du serpent 1*/
		if (meilleurDistance1 == HAUT) // se dirige vers le trou du haut puis quand il s'est téléporté avance vers la pomme
		{
			if (utiliserIssue1)
			{
				directionSerpent1(lesX1, lesY1, lePlateau, &direction1, lesPommesX[(nbPommes1+nbPommes2)], lesPommesY[(nbPommes1+nbPommes2)], lesX2, lesY2, direction2);
			}
			else
			{
				directionSerpent1(lesX1, lesY1, lePlateau, &direction1, ISSUE_HAUT_X, ISSUE_HAUT_Y, lesX2, lesY2, direction2);
			}
		}
		if (meilleurDistance1 == BAS) // se dirige vers le trou du bas puis quand il s'est téléporté avance vers la pomme
		{
			if (utiliserIssue1)
			{
				directionSerpent1(lesX1, lesY1, lePlateau, &direction1, lesPommesX[(nbPommes1+nbPommes2)], lesPommesY[(nbPommes1+nbPommes2)], lesX2, lesY2, direction2);
			}
			else
			{
				directionSerpent1(lesX1, lesY1, lePlateau, &direction1, ISSUE_BAS_X, ISSUE_BAS_Y, lesX2, lesY2, direction2);
			}
		}
		if (meilleurDistance1 == GAUCHE) // se dirige vers le trou de gauche puis quand il s'est téléporté avance vers la pomme
		{
			if (utiliserIssue1)
			{
				directionSerpent1(lesX1, lesY1, lePlateau, &direction1, lesPommesX[(nbPommes1+nbPommes2)], lesPommesY[(nbPommes1+nbPommes2)], lesX2, lesY2, direction2);
			}
			else
			{
				directionSerpent1(lesX1, lesY1, lePlateau, &direction1, ISSUE_GAUCHE_X, ISSUE_GAUCHE_Y, lesX2, lesY2, direction2);
			}
		}
		if (meilleurDistance1 == DROITE) // se dirige vers le trou de droite puis quand il s'est téléporté avance vers la pomme
		{
			if (utiliserIssue1)
			{
				directionSerpent1(lesX1, lesY1, lePlateau, &direction1, lesPommesX[(nbPommes1+nbPommes2)], lesPommesY[(nbPommes1+nbPommes2)], lesX2, lesY2, direction2);
			}
			else
			{
				directionSerpent1(lesX1, lesY1, lePlateau, &direction1, ISSUE_DROITE_X, ISSUE_DROITE_Y, lesX2, lesY2, direction2);
			}
		}
		if (meilleurDistance1 == CHEMIN_POMME) // sinon se dirige uniquement vers la pomme
		{
			directionSerpent1(lesX1, lesY1, lePlateau, &direction1, lesPommesX[(nbPommes1+nbPommes2)], lesPommesY[(nbPommes1+nbPommes2)], lesX2, lesY2, direction2);
		}

		/* déplacements du serpent 2*/
		if (meilleurDistance2 == HAUT) // se dirige vers le trou du haut puis quand il s'est téléporté avance vers la pomme
		{
			if (utiliserIssue2)
			{
				directionSerpent2(lesX2, lesY2, lePlateau, &direction2, lesPommesX[(nbPommes1+nbPommes2)], lesPommesY[(nbPommes1+nbPommes2)], lesX1, lesY1, direction2);
			}
			else
			{
				directionSerpent2(lesX2, lesY2, lePlateau, &direction2, ISSUE_HAUT_X, ISSUE_HAUT_Y, lesX1, lesY1, direction2);
			}
		}
		if (meilleurDistance2 == BAS) // se dirige vers le trou du bas puis quand il s'est téléporté avance vers la pomme
		{
			if (utiliserIssue2)
			{
				directionSerpent2(lesX2, lesY2, lePlateau, &direction2, lesPommesX[(nbPommes1+nbPommes2)], lesPommesY[(nbPommes1+nbPommes2)], lesX1, lesY1, direction2);
			}
			else
			{
				directionSerpent2(lesX2, lesY2, lePlateau, &direction2, ISSUE_BAS_X, ISSUE_BAS_Y, lesX1, lesY1, direction2);
			}
		}
		if (meilleurDistance2 == GAUCHE) // se dirige vers le trou de gauche puis quand il s'est téléporté avance vers la pomme
		{
			if (utiliserIssue2)
			{
				directionSerpent2(lesX2, lesY2, lePlateau, &direction2, lesPommesX[(nbPommes1+nbPommes2)], lesPommesY[(nbPommes1+nbPommes2)], lesX1, lesY1, direction2);
			}
			else
			{
				directionSerpent2(lesX2, lesY2, lePlateau, &direction2, ISSUE_GAUCHE_X, ISSUE_GAUCHE_Y, lesX1, lesY1, direction2);
			}
		}
		if (meilleurDistance2 == DROITE) // se dirige vers le trou de droite puis quand il s'est téléporté avance vers la pomme
		{
			if (utiliserIssue2)
			{
				directionSerpent2(lesX2, lesY2, lePlateau, &direction2, lesPommesX[(nbPommes1+nbPommes2)], lesPommesY[(nbPommes1+nbPommes2)], lesX1, lesY1, direction2);
			}
			else
			{
				directionSerpent2(lesX2, lesY2, lePlateau, &direction2, ISSUE_DROITE_X, ISSUE_DROITE_Y, lesX1, lesY1, direction2);
			}
		}
		if (meilleurDistance2 == CHEMIN_POMME) // sinon se dirige uniquement vers la pomme
		{
			directionSerpent2(lesX2, lesY2, lePlateau, &direction2, lesPommesX[(nbPommes1+nbPommes2)], lesPommesY[(nbPommes1+nbPommes2)], lesX1, lesY1, direction2);
		}

		// deplacement du serpent à chaque fois et incrémentation du compteur de déplacements
		progresser1(lesX1, lesY1, direction1, lePlateau, &collision1, &pommeMangee1, &utiliserIssue1);
		deplacements1++; /* ajout d'un déplacement pour le serpent 1*/
		progresser2(lesX2, lesY2, direction2, lePlateau, &collision2, &pommeMangee2, &utiliserIssue2);
		deplacements2++; /* ajout d'un déplacement pour le serpent 2*/


		// Ajoute une pomme au compteur de pomme quand elle est mangée et arrete le jeu si score atteint 10
		if (pommeMangee1)
		{
			nbPommes1++;
			if ((nbPommes1+nbPommes2) == NB_POMMES) {
				gagne = true;
				utiliserIssue1 = false;
			}
			if (!gagne)
			{
				ajouterPomme(lePlateau, (nbPommes1+nbPommes2));
				// recalcul la meilleure position après l'apparition d'une nouvelle pomme
				meilleurDistance1 = calculDistance1(lesX1, lesY1, lesPommesX[(nbPommes1+nbPommes2)], lesPommesY[(nbPommes1+nbPommes2)]); 
				pommeMangee1 = false;
			}
		}
		else if (pommeMangee2){
			nbPommes2++;
			if ((nbPommes1+nbPommes2) == NB_POMMES) {
				gagne = true;
				utiliserIssue2 = false;
			}
			if (!gagne)
			{
				ajouterPomme(lePlateau, (nbPommes1+nbPommes2));
				// recalcul la meilleure position après l'apparition d'une nouvelle pomme
				meilleurDistance2 = calculDistance2(lesX2, lesY2, lesPommesX[(nbPommes1+nbPommes2)], lesPommesY[(nbPommes1+nbPommes2)]); 
				pommeMangee2 = false;
			}
		}

		if (!gagne)
		{
			if (!collision1 && !collision2)
			{
				usleep(ATTENTE);
				if (kbhit() == 1)
				{
					touche = getchar();
				}
			}
		}
	} while (touche != STOP && !collision1 && !collision2 && !gagne); 

	enable_echo();
	gotoxy(1, HAUTEUR_PLATEAU + 1);

	// afficher les performances du programme
	printf("Serpent 1 : %d déplacements et %d pommes mangées\n", deplacements1, nbPommes1);
	printf("Serpent 2 : %d déplacements et %d pommes mangées\n", deplacements2, nbPommes2);

	return EXIT_SUCCESS;
}


/************************************************
		FONCTIONS ET PROCEDURES DU CODE 	   


*************************************************/
void initPlateau(tPlateau plateau, int lesX[], int lesY[])
{
	// initialisation du plateau avec des espaces
	for (int i = 1 ; i <= LARGEUR_PLATEAU ; i++)
	{
		for (int j = 1 ; j <= HAUTEUR_PLATEAU ; j++)
		{
			plateau[i][j] = VIDE;
		}
	}
	// Mise en place la bordure autour du plateau
	// première ligne
	for (int i = 1 ; i <= LARGEUR_PLATEAU ; i++)
	{
		plateau[i][1] = BORDURE;
		plateau[LARGEUR_PLATEAU / 2][1] = VIDE; // trou du haut
	}
	// lignes intermédiaires
	for (int j = 1 ; j <= HAUTEUR_PLATEAU ; j++)
	{
		plateau[1][j] = BORDURE;
		plateau[1][HAUTEUR_PLATEAU / 2] = VIDE; // trou de gauche
		plateau[LARGEUR_PLATEAU][j] = BORDURE;
		plateau[LARGEUR_PLATEAU][HAUTEUR_PLATEAU / 2] = VIDE; // trou de droite
	}
	// dernière ligne
	for (int i = 1 ; i <= LARGEUR_PLATEAU ; i++)
	{
		plateau[i][HAUTEUR_PLATEAU] = BORDURE;
		plateau[LARGEUR_PLATEAU / 2][HAUTEUR_PLATEAU] = VIDE; // trou du bas
	}
	//  place les pavés sur le plateau
    placerPaves(plateau);
}

void placerPaves(tPlateau plateau) {
    for (int i = 0; i < NB_PAVES; i++) {
        for (int x = lesPavesX[i]; x < lesPavesX[i] + TAILLE_PAVES; x++) {
            for (int y = lesPavesY[i]; y < lesPavesY[i] + TAILLE_PAVES; y++) {
                // Vérifier si x et y sont dans les limites du plateau
                if (x >= 1 && x <= LARGEUR_PLATEAU && y >= 1 && y <= HAUTEUR_PLATEAU) {
                    plateau[x][y] = BORDURE;
                }
            }
        }
    }
}

void dessinerPlateau(tPlateau plateau)
{
	int i, j;
	// affiche à l'écran le contenu du tableau 2D représentant le plateau
	for (i = 1 ; i <= LARGEUR_PLATEAU ; i++)
	{
		for (j = 1 ; j <= HAUTEUR_PLATEAU ; j++)
		{
			afficher(i, j, plateau[i][j]);
		}
	}
}

void ajouterPomme(tPlateau plateau, int iPomme)
{
	// génère aléatoirement la position d'une pomme,
	// vérifie que ça correspond à une case vide
	// du plateau puis l'ajoute au plateau et l'affiche
	int xPomme, yPomme;
	do
	{
		xPomme = lesPommesX[iPomme];
		yPomme = lesPommesY[iPomme];
	} while (plateau[xPomme][yPomme] != ' ');
	plateau[xPomme][yPomme] = POMME;
	afficher(xPomme, yPomme, POMME);
}

void afficher(int x, int y, char car)
{
	gotoxy(x, y);
	printf("%c", car);
	gotoxy(1, 1);
}

void effacer(int x, int y)
{
	gotoxy(x, y);
	printf(" ");
	gotoxy(1, 1);
}

/************************************************
	   FONCTIONS ET PROCEDURES DU SERPENT 1	    
*************************************************/
void dessinerSerpent1(int lesX1[], int lesY1[])
{
	int i;
	// affiche les anneaux puis la tête
	for(i = 1 ; i < TAILLE ; i++)
	{
		afficher(lesX1[i], lesY1[i], CORPS);
	}
	afficher(lesX1[0], lesY1[0], TETE_SERPENT_1);
}

void directionSerpent1(int lesX1[], int lesY1[], tPlateau plateau, char *direction1, int x, int y, int lesX2[], int lesY2[], char directionSerpent2)
{
	// Calcul des directions possibles
	int differenceX = x - lesX1[0]; // Différence en X
	int differenceY = y - lesY1[0]; // Différence en Y

	// Essayer de se déplacer dans la direction verticale
	if (differenceY != 0)
	{
		*direction1 = (differenceY > 0) ? BAS : HAUT;
		if (verifierCollisionProchainDeplacement1(lesX1, lesY1, plateau, *direction1, lesX2, lesY2, directionSerpent2))
		{
			// Si collision, essayer la direction horizontale
			*direction1 = (differenceX > 0) ? DROITE : GAUCHE;
			if (verifierCollisionProchainDeplacement1(lesX1, lesY1, plateau, *direction1, lesX2, lesY2, directionSerpent2))
			{
				// Si collision, essayer l'autre direction horizontale
				*direction1 = (differenceX > 0) ? GAUCHE : DROITE;
				if (verifierCollisionProchainDeplacement1(lesX1, lesY1, plateau, *direction1, lesX2, lesY2, directionSerpent2))
				{
					// Si collision, essayer l'autre direction verticale
					*direction1 = (differenceY > 0) ? HAUT : BAS;
				}
			}
		}
	}
	
	// Si pas de déplacement horizontal possible, essayer horizontale
	else if (differenceX != 0)
	{
		*direction1 = (differenceX > 0) ? DROITE : GAUCHE;
		if (verifierCollisionProchainDeplacement1(lesX1, lesY1, plateau, *direction1, lesX2, lesY2, directionSerpent2))
		{
			// Si collision, essayer la direction verticale
			*direction1 = (differenceY > 0) ? BAS : HAUT;
			if (verifierCollisionProchainDeplacement1(lesX1, lesY1, plateau, *direction1, lesX2, lesY2, directionSerpent2))
			{
				// Si collision, essayer l'autre direction verticale
				*direction1 = (differenceY > 0) ? HAUT : BAS;
				if (verifierCollisionProchainDeplacement1(lesX1, lesY1, plateau, *direction1, lesX2, lesY2, directionSerpent2))
				{
					// Si collision, essayer l'autre direction horizontale
					*direction1 = (differenceX > 0) ? GAUCHE : DROITE;
				}
			}
		}
	}
}

int calculDistance1(int lesX1[], int lesY1[], int pommeX, int pommeY)
{
    // définition des variables
    int passageTrouGauche, passageTrouDroit, passageTrouHaut, passageTrouBas, passageDirect;

    // Calcul des distances
    passageTrouGauche = abs(lesX1[0] - ISSUE_GAUCHE_X) + abs(lesY1[0] - ISSUE_GAUCHE_Y) + 
        abs(pommeX - ISSUE_DROITE_X) + abs(pommeY - ISSUE_DROITE_Y);
    passageTrouDroit = abs(lesX1[0] - ISSUE_DROITE_X) + abs(lesY1[0] - ISSUE_DROITE_Y) + 
        abs(pommeX - ISSUE_GAUCHE_X) + abs(pommeY - ISSUE_GAUCHE_Y);
    passageTrouHaut = abs(lesX1[0] - ISSUE_HAUT_X) + abs(lesY1[0] - ISSUE_HAUT_Y) + 
        abs(pommeX - ISSUE_BAS_X) + abs(pommeY - ISSUE_BAS_Y);
    passageTrouBas = abs(lesX1[0] - ISSUE_BAS_X) + abs(lesY1[0] - ISSUE_BAS_Y) + 
        abs(pommeX - ISSUE_HAUT_X) + abs(pommeY - ISSUE_HAUT_Y);
    passageDirect = abs(lesX1[0] - pommeX) + abs(lesY1[0] - pommeY);

    // compare les résultats pour obtenir le meilleur chemin
    // chemin direct vers la pomme sans passer dans un trou
    if (passageDirect <= passageTrouHaut && passageDirect <= passageTrouBas &&
        passageDirect <= passageTrouGauche && passageDirect <= passageTrouDroit)
    {
        return CHEMIN_POMME;
    }
    // Privilégie d'abord les passages verticaux
    else if (passageTrouHaut <= passageTrouBas)
    {
        return HAUT;
    }
    else if (passageTrouBas <= passageTrouGauche && passageTrouBas <= passageTrouDroit)
    {
        return BAS;
    }
    else if (passageTrouGauche <= passageTrouDroit)
    {
        return GAUCHE;
    }
    else
    {
        return DROITE;
    }
}

bool verifierCollisionProchainDeplacement1(int lesX1[], int lesY1[], tPlateau plateau, char prochaineDirection1, int lesX2[], int lesY2[], char directionSerpent2)
{
    int nouvelleX = lesX1[0];
    int nouvelleY = lesY1[0];

    // Calcul de la nouvelle position
    switch (prochaineDirection1)
    {
        case HAUT:
            nouvelleY--;
            break;
        case BAS:
            nouvelleY++;
            break;
        case GAUCHE:
            nouvelleX--;
            break;
        case DROITE:
            nouvelleX++;
            break;
    }

    // Collision avec les bordures
    if (plateau[nouvelleX][nouvelleY] == BORDURE)
    {
        return true;
    }

    // Collision tête contre tête avec le serpent 2
    if (nouvelleX == lesX2[0] && nouvelleY == lesY2[0])
    {
        // Si les serpents sont à égale distance de la pomme, 
        // le serpent 2 a la priorité (le serpent 1 doit éviter)
        if (plateau[lesX2[0]][lesY2[0]] == POMME)
        {
            return true;
        }
        return true;
    }

    // Prédiction de la prochaine position du serpent 2
    int prochaineX2 = lesX2[0];
    int prochaineY2 = lesY2[0];
    switch (directionSerpent2) {
        case HAUT:
            prochaineY2--;
            break;
        case BAS:
            prochaineY2++;
            break;
        case GAUCHE:
            prochaineX2--;
            break;
        case DROITE:
            prochaineX2++;
            break;
    }

    // Si la nouvelle position va créer une collision, éviter
    if (nouvelleX == prochaineX2 && nouvelleY == prochaineY2)
    {
        return true;
    }

    // Collision avec le corps des deux serpents
    for (int i = 0; i < TAILLE; i++)
    {
        if ((lesX1[i] == nouvelleX && lesY1[i] == nouvelleY) || 
            (lesX2[i] == nouvelleX && lesY2[i] == nouvelleY))
        {
            return true;
        }
    }

    return false;
}

void progresser1(int lesX1[], int lesY1[], char direction1, tPlateau plateau, bool *collision1, bool *pommeMangee1, bool *utiliserIssue1)
{
    // Effacer l'ancienne position du serpent dans le plateau
    plateau[lesX1[TAILLE - 1]][lesY1[TAILLE - 1]] = VIDE;
    for (int i = 0; i < TAILLE; i++) {
        plateau[lesX1[i]][lesY1[i]] = VIDE;
    }
    
    // Effacer le dernier élément à l'écran
    effacer(lesX1[TAILLE - 1], lesY1[TAILLE - 1]);

    // Mettre à jour les positions
    for (int i = TAILLE - 1; i > 0; i--)
    {
        lesX1[i] = lesX1[i - 1];
        lesY1[i] = lesY1[i - 1];
    }
    
    // Faire progresser la tête dans la nouvelle direction
    switch (direction1)
    {
        case HAUT:
            lesY1[0] = lesY1[0] - 1;
            break;
        case BAS:
            lesY1[0] = lesY1[0] + 1;
            break;
        case DROITE:
            lesX1[0] = lesX1[0] + 1;
            break;
        case GAUCHE:
            lesX1[0] = lesX1[0] - 1;
            break;
    }

    // Gestion des passages par les issues
    if (lesX1[0] <= 0)
    {
        lesX1[0] = LARGEUR_PLATEAU;
        *utiliserIssue1 = true;
    }
    else if (lesX1[0] > LARGEUR_PLATEAU)
    {
        lesX1[0] = 1;
        *utiliserIssue1 = true;
    }
    else if (lesY1[0] <= 0)
    {
        lesY1[0] = HAUTEUR_PLATEAU;
        *utiliserIssue1 = true;
    }
    else if (lesY1[0] > HAUTEUR_PLATEAU)
    {
        lesY1[0] = 1;
        *utiliserIssue1 = true;
    }

    *pommeMangee1 = false;
    // Vérification des collisions et mise à jour du plateau
    if (plateau[lesX1[0]][lesY1[0]] == POMME)
    {
        *pommeMangee1 = true;
        plateau[lesX1[0]][lesY1[0]] = VIDE;
    }
    else if (plateau[lesX1[0]][lesY1[0]] == BORDURE)
    {
        *collision1 = true;
    }
    else if (plateau[lesX1[0]][lesY1[0]] == TETE_SERPENT_2 || plateau[lesX1[0]][lesY1[0]] == CORPS)
    {
        *collision1 = true;
    }

    // Mise à jour du plateau avec les nouvelles positions
    plateau[lesX1[0]][lesY1[0]] = TETE_SERPENT_1;
    for (int i = 1; i < TAILLE; i++) {
        plateau[lesX1[i]][lesY1[i]] = CORPS;
    }

    // Dessiner le serpent à l'écran
    dessinerSerpent1(lesX1, lesY1);
}
/************************************************
	   FONCTIONS ET PROCEDURES DU SERPENT 2	    
*************************************************/
void dessinerSerpent2(int lesX2[], int lesY2[])
{
	int i;
	// affiche les anneaux puis la tête
	for(i = 1 ; i < TAILLE ; i++)
	{
		afficher(lesX2[i], lesY2[i], CORPS);
	}
	afficher(lesX2[0], lesY2[0], TETE_SERPENT_2);
}

void directionSerpent2(int lesX2[], int lesY2[], tPlateau plateau, char *direction2, int x, int y, int lesX1[], int lesY1[], char directionSerpent1)
{
    // Calcul des directions possibles
    int differenceX = x - lesX2[0]; // Différence en X
    int differenceY = y - lesY2[0]; // Différence en Y
    
    // Tableau pour stocker les directions possibles sans collision
    char directionsValides[4] = {HAUT, BAS, GAUCHE, DROITE};
    bool directionValide[4] = {true, true, true, true}; // indique si la direction est valide
    int nbDirectionsValides = 4;
    
    // Vérifier chaque direction possible
    for(int i = 0; i < 4; i++) {
        if(verifierCollisionProchainDeplacement2(lesX2, lesY2, plateau, directionsValides[i], lesX1, lesY1, directionSerpent1)) {
            directionValide[i] = false;
            nbDirectionsValides--;
        }
    }
    
    // Si au moins une direction est valide
    if(nbDirectionsValides > 0) {
        // Priorité à la direction qui rapproche de l'objectif
        if(abs(differenceX) >= abs(differenceY)) {
            // Essayer d'abord horizontal
            if(differenceX > 0 && directionValide[3]) { // DROITE
                *direction2 = DROITE;
            } else if(differenceX < 0 && directionValide[2]) { // GAUCHE
                *direction2 = GAUCHE;
            }
            // Si horizontal impossible, essayer vertical
            else if(differenceY > 0 && directionValide[1]) { // BAS
                *direction2 = BAS;
            } else if(differenceY < 0 && directionValide[0]) { // HAUT
                *direction2 = HAUT;
            }
            // Sinon prendre la première direction valide
            else {
                for(int i = 0; i < 4; i++) {
                    if(directionValide[i]) {
                        *direction2 = directionsValides[i];
                        break;
                    }
                }
            }
        } else {
            // Même quand la différence Y est plus grande, essayer d'abord horizontal
            if(differenceX > 0 && directionValide[3]) { // DROITE
                *direction2 = DROITE;
            } else if(differenceX < 0 && directionValide[2]) { // GAUCHE
                *direction2 = GAUCHE;
            }
            // Si horizontal impossible, essayer vertical
            else if(differenceY > 0 && directionValide[1]) { // BAS
                *direction2 = BAS;
            } else if(differenceY < 0 && directionValide[0]) { // HAUT
                *direction2 = HAUT;
            }
            // Sinon prendre la première direction valide
            else {
                for(int i = 0; i < 4; i++) {
                    if(directionValide[i]) {
                        *direction2 = directionsValides[i];
                        break;
                    }
                }
            }
        }
    }
    // Si aucune direction n'est valide, on garde la direction actuelle
}

int calculDistance2(int lesX2[], int lesY2[], int pommeX, int pommeY)
{
    // définition des variables
    int passageTrouGauche, passageTrouDroit, passageTrouHaut, passageTrouBas, passageDirect;

    // Calcul des distances
    passageTrouGauche = abs(lesX2[0] - ISSUE_GAUCHE_X) + abs(lesY2[0] - ISSUE_GAUCHE_Y) + 
        abs(pommeX - ISSUE_DROITE_X) + abs(pommeY - ISSUE_DROITE_Y);
    passageTrouDroit = abs(lesX2[0] - ISSUE_DROITE_X) + abs(lesY2[0] - ISSUE_DROITE_Y) + 
        abs(pommeX - ISSUE_GAUCHE_X) + abs(pommeY - ISSUE_GAUCHE_Y);
    passageTrouHaut = abs(lesX2[0] - ISSUE_HAUT_X) + abs(lesY2[0] - ISSUE_HAUT_Y) + 
        abs(pommeX - ISSUE_BAS_X) + abs(pommeY - ISSUE_BAS_Y);
    passageTrouBas = abs(lesX2[0] - ISSUE_BAS_X) + abs(lesY2[0] - ISSUE_BAS_Y) + 
        abs(pommeX - ISSUE_HAUT_X) + abs(pommeY - ISSUE_HAUT_Y);
    passageDirect = abs(lesX2[0] - pommeX) + abs(lesY2[0] - pommeY);


    // compare les résultats pour obtenir le meilleur chemin
    // chemin direct vers la pomme sans passer dans un trou
    if (passageDirect < passageTrouHaut && passageDirect < passageTrouBas &&
        passageDirect < passageTrouGauche && passageDirect < passageTrouDroit)
    {
        return CHEMIN_POMME;
    }
    // Privilégie d'abord les passages horizontaux
    else if (passageTrouGauche <= passageTrouDroit)
    {
        return GAUCHE;
    }
    else if (passageTrouDroit <= passageTrouHaut && passageTrouDroit <= passageTrouBas)
    {
        return DROITE;
    }
    else if (passageTrouHaut <= passageTrouBas)
    {
        return HAUT;
    }
    else
    {
        return BAS;
    }
}

bool verifierCollisionProchainDeplacement2(int lesX2[], int lesY2[], tPlateau plateau, char prochaineDirection2, int lesX1[], int lesY1[], char directionSerpent1)
{
    int nouvelleX = lesX2[0];
    int nouvelleY = lesY2[0];

    // Calcul de la nouvelle position
    switch (prochaineDirection2)
    {
        case HAUT:
            nouvelleY--;
            break;
        case BAS:
            nouvelleY++;
            break;
        case GAUCHE:
            nouvelleX--;
            break;
        case DROITE:
            nouvelleX++;
            break;
    }

    // Prédiction de la prochaine position du serpent 1
    int prochaineX1 = lesX1[0];
    int prochaineY1 = lesY1[0];
    switch (directionSerpent1) {
        case HAUT:
            prochaineY1--;
            break;
        case BAS:
            prochaineY1++;
            break;
        case GAUCHE:
            prochaineX1--;
            break;
        case DROITE:
            prochaineX1++;
            break;
    }

    // Si la nouvelle position va créer une collision, éviter
    if (nouvelleX == prochaineX1 && nouvelleY == prochaineY1)
    {
        return true;
    }

    // Collision avec les bordures
    if (plateau[nouvelleX][nouvelleY] == BORDURE)
    {
        return true;
    }

    // Collision tête contre tête avec le serpent 1
    if (nouvelleX == lesX1[0] && nouvelleY == lesY1[0])
    {
        return true;
    }

    // Collision avec le corps des deux serpents
    for (int i = 0; i < TAILLE; i++)
    {
        if ((lesX2[i] == nouvelleX && lesY2[i] == nouvelleY) || 
            (lesX1[i] == nouvelleX && lesY1[i] == nouvelleY))
        {
            return true;
        }
    }

    return false;
}

void progresser2(int lesX2[], int lesY2[], char direction2, tPlateau plateau, bool *collision2, bool *pommeMangee2, bool *utiliserIssue2)
{
    // Effacer l'ancienne position du serpent dans le plateau
    plateau[lesX2[TAILLE - 1]][lesY2[TAILLE - 1]] = VIDE;
    for (int i = 0; i < TAILLE; i++) {
        plateau[lesX2[i]][lesY2[i]] = VIDE;
    }
    
    // Effacer le dernier élément à l'écran
    effacer(lesX2[TAILLE - 1], lesY2[TAILLE - 1]);

    // Mettre à jour les positions
    for (int i = TAILLE - 1; i > 0; i--)
    {
        lesX2[i] = lesX2[i - 1];
        lesY2[i] = lesY2[i - 1];
    }
    
    // Faire progresser la tête dans la nouvelle direction
    switch (direction2)
    {
        case HAUT:
            lesY2[0] = lesY2[0] - 1;
            break;
        case BAS:
            lesY2[0] = lesY2[0] + 1;
            break;
        case DROITE:
            lesX2[0] = lesX2[0] + 1;
            break;
        case GAUCHE:
            lesX2[0] = lesX2[0] - 1;
            break;
    }

    // Gestion des passages par les issues
    if (lesX2[0] <= 0)
    {
        lesX2[0] = LARGEUR_PLATEAU;
        *utiliserIssue2 = true;
    }
    else if (lesX2[0] > LARGEUR_PLATEAU)
    {
        lesX2[0] = 1;
        *utiliserIssue2 = true;
    }
    else if (lesY2[0] <= 0)
    {
        lesY2[0] = HAUTEUR_PLATEAU;
        *utiliserIssue2 = true;
    }
    else if (lesY2[0] > HAUTEUR_PLATEAU)
    {
        lesY2[0] = 1;
        *utiliserIssue2 = true;
    }

    *pommeMangee2 = false;
    // Vérification des collisions et mise à jour du plateau
    if (plateau[lesX2[0]][lesY2[0]] == POMME)
    {
        *pommeMangee2 = true;
        plateau[lesX2[0]][lesY2[0]] = VIDE;
    }
    else if (plateau[lesX2[0]][lesY2[0]] == BORDURE)
    {
        *collision2 = true;
    }
    else if (plateau[lesX2[0]][lesY2[0]] == TETE_SERPENT_1 || plateau[lesX2[0]][lesY2[0]] == CORPS)
    {
        *collision2 = true;
    }

    // Mise à jour du plateau avec les nouvelles positions
    plateau[lesX2[0]][lesY2[0]] = TETE_SERPENT_2;
    for (int i = 1; i < TAILLE; i++) {
        plateau[lesX2[i]][lesY2[i]] = CORPS;
    }

    // Dessiner le serpent à l'écran
    dessinerSerpent2(lesX2, lesY2);
}

/************************************************
				 FONCTIONS UTILITAIRES 			
*************************************************/
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