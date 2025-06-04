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


// taille du serpent
#define TAILLE 10
// dimensions du plateau
#define LARGEUR_PLATEAU 80	
#define HAUTEUR_PLATEAU 40
// position initiale de la tête du serpent
#define X_DEPART_SERPENT 40
#define Y_DEPART_SEREPNT 20
// nombre de pommes à manger pour gagner
#define NB_POMMES 10
// temporisation entre deux déplacements du serpent (en microsecondes)
#define ATTENTE 60000
// caractères pour représenter le serpent
#define CORPS 'X'
#define TETE 'O'
// touches de direction ou d'arrêt du jeu
#define HAUT 'z'
#define BAS 's'
#define GAUCHE 'q'
#define DROITE 'd'
#define STOP 'a'
// caractères pour les éléments du plateau
#define BORDURE '#'
#define VIDE ' '
#define POMME '6'



// définition d'un type pour le plateau : tPlateau
// Attention, pour que les indices du tableau 2D (qui commencent à 0) coincident
// avec les coordonées à l'écran (qui commencent à 1), on ajoute 1 aux dimensions
// et on neutralise la ligne 0 et la colonne 0 du tableau 2D (elles ne sont jamais
// utilisées)
int COORDONNE_POMME_X[NB_POMMES] = {75, 75, 78, 2, 8, 78, 74,  2, 72, 5};
int COORDONNE_POMME_Y[NB_POMMES] = { 8, 39,  2, 2, 5, 39, 33, 38, 35, 2};
typedef char tPlateau[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1];
// compteur de pommes mangées
int NbPommesManger = 0;
int nbDepUnitaires = 0;

/* Déclaration des fonctions et procédures (les prototypes) */
void initPlateau(tPlateau plateau);
void dessinerPlateau(tPlateau plateau);
void ajouterPomme(tPlateau plateau);
void afficher(int, int, char);
void effacer(int x, int y);
void dessinerSerpent(int lesX[], int lesY[]);
void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool * collision, bool * pomme);
void gotoxy(int x, int y);
int kbhit();
void disable_echo();
void enable_echo();


int main()
{
	time_t debut = clock();

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
   
	// initialisation de la position du serpent : positionnement de la
	// tête en (X_DEPART_SERPENT, Y_DEPART_SEREPNT), puis des anneaux à sa gauche
	for(int i=0 ; i<TAILLE ; i++)
	{
		lesX[i] = X_DEPART_SERPENT-i;
		lesY[i] = Y_DEPART_SEREPNT;
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
	do {
		switch (touche)
		{
			case GAUCHE :
				direction = GAUCHE;
				break;
			case HAUT 	:
				direction = HAUT;
				break;
			case BAS 	:
				direction = BAS;
				break;
			case DROITE :
				direction = DROITE;
				break;
		}
		progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee);
		if (pommeMangee)
		{
            NbPommesManger++;
			gagne = (NbPommesManger==NB_POMMES);
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
		gotoxy(LARGEUR_PLATEAU+2, 1);
		printf("Compteur de pas : %d\n", nbDepUnitaires);
        gotoxy(LARGEUR_PLATEAU+2, 2);
		printf("Temps écoulé : %.2f\n", (difftime(fin, debut)) );
	}
	return EXIT_SUCCESS;
}


/************************************************/
/*		FONCTIONS ET PROCEDURES DU JEU 			*/
/************************************************/
void initPlateau(tPlateau plateau)
{
	int i, j;
	// initialisation du plateau avec des espaces
	for (i = 1 ; i <= LARGEUR_PLATEAU ; i++)
	{
		for (int j=1 ; j <= HAUTEUR_PLATEAU ; j++)
		{
			plateau[i][j] = VIDE;
		}
	}
	// Mise en place la bordure autour du plateau
	// première ligne
	for (i = 1 ; i <= LARGEUR_PLATEAU ; i++)
	{
		plateau[i][1] = BORDURE;
	}
	// lignes intermédiaires
	for (j = 1 ; j <= HAUTEUR_PLATEAU ; j++)
	{
			plateau[1][j] = BORDURE;
			plateau[LARGEUR_PLATEAU][j] = BORDURE;
		}
	// dernière ligne
	for (i = 1; i <= LARGEUR_PLATEAU ; i++)
	{
		plateau[i][HAUTEUR_PLATEAU] = BORDURE;
	}
}

void dessinerPlateau(tPlateau plateau)
{
	int i, j;
	// affiche eà l'écran le contenu du tableau 2D représentant le plateau
	for (i = 1 ; i <= LARGEUR_PLATEAU ; i++){
		for (j = 1 ; j <= HAUTEUR_PLATEAU ; j++)
		{
			afficher(i, j, plateau[i][j]);
		}
	}
}

void ajouterPomme(tPlateau plateau)
{
	// génère aléatoirement la position d'une pomme,
	// vérifie que ça correspond à une case vide
	// du plateau puis l'ajoute au plateau et l'affiche
	plateau[COORDONNE_POMME_X[NbPommesManger]][COORDONNE_POMME_Y[NbPommesManger]] = POMME;
	afficher(COORDONNE_POMME_X[NbPommesManger], COORDONNE_POMME_Y[NbPommesManger], POMME);
}

void afficher(int x, int y, char car)
{
	gotoxy(x, y);
	printf("%c", car);
	gotoxy(1,1);
}

void effacer(int x, int y)
{
	gotoxy(x, y);
	printf(" ");
	gotoxy(1,1);
}

void dessinerSerpent(int lesX[], int lesY[])
{
	int i;
	// affiche les anneaux puis la tête
	for(i = 1 ; i < TAILLE ; i++)
	{
		afficher(lesX[i], lesY[i], CORPS);
	}
	afficher(lesX[0], lesY[0],TETE);
}

void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool * collision, bool * pomme)
{
	int i;
	// efface le dernier élément avant d'actualiser la position de tous les 
	// élémentds du serpent avant de le  redessiner et détecte une
	// collision avec une pomme ou avec une bordure
	effacer(lesX[TAILLE-1], lesY[TAILLE-1]);

	for(i = TAILLE - 1 ; i > 0 ; i--)
	{
		lesX[i] = lesX[i-1];
		lesY[i] = lesY[i-1];
	}
	//faire progresser la tete dans la nouvelle direction
	switch(direction)
	{
		case HAUT	: 
			lesY[0] = lesY[0] - 1;
			break;
		case BAS	:
			lesY[0] = lesY[0] + 1;
			break;
		case DROITE	:
			lesX[0] = lesX[0] + 1;
			break;
		case GAUCHE	:
			lesX[0] = lesX[0] - 1;
			break;
	}
	*pomme = false;
	// détection d'une "collision" avec une pomme
	if (plateau[lesX[0]][lesY[0]] == POMME)
	{
		*pomme = true;
		// la pomme disparait du plateau
		plateau[lesX[0]][lesY[0]] = VIDE;
	}
	// détection d'une collision avec la bordure
	else if (plateau[lesX[0]][lesY[0]] == BORDURE)
	{
		*collision = true;
	}
   	dessinerSerpent(lesX, lesY);
	nbDepUnitaires++;
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