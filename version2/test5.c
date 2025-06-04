char choisirDirection(int x, int y, int cibleX, int cibleY, tPlateau plateau) {
    int distNormale, distTrou;
    char meilleureDirection = DROITE;
    int meilleureDistance = LARGEUR_PLATEAU + HAUTEUR_PLATEAU;

    // Calcul de la distance pour chaque direction
    // DROITE
    distNormale = abs((x + 1) % LARGEUR_PLATEAU - cibleX) + abs(y - cibleY);
    distTrou = abs(1 - cibleX) + abs(y - cibleY); // Sortie du trou
    if (distNormale < meilleureDistance || distTrou < meilleureDistance) {
        meilleureDirection = DROITE;
        meilleureDistance = (distNormale < distTrou) ? distNormale : distTrou;
    }

    // GAUCHE
    distNormale = abs((x - 1 + LARGEUR_PLATEAU) % LARGEUR_PLATEAU - cibleX) + abs(y - cibleY);
    distTrou = abs(LARGEUR_PLATEAU - 2 - cibleX) + abs(y - cibleY);
    if (distNormale < meilleureDistance || distTrou < meilleureDistance) {
        meilleureDirection = GAUCHE;
        meilleureDistance = (distNormale < distTrou) ? distNormale : distTrou;
    }

    // BAS
    distNormale = abs(x - cibleX) + abs((y + 1) % HAUTEUR_PLATEAU - cibleY);
    distTrou = abs(x - cibleX) + abs(1 - cibleY);
    if (distNormale < meilleureDistance || distTrou < meilleureDistance) {
        meilleureDirection = BAS;
        meilleureDistance = (distNormale < distTrou) ? distNormale : distTrou;
    }

    // HAUT
    distNormale = abs(x - cibleX) + abs((y - 1 + HAUTEUR_PLATEAU) % HAUTEUR_PLATEAU - cibleY);
    distTrou = abs(x - cibleX) + abs(HAUTEUR_PLATEAU - 2 - cibleY);
    if (distNormale < meilleureDistance || distTrou < meilleureDistance) {
        meilleureDirection = HAUT;
        meilleureDistance = (distNormale < distTrou) ? distNormale : distTrou;
    }

    return meilleureDirection;
}



direction = choisirDirection(lesX[0], lesY[0], lesPommesX[nbPommes], lesPommesY[nbPommes], lePlateau);
