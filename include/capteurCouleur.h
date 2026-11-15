#ifndef CAPTEUR_COULEUR_H
#define CAPTEUR_COULEUR_H

#include <Arduino.h>

// ---------------------------------------------------------------
// CONSTANTES
// ---------------------------------------------------------------
#define NB_ECHANTILLONS 10   // nombre de lectures gardées pour la moyenne
#define NB_COULEURS     5    // nombre de couleurs à reconnaître

// ---------------------------------------------------------------
// TYPES
// ---------------------------------------------------------------

// Une lecture du capteur (valeurs brutes rouge, vert, bleu)
struct Couleur {
    float R;
    float G;
    float B;
};

// Identifiant de chaque couleur
enum CouleurId {
    AUCUN = 0,
    ROSE,
    JAUNE,
    ORANGE,
    BLEU,
    VERT
};

// ---------------------------------------------------------------
// FONCTIONS
// ---------------------------------------------------------------

// Capteur
bool initialiserCapteurCouleur();   // à appeler dans setup(), retourne false si capteur introuvable
Couleur lireCapteur();              // une lecture brute RGB

// Mémoire des échantillons
void ajouterEchantillon(Couleur c);
void viderEchantillons();
Couleur moyenneCouleur();

// Identification
CouleurId identifierCouleur(Couleur m);
String nomCouleur(CouleurId id);

// Calibration (remplit automatiquement le tableau de seuils)
void calibrerCouleur(const char* nom, int index, int duree_ms);
void modeCalibration();

#endif