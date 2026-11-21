#include <Arduino.h>
#include <LibRobus.h>
#include <Mouvement.h>
#include <DetecteurSuiveurLigne.h>

namespace {
// Capteurs vus dans le sens de marche du robot.
const byte CAPTEUR_GAUCHE = A4;
const byte CAPTEUR_CENTRE = A5;
const byte CAPTEUR_DROITE = A6;

// Si la mesure atteint le seuil, le capteur voit la ligne noire.
const int SEUIL_GAUCHE = 105;
const int SEUIL_CENTRE = 146;
const int SEUIL_DROITE = 77;

const float VITESSE_AVANCE = 0.23f;
const float VITESSE_INTERIEURE = 0.12f;
const float VITESSE_EXTERIEURE = 0.27f;

enum Direction { TOUT_DROIT, GAUCHE, DROITE };
Direction dernierVirage = TOUT_DROIT;

void rouler(float vitesseGauche, float vitesseDroite) {
  MOTOR_SetSpeed(LEFT, vitesseGauche);
  MOTOR_SetSpeed(RIGHT, vitesseDroite);
}

} // namespace

void reinitialiserSuiveurLigne() {
  dernierVirage = TOUT_DROIT;
}

void initialiserSuiveurLigne() {
  pinMode(CAPTEUR_GAUCHE, INPUT);
  pinMode(CAPTEUR_CENTRE, INPUT);
  pinMode(CAPTEUR_DROITE, INPUT);
}

int detecterPositionRobot() {
  const bool gauche = analogRead(CAPTEUR_GAUCHE) >= SEUIL_GAUCHE;
  const bool centre = analogRead(CAPTEUR_CENTRE) >= SEUIL_CENTRE;
  const bool droite = analogRead(CAPTEUR_DROITE) >= SEUIL_DROITE;
  return (gauche << 2) | (centre << 1) | droite;
}

bool ligneCentree() {
  return (detecterPositionRobot() & 2) != 0;
}

void suivreLigne() {
  const int position = detecterPositionRobot();


  switch (position) {
    case 0: {
      // Seul le motif sans capteur actif signifie que la ligne est perdue.
      Direction recherche = dernierVirage;
      if (recherche == TOUT_DROIT) recherche = GAUCHE;

      if (recherche == GAUCHE) {
        rouler(0.08f, 0.19f);
      } else {
        rouler(0.19f, 0.08f);
      }
      break;
    }
    case 1:     //virage à droite fort (capteur droit seul)
      dernierVirage = DROITE;
      rouler(VITESSE_EXTERIEURE, VITESSE_INTERIEURE);
      break;
      
    case 2:   // ligne centrée (capteur centre seul)
      rouler(VITESSE_AVANCE, VITESSE_AVANCE);
      break;

    case 3:  // virage à droite léger (capteur centre et droit)
      dernierVirage = DROITE;
      rouler(0.24f, 0.17f);
      break
      ;
    case 4:   // virage à gauche fort (capteur gauche seul)
      dernierVirage = GAUCHE;
      rouler(VITESSE_INTERIEURE, VITESSE_EXTERIEURE);
      break;

    case 5:     // traverser droit. (capteur gauche et droit)
      rouler(VITESSE_AVANCE, VITESSE_AVANCE);
      break;

    case 6:  // virage à gauche léger (capteur gauche et centre)
      dernierVirage = GAUCHE;
      rouler(0.17f, 0.24f);
      break;

    case 7:  // avance (tous les capteurs actifs)
        rouler(VITESSE_AVANCE, VITESSE_AVANCE);
      break;
  }
}
