#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>

// Configuration du jeu
#define TAILLE 10
#define LARGEUR_PLATEAU 80	
#define HAUTEUR_PLATEAU 40	
#define X_INITIAL 40
#define Y_INITIAL 20
#define NB_POMMES 10
#define ATTENTE 200000
#define CORPS 'X'
#define TETE 'O'
#define HAUT 'z'
#define BAS 's'
#define GAUCHE 'q'
#define DROITE 'd'
#define STOP 'a'
#define BORDURE '#'
#define VIDE ' '
#define POMME '6'

typedef char tPlateau[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1];

// Tableaux des pommes prédéfinies
int lesPommesX[NB_POMMES] = {75, 75, 78, 2, 8, 78, 74, 2, 72, 5};
int lesPommesY[NB_POMMES] = {8, 39, 2, 2, 5, 39, 33, 38, 35, 2};
bool pommesMangees[NB_POMMES] = {false};

// Prototypes de fonctions
void initPlateau(tPlateau plateau);
void dessinerPlateau(tPlateau plateau);
void ajouterPomme(tPlateau plateau, int pommeIndex);
void afficher(int, int, char);
void effacer(int x, int y);
void dessinerSerpent(int lesX[], int lesY[]);
char choisirDirection(int lesX[], int lesY[], tPlateau plateau, int pommeX, int pommeY);
void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool * collision, bool * pomme, int * indexPommeMangee);
void gotoxy(int x, int y);
void disable_echo();
void enable_echo();

int main() {
    int lesX[TAILLE];
    int lesY[TAILLE];
    char direction;
    tPlateau lePlateau;
    bool collision = false;
    bool gagne = false;
    bool pommeMangee = false;
    int nbPommes = 0;
    int tempsCPU = 0;
    int deplacements = 0;
    int indexPommeMangee = -1;
    clock_t debut, fin;

    // Initialisation du serpent
    for(int i = 0; i < TAILLE; i++) {
        lesX[i] = X_INITIAL - i;
        lesY[i] = Y_INITIAL;
    }

    // Préparation du plateau
    initPlateau(lePlateau);
    system("clear");
    dessinerPlateau(lePlateau);

    // Ajout de la première pomme
    ajouterPomme(lePlateau, 0);

    // Initialisation du serpent
    dessinerSerpent(lesX, lesY);
    disable_echo();
    direction = DROITE;

    // Début du chrono CPU
    debut = clock();

    // Boucle de jeu
    while (nbPommes < NB_POMMES && !collision) {
        // Choisir la direction vers la prochaine pomme
        direction = choisirDirection(lesX, lesY, lePlateau, 
                                     lesPommesX[nbPommes], 
                                     lesPommesY[nbPommes]);
        
        // Progresser et vérifier les conditions
        progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee, &indexPommeMangee);
        deplacements++;

        if (pommeMangee) {
            pommesMangees[indexPommeMangee] = true;
            nbPommes++;
            pommeMangee = false;
            
            // Ajouter la prochaine pomme si disponible
            if (nbPommes < NB_POMMES) {
                ajouterPomme(lePlateau, nbPommes);
            }
        }

        usleep(ATTENTE);
    }

    // Fin du chrono CPU
    fin = clock();
    tempsCPU = (fin - debut) * 1000000 / CLOCKS_PER_SEC;

    // Affichage des résultats
    enable_echo();
    gotoxy(1, HAUTEUR_PLATEAU+2);
    printf("Nombre de déplacements : %d\n", deplacements);
    printf("Temps CPU : %d microsecondes\n", tempsCPU);

    return EXIT_SUCCESS;
}

// Reste du code identique à la version précédente, 
// avec quelques modifications dans progresser() et ajouterPomme()

char choisirDirection(int lesX[], int lesY[], tPlateau plateau, int pommeX, int pommeY) {
    int teteX = lesX[0];
    int teteY = lesY[0];

    // Calcul de la différence de position
    int diffX = pommeX - teteX;
    int diffY = pommeY - teteY;

    // Priorité aux déplacements horizontaux ou verticaux
    if (abs(diffX) > abs(diffY)) {
        // Se déplacer horizontalement en priorité
        return (diffX > 0) ? DROITE : GAUCHE;
    } else {
        // Se déplacer verticalement
        return (diffY > 0) ? BAS : HAUT;
    }
}

void initPlateau(tPlateau plateau){
    // initialisation du plateau avec des espaces
    for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
        for (int j=1 ; j<=HAUTEUR_PLATEAU ; j++){
            plateau[i][j] = VIDE;
        }
    }
    // Mise en place la bordure autour du plateau
    // première ligne
    for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
        plateau[i][1] = BORDURE;
    }
    // lignes intermédiaires
    for (int j=1 ; j<=HAUTEUR_PLATEAU ; j++){
            plateau[1][j] = BORDURE;
            plateau[LARGEUR_PLATEAU][j] = BORDURE;
        }
    // dernière ligne
    for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
        plateau[i][HAUTEUR_PLATEAU] = BORDURE;
    }
}

void dessinerPlateau(tPlateau plateau){
    // affiche à l'écran le contenu du tableau 2D représentant le plateau
    for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
        for (int j=1 ; j<=HAUTEUR_PLATEAU ; j++){
            afficher(i, j, plateau[i][j]);
        }
    }
}



void afficher(int x, int y, char car){
    gotoxy(x, y);
    printf("%c", car);
    gotoxy(1,1);
}

void effacer(int x, int y){
    gotoxy(x, y);
    printf(" ");
    gotoxy(1,1);
}

void dessinerSerpent(int lesX[], int lesY[]){
    // affiche les anneaux puis la tête
    for(int i=1 ; i<TAILLE ; i++){
        afficher(lesX[i], lesY[i], CORPS);
    }
    afficher(lesX[0], lesY[0],TETE);
}

void ajouterPomme(tPlateau plateau, int pommeIndex) {
    // Ajouter la pomme à sa position prédéfinie
    int x = lesPommesX[pommeIndex];
    int y = lesPommesY[pommeIndex];
    
    plateau[x][y] = POMME;
    afficher(x, y, POMME);
}

void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool * collision, bool * pomme, int * indexPommeMangee) {
    // efface le dernier élément avant d'actualiser la position de tous les 
    // éléments du serpent avant de le  redessiner et détecte une
    // collision avec une pomme ou avec une bordure
    effacer(lesX[TAILLE-1], lesY[TAILLE-1]);

    for(int i=TAILLE-1 ; i>0 ; i--){
        lesX[i] = lesX[i-1];
        lesY[i] = lesY[i-1];
    }
    //faire progresser la tete dans la nouvelle direction
    switch(direction){
        case HAUT : 
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
    *pomme = false;
    *indexPommeMangee = -1;

    // détection d'une "collision" avec une pomme
    for (int i = 0; i < NB_POMMES; i++) {
        if (!pommesMangees[i] && 
            lesX[0] == lesPommesX[i] && 
            lesY[0] == lesPommesY[i]) {
            *pomme = true;
            *indexPommeMangee = i;
            plateau[lesX[0]][lesY[0]] = VIDE;
            effacer(lesX[0], lesY[0]);
            break;
        }
    }

    // détection d'une collision avec la bordure
    if (plateau[lesX[0]][lesY[0]] == BORDURE){
        *collision = true;
    }
    
    dessinerSerpent(lesX, lesY);
}

// Le reste du code reste identique à la version précédente

void gotoxy(int x, int y) { 
    printf("\033[%d;%df", y, x);
}

void disable_echo() {
    struct termios tty;
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    tty.c_lflag &= ~ECHO;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

void enable_echo() {
    struct termios tty;
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    tty.c_lflag |= ECHO;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}