/**
 * @file version3.c
 * @brief Jeu Snake version 3 pour le projet SAE 1.02.
 *
 * Ce fichier implémente la version 3 de la SAE 1.02 du jeu Snake avec des obstacles fixes.
 * Le serpent se déplace de manière autonome sur un plateau contenant des murs et des pommes
 * positionnées à des coordonnées définies.
 *
 * @details
 * - Le serpent se déplace de manière autonome
 * - Le plateau contient 6 murs fixes de taille 5x5
 * - Les pommes apparaissent à des positions prédéfinies
 * - Le jeu se termine quand le serpent heurte un mur ou mange 10 pommes
 *
 * @author LE SECH Marceau
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

#define TAILLE 10
#define LARGEUR_PLATEAU 80
#define HAUTEUR_PLATEAU 40
#define X_DEPART_SERPENT 40
#define Y_DEPART_SERPENT 20
#define NB_POMMES 10
#define NB_PAVES 6
#define TAILLE_PAVE 5
#define ATTENTE 200000
#define CORPS 'X'
#define TETE 'O'
#define BORDURE '#'
#define STOP 'a'
#define VIDE ' '
#define POMME '6'
#define PAVE '#'
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
} Portail;

const Portail TROU_HAUT = {40, 0};
const Portail TROU_BAS = {40, 40};
const Portail TROU_GAUCHE = {0, 20};
const Portail TROU_DROITE = {80, 20};

typedef char tPlateau[LARGEUR_PLATEAU + 1][HAUTEUR_PLATEAU + 1];

// Nouvelles coordonnées fixes pour les pommes
const int lesPommesX[NB_POMMES] = {75, 75, 78, 2, 8, 78, 74, 2, 72, 5};
const int lesPommesY[NB_POMMES] = {8, 39, 2, 2, 5, 39, 33, 38, 35, 2};

// Coordonnées fixes pour les pavés
const int lesPavesX[NB_PAVES] = {3, 74, 3, 74, 38, 38};
const int lesPavesY[NB_PAVES] = {3, 3, 34, 34, 21, 15};

int NbPasSerpent = 0;
int NbPommesManger = 0;

// Prototypes de fonctions
void initPlateau(tPlateau plateau);
void dessinerPlateau(tPlateau plateau);
void ajouterPomme(tPlateau plateau);
void ajouterPaves(tPlateau plateau);
void afficher(int x, int y, char car);
void effacer(int x, int y);
void dessinerSerpent(int lesX[], int lesY[]);
void determinerDirectionVersObjectif(int lesX[], int lesY[], tPlateau plateau, char *direction, int cibleX, int cibleY);
bool detecterCollision(int lesX[], int lesY[], tPlateau plateau, char directionProchaine);
int distancePlusCourt(int lesX[], int lesY[], int pommeX, int pommeY);
void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool *collision, bool *pomme, bool *SerpentPasserPortail);
void gotoxy(int x, int y);
int kbhit();
void disable_echo();
void enable_echo();

int main()
{
    clock_t begin = clock();
    
    int lesX[TAILLE];
    int lesY[TAILLE];
    char touche;
    char direction;
    tPlateau lePlateau;
    bool collision = false;
    bool gagne = false;
    bool pommeMangee = false;
    bool SerpentPasserPortail = false;

    for (int i = 0; i < TAILLE; i++) {
        lesX[i] = X_DEPART_SERPENT - i;
        lesY[i] = Y_DEPART_SERPENT;
    }

    initPlateau(lePlateau);
    system("clear");
    ajouterPaves(lePlateau);
    ajouterPomme(lePlateau);
    dessinerPlateau(lePlateau);
    dessinerSerpent(lesX, lesY);
    disable_echo();
    direction = DROITE;

    int meilleurDistance = distancePlusCourt(lesX, lesY, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);

    do {
        switch (meilleurDistance) {
            case CHEMIN_HAUT:
                if (SerpentPasserPortail) {
                    determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);
                } else {
                    determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, TROU_HAUT.x, TROU_HAUT.y);
                }
                break;
            case CHEMIN_BAS:
                if (SerpentPasserPortail) {
                    determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);
                } else {
                    determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, TROU_BAS.x, TROU_BAS.y);
                }
                break;
            case CHEMIN_GAUCHE:
                if (SerpentPasserPortail) {
                    determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);
                } else {
                    determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, TROU_GAUCHE.x, TROU_GAUCHE.y);
                }
                break;
            case CHEMIN_DROITE:
                if (SerpentPasserPortail) {
                    determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);
                } else {
                    determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, TROU_DROITE.x, TROU_DROITE.y);
                }
                break;
            default:
                determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);
                break;
        }

        progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee, &SerpentPasserPortail);

        if (pommeMangee) {
            NbPommesManger++;
            gagne = (NbPommesManger == NB_POMMES);
            SerpentPasserPortail = false;
            if (!gagne) {
                ajouterPomme(lePlateau);
                meilleurDistance = distancePlusCourt(lesX, lesY, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);
                pommeMangee = false;
            }
        }

        if (!gagne && !collision) {
            usleep(ATTENTE);
            if (kbhit() == 1) {
                touche = getchar();
            }
        }
    } while (touche != STOP && !collision && !gagne);

    enable_echo();
    gotoxy(1, HAUTEUR_PLATEAU + 1);
    
    clock_t end = clock();
    double tmpsCPU = ((end - begin) * 1.0) / CLOCKS_PER_SEC;
    
    printf("Compteur de pas : %d\n", NbPasSerpent);
    printf("Temps CPU = %.3f secondes\n", tmpsCPU);

    return EXIT_SUCCESS;
}

void initPlateau(tPlateau plateau)
{
    int i, j;

    for (i = 1; i <= LARGEUR_PLATEAU; i++) {
        for (j = 1; j <= HAUTEUR_PLATEAU; j++) {
            plateau[i][j] = VIDE;
        }
    }

    for (i = 1; i <= LARGEUR_PLATEAU; i++) {
        plateau[i][1] = BORDURE;
    }

    for (j = 1; j <= HAUTEUR_PLATEAU; j++) {
        plateau[1][j] = BORDURE;
        plateau[LARGEUR_PLATEAU][j] = BORDURE;
    }

    for (i = 1; i <= LARGEUR_PLATEAU; i++) {
        plateau[i][HAUTEUR_PLATEAU] = BORDURE;
    }

    plateau[LARGEUR_PLATEAU][HAUTEUR_PLATEAU / 2] = VIDE;
    plateau[LARGEUR_PLATEAU / 2][HAUTEUR_PLATEAU] = VIDE;
    plateau[LARGEUR_PLATEAU / 2][1] = VIDE;
    plateau[1][HAUTEUR_PLATEAU / 2] = VIDE;
}

void ajouterPaves(tPlateau plateau)
{
    // Ajoute les pavés de taille 5x5 aux positions définies
    for (int p = 0; p < NB_PAVES; p++) {
        for (int i = 0; i < TAILLE_PAVE; i++) {
            for (int j = 0; j < TAILLE_PAVE; j++) {
                plateau[lesPavesX[p] + i][lesPavesY[p] + j] = PAVE;
                afficher(lesPavesX[p] + i, lesPavesY[p] + j, PAVE);
            }
        }
    }
}

// Les autres fonctions restent identiques à la version 2
void dessinerPlateau(tPlateau plateau)
{
    for (int i = 1; i <= LARGEUR_PLATEAU; i++) {
        for (int j = 1; j <= HAUTEUR_PLATEAU; j++) {
            afficher(i, j, plateau[i][j]);
        }
    }
}

void ajouterPomme(tPlateau plateau)
{
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
    for (int i = 1; i < TAILLE; i++) {
        afficher(lesX[i], lesY[i], CORPS);
    }
    afficher(lesX[0], lesY[0], TETE);
}

void determinerDirectionVersObjectif(int lesX[], int lesY[], tPlateau plateau, char *direction, int cibleX, int cibleY)
{
    int dx = cibleX - lesX[0];
    int dy = cibleY - lesY[0];

    if (dy != 0) {
        *direction = (dy > 0) ? BAS : HAUT;
        if (detecterCollision(lesX, lesY, plateau, *direction)) {
            *direction = (dx > 0) ? DROITE : GAUCHE;
            if (detecterCollision(lesX, lesY, plateau, *direction)) {
                *direction = (dx > 0) ? GAUCHE : DROITE;
                if (detecterCollision(lesX, lesY, plateau, *direction)) {
                    *direction = (dy > 0) ? HAUT : BAS;
                }
            }
        }
    } else if (dx != 0) {
        *direction = (dx > 0) ? DROITE : GAUCHE;
        if (detecterCollision(lesX, lesY, plateau, *direction)) {
            *direction = (dy > 0) ? BAS : HAUT;
            if (detecterCollision(lesX, lesY, plateau, *direction)) {
                *direction = (dy > 0) ? HAUT : BAS;
                if (detecterCollision(lesX, lesY, plateau, *direction)) {
                    *direction = (dx > 0) ? GAUCHE : DROITE;
                }
            }
        }
    }
}

bool detecterCollision(int lesX[], int lesY[], tPlateau plateau, char directionProchaine)
{
    int nouvelleX = lesX[0];
    int nouvelleY = lesY[0];

    switch (directionProchaine) {
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

    if (plateau[nouvelleX][nouvelleY] == BORDURE || plateau[nouvelleX][nouvelleY] == PAVE) {
        return true;
    }

    for (int i = 0; i < TAILLE; i++) {
        if (lesX[i] == nouvelleX && lesY[i] == nouvelleY) {
            return true;
        }
    }

    return false;
}


int distancePlusCourt(int lesX[], int lesY[], int pommeX, int pommeY)
{
    // Définition des variables pour calculer les distances de chaque chemin
    int CheminDirectPomme;
    int CheminPortailHaut;
    int CheminPortailBas;
    int CheminPortailGauche;
    int CheminPortailDroite;

    // Calcul de la distance directe entre la tête du serpent et la pomme
    CheminDirectPomme = abs(lesX[0] - pommeX) + abs(lesY[0] - pommeY);

    // Calcul des distances en passant par différents portails (haut, bas, gauche, droite)
    // Chaque chemin nécessite de passer par un portail et d'en sortir de l'autre côté avant d'atteindre la pomme.
    CheminPortailHaut = abs(lesX[0] - TROU_HAUT.x) + abs(lesY[0] - TROU_HAUT.y) + abs(pommeX - TROU_BAS.x) + abs(pommeY - TROU_BAS.y);
    CheminPortailBas = abs(lesX[0] - TROU_BAS.x) + abs(lesY[0] - TROU_BAS.y) + abs(pommeX - TROU_HAUT.x) + abs(pommeY - TROU_HAUT.y);
    CheminPortailGauche = abs(lesX[0] - TROU_GAUCHE.x) + abs(lesY[0] - TROU_GAUCHE.y) + abs(pommeX - TROU_DROITE.x) + abs(pommeY - TROU_DROITE.y);
    CheminPortailDroite = abs(lesX[0] - TROU_DROITE.x) + abs(lesY[0] - TROU_DROITE.y) + abs(pommeX - TROU_GAUCHE.x) + abs(pommeY - TROU_GAUCHE.y);

    // Tableau contenant les distances calculées pour chaque chemin
    int tab[5] = {CheminDirectPomme, CheminPortailHaut, CheminPortailBas, CheminPortailGauche, CheminPortailDroite};

    // Initialisation de la distance minimale à la distance directe vers la pomme
    int minDistance = CheminDirectPomme;
    int minDistanceI = 0; // Index du chemin avec la distance minimale

    // Comparaison des distances pour déterminer le chemin le plus court
    for (int i = 0; i < 5; i++)
    {
        if (minDistance > tab[i]) // Si une distance plus courte est trouvée
        {
            minDistance = tab[i]; // Met à jour la distance minimale
            minDistanceI = i;     // Met à jour l'index du chemin le plus court
        }
    }

    // En fonction de l'index du chemin le plus court, retourner le type de chemin
    switch (minDistanceI)
    {
    case 0:
        return CHEMIN_POMME; // Si le chemin direct est le plus court, retourner CHEMIN_POMME
        break;
    case 1:
        return CHEMIN_HAUT;  // Si le chemin via le portail haut est le plus court, retourner CHEMIN_HAUT
        break;
    case 2:
        return CHEMIN_BAS;   // Si le chemin via le portail bas est le plus court, retourner CHEMIN_BAS
        break;
    case 3:
        return CHEMIN_GAUCHE; // Si le chemin via le portail gauche est le plus court, retourner CHEMIN_GAUCHE
        break;
    default:
        return CHEMIN_DROITE; // Si le chemin via le portail droit est le plus court, retourner CHEMIN_DROITE
        break;
    }
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
