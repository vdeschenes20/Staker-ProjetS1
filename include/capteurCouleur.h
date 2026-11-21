#ifndef CAPTEUR_COULEUR_H
#define CAPTEUR_COULEUR_H

#include <Arduino.h>

// ---------------------------------------------------------------
// CONSTANTES
// ---------------------------------------------------------------
#define NB_COULEURS     5    // nombre de couleurs à reconnaître
#define MARGE          0.10f // marge de sécurité autour des min/max mesurés
#define NB_LECTURES     10   // nombre de lectures pour la décision finale
#define SEUIL_VOTES     8    // nombre de lectures identiques requis pour prendre une décision

// ---------------------------------------------------------------
// TYPES
// ---------------------------------------------------------------

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


// Identification
CouleurId identifierCouleur(Couleur m);
String nomCouleur(CouleurId id);
CouleurId decisionCouleur();

// Calibration (remplit automatiquement le tableau de seuils)
void calibrerCouleur(const char* nom, int index, int duree_ms);
void modeCalibration();

#endif