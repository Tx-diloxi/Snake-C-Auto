/**
* @file version2.c
* @brief Jeu Snake version 4 pour le projet SAE 1.02.
*
* Ce fichier implémente la suite de la SAE 1.02 du jeu Shake. Le joueur contrôle un serpent 
* qui se déplace sur un plateau, mange des pommes qui sont positionnées à des coordonnées définit.
*
* @details
* - Le serpent se déplace de manière autonome et peut changer de direction sans intervention manuelle.
* - Une pomme apparaît sur le plateau aux coordonnés définit 
* - Le jeu se termine lorsque le serpent heurte un mur ou mange 10 pommes.
*
* @author
* LE SECH Marceau
*
* @version 2.0
* @date 08/12/2024
*/

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
#define CHEMIN_HAUT 1  // Identifiant pour la direction haut.
#define CHEMIN_BAS 2  // Identifiant pour la direction bas.
#define CHEMIN_GAUCHE 3  // Identifiant pour la direction gauche.
#define CHEMIN_DROITE 4  // Identifiant pour la direction droite.
#define CHEMIN_POMME 5  // Identifiant pour la direction menant vers une pomme.

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
int NbPommesManger = 0;  // Compteur du nombre de pommes mangées par le serpent.

void initPlateau(tPlateau plateau);  // Fonction pour initialiser le plateau de jeu.
void dessinerPlateau(tPlateau plateau);  // Fonction pour dessiner le plateau dans la console.
void ajouterPomme(tPlateau plateau);  // Fonction pour ajouter une pomme sur le plateau.
void afficher(int, int, char);  // Fonction pour afficher un caractère à une position donnée sur le plateau.
void effacer(int x, int y);  // Fonction pour effacer un caractère à une position donnée sur le plateau.
void dessinerSerpent(int lesX[], int lesY[]);  // Fonction pour dessiner le serpent.
void determinerDirectionVersObjectif(int lesX[], int lesY[], tPlateau plateau, char *direction, int cibleX, int cibleY);  // Fonction pour déterminer la direction vers un objectif.
bool detecterCollision(int lesX[], int lesY[], tPlateau plateau, char directionProchaine);  // Fonction pour détecter une collision avec le serpent.
int distancePlusCourt(int lesX[], int lesY[], int pommeX, int pommeY);  // Fonction pour calculer la distance la plus courte entre le serpent et une pomme.
void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool *collision, bool *pomme, bool *SerpentPasserPortail);  // Fonction pour faire avancer le serpent dans la direction choisie.
void gotoxy(int x, int y);  // Fonction pour déplacer le curseur à une position donnée sur la console.
int kbhit();  // Fonction pour détecter si une touche a été pressée.
void disable_echo();  // Fonction pour désactiver l'affichage des touches sur la console.
void enable_echo();  // Fonction pour réactiver l'affichage des touches sur la console.

int main()
{
    clock_t begin = clock();  // Enregistre l'heure de début pour calculer le temps d'exécution du programme.

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

    bool SerpentPasserPortail = false;  // Indicateur pour savoir si le serpent a passé un portail.

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

    int meilleurDistance = distancePlusCourt(lesX, lesY, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);  // Calcule la meilleure direction à suivre pour atteindre la pomme.

    // Boucle de jeu. Le jeu continue tant que l'utilisateur n'appuie pas sur la touche STOP ou qu'il n'y a pas de collision ou que toutes les pommes ne sont pas mangées.
    do
    {
        // Choisit la direction en fonction de la meilleure distance vers la pomme ou vers un portail.
        switch (meilleurDistance)
        {
            case CHEMIN_HAUT:  // Si la meilleure direction est vers le haut.
                if (SerpentPasserPortail) {
                    determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);  // Si le serpent a passé un portail, il se dirige vers la pomme.
                } else {
                    determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, TROU_HAUT.x, TROU_HAUT.y);  // Sinon, il se dirige vers le portail en haut.
                }
                break;

            case CHEMIN_BAS:  // Si la meilleure direction est vers le bas.
                if (SerpentPasserPortail) {
                    determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);
                } else {
                    determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, TROU_BAS.x, TROU_BAS.y);
                }
                break;

            case CHEMIN_GAUCHE:  // Si la meilleure direction est vers la gauche.
                if (SerpentPasserPortail) {
                    determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);
                } else {
                    determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, TROU_GAUCHE.x, TROU_GAUCHE.y);
                }
                break;

            case CHEMIN_DROITE:  // Si la meilleure direction est vers la droite.
                if (SerpentPasserPortail) {
                    determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);
                } else {
                    determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, TROU_DROITE.x, TROU_DROITE.y);
                }
                break;

            default:  // Cas où le serpent se dirige directement vers la pomme.
                determinerDirectionVersObjectif(lesX, lesY, lePlateau, &direction, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);
                break;
        }

        // Le serpent progresse dans la direction déterminée.
        progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee, &SerpentPasserPortail);

        // Ajoute une pomme au compteur de pommes quand elle est mangée et arrête le jeu si le score atteint 10.
        if (pommeMangee)
        {
            NbPommesManger++;  // Incrémente le nombre de pommes mangées.
            gagne = (NbPommesManger == NB_POMMES);  // Vérifie si toutes les pommes ont été mangées.
            SerpentPasserPortail = false;  // Réinitialise le passage de portail.
            if (!gagne)
            {
                ajouterPomme(lePlateau);  // Ajoute une nouvelle pomme sur le plateau.
                meilleurDistance = distancePlusCourt(lesX, lesY, lesPommesX[NbPommesManger], lesPommesY[NbPommesManger]);  // Recalcule la meilleure direction vers la nouvelle pomme.
                pommeMangee = false;  // Réinitialise l'indicateur de pomme mangée.
            }
        }

        // Continue à faire avancer le serpent si le jeu n'est pas terminé.
        if (!gagne)
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
    } while (touche != STOP && !collision && !gagne);  // La boucle continue tant que l'utilisateur n'appuie pas sur STOP, qu'il n'y a pas de collision et que toutes les pommes ne sont pas mangées.

    enable_echo();  // Réactive l'affichage des touches.
    gotoxy(1, HAUTEUR_PLATEAU + 1);  // Déplace le curseur en dehors du plateau de jeu.

    clock_t end = clock();  // Enregistre l'heure de fin pour calculer le temps d'exécution.
    double tmpsCPU = ((end - begin) * 1.0) / CLOCKS_PER_SEC;  // Calcule le temps CPU en secondes.

    // Affiche les performances du programme.
    printf("Compteur de pas : %d\n", NbPasSerpent);
    printf("Temps CPU = %.3f secondes\n", tmpsCPU);

    return EXIT_SUCCESS;  // Terminer le programme avec succès.
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
    plateau[lesPommesX[NbPommesManger]][lesPommesY[NbPommesManger]] = POMME;  // Place la pomme sur le plateau
    afficher(lesPommesX[NbPommesManger], lesPommesY[NbPommesManger], POMME);  // Affiche la pomme à l'écran
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


void determinerDirectionVersObjectif(int lesX[], int lesY[], tPlateau plateau, char *direction, int cibleX, int cibleY)
{
    // Calcul des directions possibles pour atteindre l'objectif
    int dx = cibleX - lesX[0]; // Différence en X entre la tête du serpent et la cible
    int dy = cibleY - lesY[0]; // Différence en Y entre la tête du serpent et la cible

    // Essayer de se déplacer dans la direction verticale (Y)
    if (dy != 0)
    {
        *direction = (dy > 0) ? BAS : HAUT; // Si la cible est en bas, se diriger vers le bas, sinon vers le haut
        if (detecterCollision(lesX, lesY, plateau, *direction)) // Vérifie s'il y a une collision dans cette direction
        {
            // Si collision, essayer la direction horizontale (X)
            *direction = (dx > 0) ? DROITE : GAUCHE; // Se diriger à droite si la cible est à droite, sinon à gauche
            if (detecterCollision(lesX, lesY, plateau, *direction)) // Vérifie s'il y a une collision dans cette direction horizontale
            {
                // Si collision dans la direction horizontale, essayer l'autre direction horizontale
                *direction = (dx > 0) ? GAUCHE : DROITE; // Si on est allé à droite, essayer à gauche et vice versa
                if (detecterCollision(lesX, lesY, plateau, *direction)) // Vérifie s'il y a une collision dans cette direction horizontale
                {
                    // Si collision, essayer l'autre direction verticale
                    *direction = (dy > 0) ? HAUT : BAS; // Si la cible est en bas, essayer de monter, sinon essayer de descendre
                }
            }
        }
    }
    
    // Si pas de déplacement vertical possible, essayer de se déplacer horizontalement (X)
    else if (dx != 0)
    {
        *direction = (dx > 0) ? DROITE : GAUCHE; // Si la cible est à droite, se diriger à droite, sinon à gauche
        if (detecterCollision(lesX, lesY, plateau, *direction)) // Vérifie s'il y a une collision dans cette direction
        {
            // Si collision, essayer la direction verticale (Y)
            *direction = (dy > 0) ? BAS : HAUT; // Si la cible est en bas, essayer de descendre, sinon monter
            if (detecterCollision(lesX, lesY, plateau, *direction)) // Vérifie s'il y a une collision dans cette direction verticale
            {
                // Si collision dans la direction verticale, essayer l'autre direction verticale
                *direction = (dy > 0) ? HAUT : BAS; // Si on est allé en bas, essayer de monter et vice versa
                if (detecterCollision(lesX, lesY, plateau, *direction)) // Vérifie s'il y a une collision dans cette direction
                {
                    // Si collision, essayer l'autre direction horizontale
                    *direction = (dx > 0) ? GAUCHE : DROITE; // Si on est allé à droite, essayer à gauche et vice versa
                }
            }
        }
    }
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

bool detecterCollision(int lesX[], int lesY[], tPlateau plateau, char directionProchaine)
{
    // Déclaration des variables pour la nouvelle position du serpent après déplacement
    int nouvelleX = lesX[0]; // Projeter des coordonnées en X
    int nouvelleY = lesY[0]; // Projeter des coordonnées en Y

    // Calcul de la nouvelle position en fonction de la direction donnée
    switch (directionProchaine)
    {
    case HAUT:
        nouvelleY--; // Déplacement vers le haut
        break;
    case BAS:
        nouvelleY++; // Déplacement vers le bas
        break;
    case GAUCHE:
        nouvelleX--; // Déplacement vers la gauche
        break;
    case DROITE:
        nouvelleX++; // Déplacement vers la droite
        break;
    }

    // Vérification des collisions avec les bords du plateau (bordure)
    if (plateau[nouvelleX][nouvelleY] == BORDURE)
    {
        return true; // Collision avec une bordure du plateau
    }

    // Vérification des collisions avec le corps du serpent (si une partie du corps est sur la nouvelle position)
    for (int i = 0; i < TAILLE; i++)
    {
        if (lesX[i] == nouvelleX && lesY[i] == nouvelleY)
        {
            return true; // Collision avec une partie du corps du serpent
        }
    }

    // Si aucune collision n'est détectée, retourner false
    return false; // Pas de collision
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
