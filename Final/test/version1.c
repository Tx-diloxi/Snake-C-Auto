/**
* @file version1.c
* @brief Jeu Snake version 4 pour le projet SAE 1.02.
*
* Ce fichier implémente la suite de la SAE 1.02 du jeu Shake. Le joueur contrôle un serpent 
* qui se déplace sur un plateau, mange des pommes qui sont positionnées à des coordonnées définit.
*
* @details
* - Déplacement avec les touches : 'z' (haut), 'q' (gauche), 's' (bas), 'd' (droite).
* - Une pomme apparaît sur le plateau aux coordonnés définit 
* - Le jeu se termine lorsque le serpent heurte un mur ou mange 10 pommes.
*
* @author
* LE SECH Marceau
* CAMARA Kémo
*
* @version 1.0
* @date 08/12/2024
*/


/* Fichiers inclus */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

#define TAILLE 10  // Taille du serpent (longueur initiale). Dans ce cas, il est de 10 unités.
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

#define PARSECONDE 1000000.0


typedef struct {
    int x;  // Coordonnée X du Portail.
    int y;  // Coordonnée Y du Portail.
} Portail;

const Portail TROU_HAUT = {40, 0};  // Portail en haut du plateau (Portail de coordonnées (40, 0)).
const Portail TROU_BAS = {40, 40};  // Portail en bas du plateau (Portail de coordonnées (40, 40)).
const Portail TROU_GAUCHE = {0, 20};  // Portail à gauche du plateau (Portail de coordonnées (0, 20)).
const Portail TROU_DROITE = {80, 20};  // Portail à droite du plateau (Portail de coordonnées (80, 20)).

typedef char tPlateau[LARGEUR_PLATEAU + 1][HAUTEUR_PLATEAU + 1];  // Initialiser le plateau de jeu.

const int lesPommesX[NB_POMMES] = {75, 75, 78, 2, 8, 78, 74, 2, 72, 5};  // Positions en X des pommes.
const int lesPommesY[NB_POMMES] = {8, 39, 2, 2, 5, 39, 33, 38, 35, 2};  // Positions en Y des pommes.

int NbPasSerpent = 0;  // Compteur du nombre de déplacements effectués par le serpent.
int NbPommesSerpentManger = 0;  // Compteur du nombre de pommes mangées par le serpent.


/* Déclaration des fonctions et procédures (les prototypes) */
void initPlateau(tPlateau plateau);
void dessinerPlateau(tPlateau plateau);
void ajouterPomme(tPlateau plateau);
void afficher(int, int, char);
void effacer(int x, int y);
void dessinerSerpent(int lesX[], int lesY[]);
void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool * collision, bool * pomme, bool *SerpentPasserPortail);
void gotoxy(int x, int y);
int kbhit();
void disable_echo();
void enable_echo();


int main()
{
	// 2 tableaux contenant les positions des éléments qui constituent le serpent
    int lesX[TAILLE];
	int lesY[TAILLE];

	// représente la touche frappée par l'utilisateur : touche de direction ou pour l'arrêt
	char touche;

	//direction courante du serpent (HAUT, BAS, GAUCHE ou DROITE)
	char direction;

	// le plateau de jeu
	tPlateau lePlateau;

	bool collision = false;
	bool gagne = false;
	bool pommeMangee = false;

	bool SerpentPasserPortail = false;
   
	// initialisation de la position du serpent : positionnement de la
	// tête en (X_DEPART_SERPENT, Y_DEPART_SERPENT), puis des anneaux à sa gauche
	for(int i=0 ; i<TAILLE ; i++)
	{
		lesX[i] = X_DEPART_SERPENT-i;
		lesY[i] = Y_DEPART_SERPENT;
	}

	// mise en place du plateau
	initPlateau(lePlateau);
	system("clear");
	dessinerPlateau(lePlateau);


	srand(time(NULL));
	ajouterPomme(lePlateau);

	// initialisation : le serpent se dirige vers la DROITE
	dessinerSerpent(lesX, lesY);
	disable_echo();
	direction = DROITE;
	touche = DROITE;

	// boucle de jeu. Arret si touche STOP, si collision avec une bordure ou
	// si toutes les pommes sont mangées
	time_t debut = clock();
	do {
		
		if ( (lesPommesX[NbPasSerpent] - lesX[0]) > 0)
		{
			direction = DROITE ;
		}
		else if ( (lesPommesX[NbPasSerpent] - lesX[0]) < 0)
		{
			direction = GAUCHE;
		}
		else if ( (lesPommesY[NbPasSerpent] - lesY[0]) > 0 )
		{
			direction = BAS ;
		}
		else
		{
			direction = HAUT;
		}
		


		progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee, &SerpentPasserPortail);
		if (pommeMangee)
		{
            NbPasSerpent++;
			gagne = (NbPasSerpent==NB_POMMES);
			if (!gagne)
			{
				ajouterPomme(lePlateau);
				pommeMangee = false;
			}	
			
		}
		if (!gagne)
		{
			if (!collision)
			{
				usleep(ATTENTE);
				if (kbhit()==1)
				{
					touche = getchar();
				}
			}
		}
	} while ( (touche != STOP) && !collision && !gagne);
    enable_echo();
	gotoxy(LARGEUR_PLATEAU+1, 1);
	if (gagne)
	{
		clock_t fin = clock();
		enable_echo();
		gotoxy(2, HAUTEUR_PLATEAU+1);
		printf("Compteur de pas : %d\n", NbPasSerpent);
        gotoxy(2,HAUTEUR_PLATEAU+2);
		printf("Temps écoulé : %f en seconde\n", difftime(fin, debut) / PARSECONDE );
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
}

void dessinerPlateau(tPlateau plateau)
{
    // Affiche à l'écran le contenu du tableau 2D représentant le plateau
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

void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool *collision, bool *pomme, bool *SerpentPasserPortail)
{
    // Efface le dernier élément (queue) du serpent avant de mettre à jour les autres segments
    effacer(lesX[TAILLE - 1], lesY[TAILLE - 1]);

    // Déplace chaque segment du serpent pour le faire progresser d'une case
    // Commence à partir de la fin du serpent et déplace chaque segment vers la position de celui qui est devant lui
    for (int i = TAILLE - 1; i > 0; i--)
    {
        lesX[i] = lesX[i - 1]; // Le segment i prend la position du segment i-1 en X
        lesY[i] = lesY[i - 1]; // Le segment i prend la position du segment i-1 en Y
    }

    // Déplace la tête du serpent dans la direction donnée
    switch (direction)
    {
    case HAUT:
        lesY[0]--; // Déplacement de la tête vers le haut
        break;
    case BAS:
        lesY[0]++; // Déplacement de la tête vers le bas
        break;
    case DROITE:
        lesX[0]++; // Déplacement de la tête vers la droite
        break;
    case GAUCHE:
        lesX[0]--; // Déplacement de la tête vers la gauche
        break;
    }

    // Gestion des téléportations du serpent via les portails
    if (lesX[0] <= 0)  // Si la tête du serpent dépasse la bordure gauche
    {
        lesX[0] = LARGEUR_PLATEAU; // La tête réapparaît à droite
        *SerpentPasserPortail = true; // Indique que le serpent a traversé un portail
    }
    else if (lesX[0] > LARGEUR_PLATEAU) // Si la tête dépasse la bordure droite
    {
        lesX[0] = 1; // La tête réapparaît à gauche
        *SerpentPasserPortail = true; // Indique que le serpent a traversé un portail
    }

    if (lesY[0] <= 0)  // Si la tête dépasse la bordure du haut
    {
        lesY[0] = HAUTEUR_PLATEAU; // La tête réapparaît en bas
        *SerpentPasserPortail = true; // Indique que le serpent a traversé un portail
    }
    else if (lesY[0] > HAUTEUR_PLATEAU) // Si la tête dépasse la bordure du bas
    {
        lesY[0] = 1; // La tête réapparaît en haut
        *SerpentPasserPortail = true; // Indique que le serpent a traversé un portail
    }

    // Initialisation de l'indicateur de pomme détectée
    *pomme = false;

    // Vérification si la tête du serpent a mangé une pomme
    if (plateau[lesX[0]][lesY[0]] == POMME)
    {
        *pomme = true; // La pomme est mangée
        plateau[lesX[0]][lesY[0]] = VIDE; // La pomme disparait du plateau
    }

    // Vérification d'une collision avec la bordure du plateau
    else if (plateau[lesX[0]][lesY[0]] == BORDURE)
    {
        *collision = true; // Collision détectée avec la bordure
    }

    // Redessine le serpent avec sa nouvelle position
    dessinerSerpent(lesX, lesY);

    // Incrémentation du compteur de pas du serpent
    NbPasSerpent++;
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

