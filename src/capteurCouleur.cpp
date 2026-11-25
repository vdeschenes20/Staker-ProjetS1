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
// TABLEAU DES SEUILS 
// ---------------------------------------------------------------
struct Seuil {
    CouleurId id;
    float minR, maxR;
    float minG, maxG;
    float minB, maxB;
};



static Seuil seuils[NB_COULEURS] = {      //NB_couleur est le nombre de couleurs à reconnaître
    {ROUGE, 417.60, 513.70, 286.20, 354.20, 279.90, 346.50},
{JAUNE, 593.10, 726.00, 651.60, 797.50, 392.40, 480.70},
{BLEU, 166.50, 204.60, 274.50, 336.60, 326.70, 400.40},
{VERT, 195.30, 239.80, 405.90, 496.10, 317.70, 389.40},
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
// Mode de décision 
// Lit le capteur plusieurs fois et prend la décision par vote
//***fonction à appeler pour avoir la couleur identifiée par le capteur***
// ---------------------------------------------------------------

// Vote sur un tableau de lectures déjà prises
static CouleurId voter(const CouleurId lectures[]) {
    for (int i = 0; i < NB_LECTURES; i++) {
        int votes = 0;
        for (int j = 0; j < NB_LECTURES; j++) {
            if (lectures[j] == lectures[i]) votes++;
        }
        if (votes >= SEUIL_VOTES && lectures[i] != AUCUN) {
            return lectures[i];
        }
    }
    return AUCUN;
}

// Prend UNE mesure par appel et la met dans un buffer circulaire
// (la nouvelle remplace la plus ancienne).
// Retourne la couleur qui a au moins SEUIL_VOTES votes sur les
// NB_LECTURES dernières mesures, sinon AUCUN.
CouleurId mesurerCouleur() {
    static CouleurId lectures[NB_LECTURES];
    static int index = 0;   // où écrire la prochaine mesure
    static int nb = 0;      // combien de mesures valides au départ

    lectures[index] = identifierCouleur(lireCapteur());
    index = (index + 1) % NB_LECTURES;

    if (nb < NB_LECTURES) nb++;
    if (nb < NB_LECTURES) return AUCUN;   // buffer pas encore plein

    return voter(lectures);
}




// ---------------------------------------------------------------
// CALIBRATION DE LA COULEUR
// À utiliser pour calibrer seulement, pas dans le programme final.
// ---------------------------------------------------------------
void calibrerCouleur(const char* nom, int index, int duree_ms) {
    float minR = 1e9, minG = 1e9, minB = 1e9;
    float maxR = 0,   maxG = 0,   maxB = 0;
    float sommeR = 0, sommeG = 0, sommeB = 0;

    int n = duree_ms / 50;           

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

    // Affichage dans le moniteur série pour vérif
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
 
    const char* noms[NB_COULEURS] = {"ROUGE", "JAUNE", "BLEU", "VERT"};

    for (int i = 0; i < NB_COULEURS; i++) {
        Serial.print("Place le capteur sur : ");
        Serial.print(noms[i]);
        Serial.println(" puis envoie une lettre");

        while (!Serial.available()) {}              // attend que tu envoies un caractère
        while (Serial.available()) Serial.read();   // vide ce que tu as envoyé

        calibrerCouleur(noms[i], i, 3000);          // lit pendant 3 secondes (peux changer la durée si besoin)
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
        case ROUGE:   return "ROUGE";
        case JAUNE:  return "JAUNE";
        case BLEU:   return "BLEU";
        case VERT:   return "VERT";
        default:     return "Aucune";
    }
}