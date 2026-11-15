// librairies
#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include "capteurCouleur.h"

// ---------------------------------------------------------------
// CAPTEUR
// ---------------------------------------------------------------
Adafruit_TCS34725 capteur = Adafruit_TCS34725(
    TCS34725_INTEGRATIONTIME_50MS,
    TCS34725_GAIN_4X
);

bool initialiserCapteurCouleur() {
    return capteur.begin();
}

//---------------------------------------------------------------
// Lecture brute du capteur
//lit le capteur et retourne les valeurs brutes RGB
//---------------------------------------------------------------
Couleur lireCapteur() {
    Couleur c;

    uint16_t rouge, vert, bleu, clair;  // "clair" pas utilisé pour l'instant,
                                        // mais utile pour la luminosité si besoin
    capteur.getRawData(&rouge, &vert, &bleu, &clair);

    c.R = rouge;
    c.G = vert;
    c.B = bleu;

    return c;
}

// ---------------------------------------------------------------
// MÉMOIRE DES ÉCHANTILLONS
//crée un tableau pour stocker les dernières lectures brutes RGB du capteur
// ---------------------------------------------------------------
static Couleur echantillons[NB_ECHANTILLONS];
static int indexEchantillon = 0;
static int nbRecues = 0;

// Ajoute une lecture à la mémoire (écrase la plus ancienne quand c'est plein)
void ajouterEchantillon(Couleur c) {
    echantillons[indexEchantillon] = c;
    indexEchantillon = (indexEchantillon + 1) % NB_ECHANTILLONS;
    if (nbRecues < NB_ECHANTILLONS) {
        nbRecues++;
    }
}

// Remet la mémoire à zéro (à appeler avant une nouvelle série de mesures)
void viderEchantillons() {
    indexEchantillon = 0;
    nbRecues = 0;
}

// ---------------------------------------------------------------
// CALCUL DE LA COULEUR MOYENNE
//fait la moyenne des lectures stockées dans le tableau echantillons[]
// ---------------------------------------------------------------
Couleur moyenneCouleur() {
    Couleur somme = {0, 0, 0};
    for (int i = 0; i < nbRecues; i++) {
        somme.R += echantillons[i].R;
        somme.G += echantillons[i].G;
        somme.B += echantillons[i].B;
    }
    if (nbRecues > 0) {
        somme.R /= nbRecues;
        somme.G /= nbRecues;
        somme.B /= nbRecues;
    }
    return somme;
}

// ---------------------------------------------------------------
// TABLEAU DES SEUILS (rempli automatiquement par la calibration)
// ---------------------------------------------------------------
struct Seuil {
    CouleurId id;
    float minR, maxR;
    float minG, maxG;
    float minB, maxB;
};

#define MARGE 0.10f   // marge de 10 % autour des min/max mesurés

// Valeurs par défaut (anciennes mesures), écrasées quand on calibre.
// L'ordre est le même que dans modeCalibration() : ROSE, JAUNE, ORANGE, BLEU, VERT

static Seuil seuils[NB_COULEURS] = {      //NB_couleur est le nombre de couleurs à 
                                         //reconnaître
    {ROSE,   312, 409, 288, 384, 181, 233},
    {JAUNE,  340, 413, 422, 517, 228, 279},
    {ORANGE, 369, 469,  29, 378, 265, 332},
    {BLEU,   309, 364, 313, 369, 156, 184},
    {VERT,   405, 481, 257, 298, 334, 391}
};

// ---------------------------------------------------------------
// IDENTIFICATION DE LA COULEUR
// Compare la couleur moyenne aux plages du tableau de seuils
//Prend la décision sur la couleur la plus proche de la moyenne
// ---------------------------------------------------------------
CouleurId identifierCouleur(Couleur m) {
    for (int i = 0; i < NB_COULEURS; i++) {
        if (m.R >= seuils[i].minR && m.R <= seuils[i].maxR &&
            m.G >= seuils[i].minG && m.G <= seuils[i].maxG &&
            m.B >= seuils[i].minB && m.B <= seuils[i].maxB) {
            return seuils[i].id;
        }
    }    // aucune couleur ne correspond aux seuils return AUCUN
    return AUCUN;
}

// ---------------------------------------------------------------
// CALIBRATION DE LA COULEUR
// Lit le capteur pendant duree_ms, puis écrit les min/max (avec marge)
// directement dans seuils[index].
// À utiliser pour calibrer seulement, pas dans le programme final.
// ---------------------------------------------------------------
void calibrerCouleur(const char* nom, int index, int duree_ms) {
    float minR = 1e9, minG = 1e9, minB = 1e9;
    float maxR = 0,   maxG = 0,   maxB = 0;
    float sommeR = 0, sommeG = 0, sommeB = 0;

    int n = duree_ms / 50;            // nombre de lectures (50 ms entre chaque)

    for (int i = 0; i < n; i++) {
        Couleur c = lireCapteur();

        if (c.R < minR) minR = c.R;
        if (c.G < minG) minG = c.G;
        if (c.B < minB) minB = c.B;
        if (c.R > maxR) maxR = c.R;
        if (c.G > maxG) maxG = c.G;
        if (c.B > maxB) maxB = c.B;

        sommeR += c.R;
        sommeG += c.G;
        sommeB += c.B;
        delay(50);
    }

    if (n == 0) return;

    // Écriture automatique dans le tableau de seuils avec une marge de sécurité
    seuils[index].minR = minR * (1.0f - MARGE);
    seuils[index].maxR = maxR * (1.0f + MARGE);
    seuils[index].minG = minG * (1.0f - MARGE);
    seuils[index].maxG = maxG * (1.0f + MARGE);
    seuils[index].minB = minB * (1.0f - MARGE);
    seuils[index].maxB = maxB * (1.0f + MARGE);

    // Affichage dans le moniteur série pour vérification
    Serial.print(nom);
    Serial.print(" | R: "); Serial.print(minR); Serial.print(" a "); Serial.print(maxR);
    Serial.print(" (moy "); Serial.print(sommeR / n); Serial.print(")");
    Serial.print(" | G: "); Serial.print(minG); Serial.print(" a "); Serial.print(maxG);
    Serial.print(" (moy "); Serial.print(sommeG / n); Serial.print(")");
    Serial.print(" | B: "); Serial.print(minB); Serial.print(" a "); Serial.print(maxB);
    Serial.print(" (moy "); Serial.print(sommeB / n); Serial.println(")");
}



// ---------------------------------------------------------------
// MODE CALIBRATION
//fonction a appeler dans setup() pour calibrer toutes les couleurs
// ---------------------------------------------------------------
void modeCalibration() {
    // Même ordre que le tableau seuils[]
    const char* noms[NB_COULEURS] = {"Rose", "Jaune", "Orange", "Bleu", "Vert"};

    for (int i = 0; i < NB_COULEURS; i++) {
        Serial.print("Place le capteur sur : ");
        Serial.print(noms[i]);
        Serial.println(" puis envoie une lettre");

        while (!Serial.available()) {}              // attend que tu envoies un caractère
        while (Serial.available()) Serial.read();   // vide ce que tu as envoyé

        calibrerCouleur(noms[i], i, 3000);          // lit pendant 3 secondes et remplit seuils[i]
    }
      // Affiche le tableau final (utile pour copier-coller dans le code)
    Serial.println("--- Tableau de seuils ---");
    for (int i = 0; i < NB_COULEURS; i++) {
        Serial.print("{"); Serial.print(noms[i]); Serial.print(", ");
        Serial.print(seuils[i].minR); Serial.print(", "); Serial.print(seuils[i].maxR); Serial.print(", ");
        Serial.print(seuils[i].minG); Serial.print(", "); Serial.print(seuils[i].maxG); Serial.print(", ");
        Serial.print(seuils[i].minB); Serial.print(", "); Serial.print(seuils[i].maxB);
        Serial.println("},");
    }
    Serial.println("Calibration terminee.");
}



// ---------------------------------------------------------------
// DÉBOGAGE
// Change l'identifiant de couleur en texte pour l'affichage
// ---------------------------------------------------------------
String nomCouleur(CouleurId id) {
    switch (id) {
        case ROSE:   return "Rose";
        case JAUNE:  return "Jaune";
        case ORANGE: return "Orange";
        case BLEU:   return "Bleu";
        case VERT:   return "Vert";
        default:     return "Aucune";
    }
}