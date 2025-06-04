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
#define X_INITIAL 40
#define Y_INITIAL 20
// nombre de pommes à manger pour gagner
#define NB_POMMES 10
// temporisation entre deux déplacements du serpent (en microsecondes)
#define ATTENTE 100000
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

#define PARSECONDE 1000000.0


// définition d'un type pour le plateau : tPlateau
// Attention, pour que les indices du tableau 2D (qui commencent à 0) coincident
// avec les coordonées à l'écran (qui commencent à 1), on ajoute 1 aux dimensions
// et on neutralise la ligne 0 et la colonne 0 du tableau 2D (elles ne sont jamais
// utilisées)
int lesPommesX[NB_POMMES] = {75, 75, 78, 2, 8, 78, 74,  2, 72, 5};
int lesPommesY[NB_POMMES] = { 8, 39,  2, 2, 5, 39, 33, 38, 35, 2};
typedef char tPlateau[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1];

typedef int tChemins[5];
// compteur de pommes mangées
int NbPommesSerpentManger = 0;
int nbDepUnitaires = 0;

typedef struct {
    int x;  // Coordonnée X du Portail.
    int y;  // Coordonnée Y du Portail.
} Portail;

const Portail TROU_HAUT = {40, 0};  // Portail en haut du plateau (Portail de coordonnées (40, 0)).
const Portail TROU_BAS = {40, 40};  // Portail en bas du plateau (Portail de coordonnées (40, 40)).
const Portail TROU_GAUCHE = {0, 20};  // Portail à gauche du plateau (Portail de coordonnées (0, 20)).
const Portail TROU_DROITE = {80, 20};  // Portail à droite du plateau (Portail de coordonnées (80, 20)).


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
int PasserPortails(int lesX[], int lesY[]);
int minimunTableau(tChemins Tableau);


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
   
	// initialisation de la position du serpent : positionnement de la
	// tête en (X_INITIAL, Y_INITIAL), puis des anneaux à sa gauche
	for(int i=0 ; i<TAILLE ; i++)
	{
		lesX[i] = X_INITIAL-i;
		lesY[i] = Y_INITIAL;
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
	
	touche = DROITE;

	// boucle de jeu. Arret si touche STOP, si collision avec une bordure ou
	// si toutes les pommes sont mangées
	int cheminPlusCourt ;
    
    time_t debut = clock();
	do {
		int CheminDirectPomme = abs(lesX[0] - lesPommesX[NbPommesSerpentManger]) + abs(lesY[0] - lesPommesY[NbPommesSerpentManger]);

        // Calcul des distances en passant par différents portails (haut, bas, gauche, droite)
        // Chaque chemin nécessite de passer par un portail et d'en sortir de l'autre côté avant d'atteindre la pomme.
        int CheminPortailHaut = abs(lesX[0] - TROU_HAUT.x) + abs(lesY[0] - TROU_HAUT.y) + abs(lesPommesX[NbPommesSerpentManger] - TROU_BAS.x) + abs(lesPommesY[NbPommesSerpentManger] - TROU_BAS.y);
        int CheminPortailBas = abs(lesX[0] - TROU_BAS.x) + abs(lesY[0] - TROU_BAS.y) + abs(lesPommesX[NbPommesSerpentManger] - TROU_HAUT.x) + abs(lesPommesY[NbPommesSerpentManger] - TROU_HAUT.y);
        int CheminPortailGauche = abs(lesX[0] - TROU_GAUCHE.x) + abs(lesY[0] - TROU_GAUCHE.y) + abs(lesPommesX[NbPommesSerpentManger] - TROU_DROITE.x) + abs(lesPommesY[NbPommesSerpentManger] - TROU_DROITE.y);
        int CheminPortailDroite = abs(lesX[0] - TROU_DROITE.x) + abs(lesY[0] - TROU_DROITE.y) + abs(lesPommesX[NbPommesSerpentManger] - TROU_GAUCHE.x) + abs(lesPommesY[NbPommesSerpentManger] - TROU_GAUCHE.y);

        // Tableau contenant les distances calculées pour chaque chemin
        tChemins Tableau = {CheminDirectPomme, CheminPortailHaut, CheminPortailBas, CheminPortailGauche, CheminPortailDroite};

        gotoxy(2+LARGEUR_PLATEAU, 1);
        printf("Pomme : %d", CheminDirectPomme);
        gotoxy(2+LARGEUR_PLATEAU, 2);
        printf("Haut : %d", CheminPortailHaut);
        gotoxy(2+LARGEUR_PLATEAU, 3);
        printf("Bas : %d", CheminPortailBas);
        gotoxy(2+LARGEUR_PLATEAU, 4);
        printf("Gauche : %d", CheminPortailGauche);
        gotoxy(2+LARGEUR_PLATEAU, 5);
        printf("Droit : %d", CheminPortailDroite);

        int test = minimunTableau(Tableau);

        switch (test)
        {
        case 0 :
            if ( (lesPommesX[NbPommesSerpentManger] - lesX[0]) > 0)
            {
                direction = DROITE ;
            }
            else if ( (lesPommesX[NbPommesSerpentManger] - lesX[0]) < 0)
            {
                direction = GAUCHE;
            }
            else if ( (lesPommesY[NbPommesSerpentManger] - lesY[0]) > 0 )
            {
                direction = BAS ;
            }
            else
            {
                direction = HAUT;
            }
            break;
        case 1 :
            if (PasserPortails(lesX, lesY) == false)
            {
                if ( (TROU_HAUT.x - lesX[0]) > 0)
                {
                    direction = DROITE ;
                }
                else if ( (TROU_HAUT.x - lesX[0]) < 0)
                {
                    direction = GAUCHE;
                }
                else if ( (TROU_HAUT.y - lesY[0]) > 0 )
                {
                    direction = BAS ;
                }
                else
                {
                    direction = HAUT;
                }
            }
            else{
                if ( (lesPommesX[NbPommesSerpentManger] - lesX[0]) > 0)
                {
                    direction = DROITE ;
                }
                else if ( (lesPommesX[NbPommesSerpentManger] - lesX[0]) < 0)
                {
                    direction = GAUCHE;
                }
                else if ( (lesPommesY[NbPommesSerpentManger] - lesY[0]) > 0 )
                {
                    direction = BAS ;
                }
                else
                {
                    direction = HAUT;
                }
            }
            
            
            break;
        case 2 :
            if (PasserPortails(lesX, lesY) == false)
            {
                if ( (TROU_BAS.x - lesX[0]) > 0)
                {
                    direction = DROITE ;
                }
                else if ( (TROU_BAS.x - lesX[0]) < 0)
                {
                    direction = GAUCHE;
                }
                else if ( (TROU_BAS.y - lesY[0]) > 0 )
                {
                    direction = BAS ;
                }
                else
                {
                    direction = HAUT;
                }
            }
            else{
                if ( (lesPommesX[NbPommesSerpentManger] - lesX[0]) > 0)
                {
                    direction = DROITE ;
                }
                else if ( (lesPommesX[NbPommesSerpentManger] - lesX[0]) < 0)
                {
                    direction = GAUCHE;
                }
                else if ( (lesPommesY[NbPommesSerpentManger] - lesY[0]) > 0 )
                {
                    direction = BAS ;
                }
                else
                {
                    direction = HAUT;
                }
            }
            break;

        case 3 :
            if (PasserPortails(lesX, lesY) == false)
            {
               if ( (TROU_GAUCHE.x - lesX[0]) > 0)
                {
                    direction = DROITE ;
                }
                else if ( (TROU_GAUCHE.x - lesX[0]) < 0)
                {
                    direction = GAUCHE;
                }
                else if ( (TROU_GAUCHE.y - lesY[0]) > 0 )
                {
                    direction = BAS ;
                }
                else
                {
                    direction = HAUT;
                }
            }
            else{
                if ( (lesPommesX[NbPommesSerpentManger] - lesX[0]) > 0)
                {
                    direction = DROITE ;
                }
                else if ( (lesPommesX[NbPommesSerpentManger] - lesX[0]) < 0)
                {
                    direction = GAUCHE;
                }
                else if ( (lesPommesY[NbPommesSerpentManger] - lesY[0]) > 0 )
                {
                    direction = BAS ;
                }
                else
                {
                    direction = HAUT;
                }
            }
            break;

        case 4 :
            if (PasserPortails(lesX, lesY) == false)
            {
                if ( (TROU_DROITE.x - lesX[0]) > 0)
                {
                    direction = DROITE ;
                }
                else if ( (TROU_DROITE.x - lesX[0]) < 0)
                {
                    direction = GAUCHE;
                }
                else if ( (TROU_DROITE.y - lesY[0]) > 0 )
                {
                    direction = BAS ;
                }
                else
                {
                    direction = HAUT;
                }
            }
            else{
                if ( (lesPommesX[NbPommesSerpentManger] - lesX[0]) > 0)
                {
                    direction = DROITE ;
                }
                else if ( (lesPommesX[NbPommesSerpentManger] - lesX[0]) < 0)
                {
                    direction = GAUCHE;
                }
                else if ( (lesPommesY[NbPommesSerpentManger] - lesY[0]) > 0 )
                {
                    direction = BAS ;
                }
                else
                {
                    direction = HAUT;
                }
            }
            break;
        }
        
		


		progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee);
        PasserPortails(lesX, lesY);

		if (pommeMangee)
		{
            NbPommesSerpentManger++;
			gagne = (NbPommesSerpentManger==NB_POMMES);
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
		printf("Compteur de pas : %d\n", nbDepUnitaires);
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

    // Gestion des téléportations du serpent via les portails
    if (lesX[0] <= 0)  // Si la tête du serpent dépasse la bordure gauche
    {
        lesX[0] = LARGEUR_PLATEAU; // La tête réapparaît à droite
    }
    else if (lesX[0] > LARGEUR_PLATEAU) // Si la tête dépasse la bordure droite
    {
        lesX[0] = 1; // La tête réapparaît à gauche
        
    }

    if (lesY[0] <= 0)  // Si la tête dépasse la bordure du haut
    {
        lesY[0] = HAUTEUR_PLATEAU; // La tête réapparaît en bas
    }
    else if (lesY[0] > HAUTEUR_PLATEAU) // Si la tête dépasse la bordure du bas
    {
        lesY[0] = 1; // La tête réapparaît en haut
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




int PasserPortails(int lesX[], int lesY[]){
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