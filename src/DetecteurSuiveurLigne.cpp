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
bool bandeEpaisseTraitee = false;

void rouler(float vitesseGauche, float vitesseDroite) {
  MOTOR_SetSpeed(LEFT, vitesseGauche);
  MOTOR_SetSpeed(RIGHT, vitesseDroite);
}

} // namespace

void reinitialiserSuiveurLigne() {
  dernierVirage = TOUT_DROIT;
  bandeEpaisseTraitee = false;
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
  if (position != 7) {
    bandeEpaisseTraitee = false;
  }

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
    case 1:
      dernierVirage = DROITE;
      rouler(VITESSE_EXTERIEURE, VITESSE_INTERIEURE);
      break;
    case 2:
      rouler(VITESSE_AVANCE, VITESSE_AVANCE);
      break;
    case 3:
      dernierVirage = DROITE;
      rouler(0.24f, 0.17f);
      break;
    case 4:
      dernierVirage = GAUCHE;
      rouler(VITESSE_INTERIEURE, VITESSE_EXTERIEURE);
      break;
    case 5:
      // Deux capteurs voient la ligne : traverser droit.
      rouler(VITESSE_AVANCE, VITESSE_AVANCE);
      break;
    case 6:
      dernierVirage = GAUCHE;
      rouler(0.17f, 0.24f);
      break;
    case 7:
      if (!bandeEpaisseTraitee) {
        bandeEpaisseTraitee = true;
        mouvementAvant(85.0f);
      } else {
        rouler(VITESSE_AVANCE, VITESSE_AVANCE);
      }
      break;
  }
}
