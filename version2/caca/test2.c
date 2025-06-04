#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

#define TAILLE 10
#define LARGEUR_PLATEAU 80
#define HAUTEUR_PLATEAU 40
#define X_DEPART_SERPENT 40
#define Y_DEPART_SERPENT 20
#define NB_POMMES 10
#define ATTENTE 60000
#define CORPS 'X'
#define TETE 'O'
#define BORDURE '#'
#define STOP 'a'
#define VIDE ' '
#define POMME '6'
#define HAUT 'z'
#define BAS 's'
#define GAUCHE 'q'
#define DROITE 'd'
#define CHEMIN_HAUT 1
#define CHEMIN_BAS 2
#define CHEMIN_GAUCHE 3
#define CHEMIN_DROITE 4
#define CHEMIN_POMME 5

typedef struct {
    int x;
    int y;
} Point;

const Point TROU_HAUT = {40, 0};
const Point TROU_BAS = {40, 40};
const Point TROU_GAUCHE = {0, 20};
const Point TROU_DROITE = {80, 20};

typedef char tPlateau[LARGEUR_PLATEAU + 1][HAUTEUR_PLATEAU + 1];

const int lesPommesX[NB_POMMES] = {75, 75, 78, 2, 8, 78, 74, 2, 72, 5};
const int lesPommesY[NB_POMMES] = {8, 39, 2, 2, 5, 39, 33, 38, 35, 2};

int NbPasSerpent = 0;
int NbPommesManger = 0;

void initPlateau(tPlateau plateau);
void dessinerPlateau(tPlateau plateau);
void ajouterPomme(tPlateau plateau);
void afficher(int, int, char);
void effacer(int x, int y);
void dessinerSerpent(int lesX[], int lesY[]);
void directionSerpentVersObjectif(int lesX[], int lesY[], tPlateau plateau, char *direction, int objectifX, int objectifY);
bool verifierCollision(int lesX[], int lesY[], tPlateau plateau, char directionProchaine);
int calculerDistance(int lesX[], int lesY[], int pommeX, int pommeY);
void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool *collision, bool *pomme, bool *teleporter);
void gotoxy(int x, int y);
int kbhit();
void disable_echo();
void enable_echo();

int main()
{
    clock_t begin = clock();

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


	bool teleporter = false;

	// initialisation de la position du serpent : positionnement de la
	// tête en (X_DEPART_SERPENT, Y_DEPART_SEREPNT), puis des anneaux à sa gauche
    for (int i = 0; i < TAILLE; i++)
    {
        lesX[i] = X_DEPART_SERPENT - i;
        lesY[i] = Y_DEPART_SERPENT;
    }

	// mise en place du plateau
    initPlateau(lePlateau);
    system("clear");
	ajouterPomme(lePlateau);
    dessinerPlateau(lePlateau);

	// initialisation : le serpent se dirige vers la DROITE
    dessinerSerpent(lesX, lesY);
    disable_echo();
    direction = DROITE;

    int meilleurDistance = calculerDistance(lesX, lesY, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);

	// boucle de jeu. Arret si touche STOP, si collision avec une bordure ou
	// si toutes les pommes sont mangées
    do
	{
		// choisis la direction en fonction de la meilleur distance
		if (meilleurDistance == CHEMIN_HAUT) // se dirige vers le trou du haut puis quand il s'est téléporter avance vers la pomme
		{
			if (teleporter)
			{
				directionSerpentVersObjectif(lesX, lesY, lePlateau, &direction, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);
			}
			else
			{
				directionSerpentVersObjectif(lesX, lesY, lePlateau, &direction, TROU_HAUT.x, TROU_HAUT.y);
			}
		}
		else if (meilleurDistance == CHEMIN_BAS) // se dirige vers le trou du bas puis quand il s'est téléporter avance vers la pomme
		{
			if (teleporter)
			{
				directionSerpentVersObjectif(lesX, lesY, lePlateau, &direction, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);
			}
			else
			{
				directionSerpentVersObjectif(lesX, lesY, lePlateau, &direction, TROU_BAS.x, TROU_BAS.y);
			}
		}
		else if (meilleurDistance == CHEMIN_GAUCHE) // se dirige vers le trou de gauche puis quand il s'est téléporter avance vers la pomme
		{
			if (teleporter)
			{
				directionSerpentVersObjectif(lesX, lesY, lePlateau, &direction, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);
			}
			else
			{
				directionSerpentVersObjectif(lesX, lesY, lePlateau, &direction, TROU_GAUCHE.x, TROU_GAUCHE.y);
			}
		}
		else if (meilleurDistance == CHEMIN_DROITE) // se dirige vers le trou de droite puis quand il s'est téléporter avance vers la pomme
		{
			if (teleporter)
			{
				directionSerpentVersObjectif(lesX, lesY, lePlateau, &direction, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);
			}
			else
			{
				directionSerpentVersObjectif(lesX, lesY, lePlateau, &direction, TROU_DROITE.x, TROU_DROITE.y);
			}
		}
		else // sinon se dirige uniquement vers la pomme
		{
			directionSerpentVersObjectif(lesX, lesY, lePlateau, &direction, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);
		}

		progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee, &teleporter);
		NbPasSerpent++;

		// Ajoute une pomme au compteur de pomme quand elle est mangée et arrete le jeu si score atteint 10
		if (pommeMangee)
		{
			NbPommesManger++;
			gagne = (NbPommesManger == NB_POMMES);
			teleporter = false; // remet en false pour pouvoir se retéléporter après avoir manger une pomme
			if (!gagne)
			{
				ajouterPomme(lePlateau);
				meilleurDistance = calculerDistance(lesX, lesY, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]); // recalcul la meilleur position après l'apparition d'une nouvelle pomme
				pommeMangee = false;
			}
		}
		if (!gagne)
		{
			if (!collision)
			{
				usleep(ATTENTE);
				if (kbhit() == 1)
				{
					touche = getchar();
				}
			}
		}
	} while (touche != STOP && !collision && !gagne);
	enable_echo();
	gotoxy(1, HAUTEUR_PLATEAU + 1);

	clock_t end = clock(); // fin du calcul du temps CPU
	double tmpsCPU = ((end - begin) * 1.0) / CLOCKS_PER_SEC;

	// afficher les performances du programme
	printf("Le serpent c'est déplacer %d fois\n", NbPasSerpent);
    printf("Temps CPU = %.3f secondes\n", tmpsCPU);
	

	return EXIT_SUCCESS;
}

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
	plateau[LARGEUR_PLATEAU][HAUTEUR_PLATEAU/2] = VIDE;
	plateau[LARGEUR_PLATEAU/2][HAUTEUR_PLATEAU] = VIDE;
	plateau[LARGEUR_PLATEAU/2][1] = VIDE;
	plateau[1][HAUTEUR_PLATEAU/2] = VIDE;

}


void dessinerPlateau(tPlateau plateau)
{
	// affiche à l'écran le contenu du tableau 2D représentant le plateau
	for (int i = 1; i <= LARGEUR_PLATEAU; i++)
	{
		for (int j = 1; j <= HAUTEUR_PLATEAU; j++)
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
	plateau[lesPommesX[NbPommesManger]][lesPommesY[NbPommesManger]] = POMME;
	afficher(lesPommesX[NbPommesManger], lesPommesY[NbPommesManger], POMME);
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

void dessinerSerpent(int lesX[], int lesY[])
{
	// affiche les anneaux puis la tête
	for (int i = 1; i < TAILLE; i++)
	{
		afficher(lesX[i], lesY[i], CORPS);
	}
	afficher(lesX[0], lesY[0], TETE);
}

void directionSerpentVersObjectif(int lesX[], int lesY[], tPlateau plateau, char *direction, int objectifX, int objectifY)
{
    // Calculer les différences de position
    int dx = objectifX - lesX[0];
    int dy = objectifY - lesY[0];

    // Tableau des directions à tester
    char directions[4];
    int nbDirections = 0;

    // Prioritiser les directions verticales si dy est significatif
    if (abs(dy) > abs(dx)) {
        directions[nbDirections++] = (dy > 0) ? BAS : HAUT;
        directions[nbDirections++] = (dx > 0) ? DROITE : GAUCHE;
        directions[nbDirections++] = (dx > 0) ? GAUCHE : DROITE;
        directions[nbDirections++] = (dy > 0) ? HAUT : BAS;
    } 
    // Sinon, prioritiser les directions horizontales
    else {
        directions[nbDirections++] = (dx > 0) ? DROITE : GAUCHE;
        directions[nbDirections++] = (dy > 0) ? BAS : HAUT;
        directions[nbDirections++] = (dy > 0) ? HAUT : BAS;
        directions[nbDirections++] = (dx > 0) ? GAUCHE : DROITE;
    }

    // Tester les directions dans l'ordre jusqu'à trouver une direction sans collision
    for (int i = 0; i < nbDirections; i++) {
        if (!verifierCollision(lesX, lesY, plateau, directions[i])) {
            *direction = directions[i];
            return;
        }
    }
}

int calculerDistance(int lesX[], int lesY[], int pommeX, int pommeY)
{
	// définition des variables
	int CheminDirectPomme;
	int passageTrouHaut;
	int passageTrouBas;
	int passageTrouGauche;
	int passageTrouDroit;
	

	// calcul la distance pour chaque chemin du serpent vers la pomme
	CheminDirectPomme = abs(lesX[0] - pommeX) + abs(lesY[0] - pommeY);
	passageTrouHaut = abs(lesX[0] - TROU_HAUT.x) + abs(lesY[0] - TROU_HAUT.y) + abs(pommeX - TROU_BAS.x) + abs(pommeY - TROU_BAS.y);
	passageTrouBas = abs(lesX[0] - TROU_BAS.x) + abs(lesY[0] - TROU_BAS.y) + abs(pommeX - TROU_HAUT.x) + abs(pommeY - TROU_HAUT.y);
	passageTrouGauche = abs(lesX[0] - TROU_GAUCHE.x) + abs(lesY[0] - TROU_GAUCHE.y) + abs(pommeX - TROU_DROITE.x) + abs(pommeY - TROU_DROITE.y);
	passageTrouDroit = abs(lesX[0] - TROU_DROITE.x) + abs(lesY[0] - TROU_DROITE.y) + abs(pommeX - TROU_GAUCHE.x) + abs(pommeY - TROU_GAUCHE.y);
	
	int tab[5] = {CheminDirectPomme, passageTrouHaut, passageTrouBas, passageTrouGauche, passageTrouDroit};

	int minDistance = CheminDirectPomme;
	int minDistanceI = 0; 

	for (int i = 0 ; i < 5 ; i++)
	{
		if (minDistance > tab[i])
		{
			minDistance = tab[i];
			minDistanceI = i;
		}
	}

	switch (minDistanceI)
	{
	case 0:
		return CHEMIN_POMME;
		break;
	case 1:
		return CHEMIN_HAUT;
		break;
	case 2:
		return CHEMIN_BAS;
		break;
	case 3:
		return CHEMIN_GAUCHE;
		break;
	default:
		return CHEMIN_DROITE;
		break;
	}
}

bool verifierCollision(int lesX[], int lesY[], tPlateau plateau, char directionProchaine)
{
	int nouvelleX = lesX[0]; // projeter des coordonnées en X
	int nouvelleY = lesY[0]; // projeter des coordonnées en Y

	// Calcul de la nouvelle position en fonction de la direction donnée
	switch (directionProchaine)
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

	// Vérification des collisions avec les bords du tableau
	if (plateau[nouvelleX][nouvelleY] == BORDURE)
	{
		return true; // Collision avec une bordure
	}

	// Vérification des collisions avec le corps du serpent
	for (int i = 0; i < TAILLE; i++)
	{
		if (lesX[i] == nouvelleX && lesY[i] == nouvelleY)
		{
			return true; // Collision avec le corps du serpent
		}
	}

	return false; // Pas de collision
}

/**
 * @brief Procédure qui calcule la prochaine position du serpent et qui l'affiche,
 * elle permet aussi de savoir si le serpent entre en collision avec une bordure, un pavé, ou une pomme.
 * @param lesX de type int tableau, Entrée : le tableau des X de N élément
 * @param lesY de type int tableau, Entrée : le tableau des Y de N élément
 * @param plateau de type tPlateau, qui donne en Entrée le plateau du jeu
 * @param direction de type char, Entrée : la direction du serpent attribuée aux touches 'z' 'q' 's' 'd'
 * @param collision de type bool, vérifie si il y a une collision
 * @param pomme de type bool, vérifie si une pomme est mangée
 * @param teleporter de type bool, vérifie si le serpent s'est téléporter
 */
void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool *collision, bool *pomme, bool *teleporter)
{
	// efface le dernier élément avant d'actualiser la position de tous les
	// élémentds du serpent avant de le  redessiner et détecte une
	// collision avec une pomme ou avec une bordure
	effacer(lesX[TAILLE - 1], lesY[TAILLE - 1]);

	for (int i = TAILLE - 1; i > 0; i--)
	{
		lesX[i] = lesX[i - 1];
		lesY[i] = lesY[i - 1];
	}
	// faire progresser la tete dans la nouvelle direction
	switch (direction)
	{
	case HAUT:
		lesY[0] = lesY[0] - 1;
		break;
	case BAS:
		lesY[0] = lesY[0] + 1;
		break;
	case DROITE:
		lesX[0] = lesX[0] + 1;
		break;
	case GAUCHE:
		lesX[0] = lesX[0] - 1;
		break;
	}

	// Faire des trous dans les bordures
	for (int i = 1; i < TAILLE; i++)
	{
		if (lesX[0] <= 0)
		{
			lesX[0] = LARGEUR_PLATEAU; // faire apparaitre à gauche
			*teleporter = true;		   // quand le serpent traverse le trou
		}
		else if (lesX[0] > LARGEUR_PLATEAU)
		{
			lesX[0] = 1;		// faire apparaitre à droite
			*teleporter = true; // quand le serpent traverse le trou
		}
		else if (lesY[0] <= 0)
		{
			lesY[0] = HAUTEUR_PLATEAU; // faire apparaitre en haut
			*teleporter = true;		   // quand le serpent traverse le trou
		}
		else if (lesY[0] > HAUTEUR_PLATEAU)
		{
			lesY[0] = 1;		// faire apparaitre en bas
			*teleporter = true; // quand le serpent traverse le trou
		}
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
}

void gotoxy(int x, int y)
{
    printf("\033[%d;%df", y, x);
}

int kbhit()
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

void disable_echo()
{
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void enable_echo()
{
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}
