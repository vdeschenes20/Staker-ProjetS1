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
const float VITESSE_EXTERIEURE = 0.12f;

enum Direction { TOUT_DROIT, GAUCHE, DROITE };
Direction dernierVirage = TOUT_DROIT;

struct AccumulateurCapteur {
  unsigned long somme;
  int minimum;
  int maximum;
};

struct StatistiquesCapteur {
  float moyenne;
  int minimum;
  int maximum;
};

struct StatistiquesCalibration {
  StatistiquesCapteur gauche;
  StatistiquesCapteur centre;
  StatistiquesCapteur droite;
};

void ajouterMesure(AccumulateurCapteur &accumulateur, int valeur) {
  accumulateur.somme += valeur;
  if (valeur < accumulateur.minimum) accumulateur.minimum = valeur;
  if (valeur > accumulateur.maximum) accumulateur.maximum = valeur;
};

void attendreEntreeSerie(const __FlashStringHelper *message) {
  while (Serial.available() > 0) Serial.read();
  Serial.println(message);
  while (Serial.available() == 0) delay(10);
  while (Serial.available() > 0) Serial.read();
}

StatistiquesCalibration mesurerStatistiquesCapteurs() {
  AccumulateurCapteur gauche = {0, 1023, 0};
  AccumulateurCapteur centre = {0, 1023, 0};
  AccumulateurCapteur droite = {0, 1023, 0};
  unsigned int nombreMesures = 0;
  const unsigned long debut = millis();

  while (millis() - debut < 5000UL) {
    const MesuresCapteursSuiveurLigne mesures = lireCapteursSuiveurLigne();
    ajouterMesure(gauche, mesures.gauche);
    ajouterMesure(centre, mesures.centre);
    ajouterMesure(droite, mesures.droite);
    ++nombreMesures;
    delay(10);
  }

  return {
    {
      static_cast<float>(gauche.somme) / nombreMesures,
      gauche.minimum,
      gauche.maximum
    },
    {
      static_cast<float>(centre.somme) / nombreMesures,
      centre.minimum,
      centre.maximum
    },
    {
      static_cast<float>(droite.somme) / nombreMesures,
      droite.minimum,
      droite.maximum
    }
  };
}

void afficherStatistiquesCapteur(
    const __FlashStringHelper *nom,
    const StatistiquesCapteur &blanc,
    const StatistiquesCapteur &noir) {
  Serial.print(nom);
  Serial.print(F(" | blanc moy: "));
  Serial.print(blanc.moyenne, 1);
  Serial.print(F(" min: "));
  Serial.print(blanc.minimum);
  Serial.print(F(" max: "));
  Serial.print(blanc.maximum);
  Serial.print(F(" | noir moy: "));
  Serial.print(noir.moyenne, 1);
  Serial.print(F(" min: "));
  Serial.print(noir.minimum);
  Serial.print(F(" max: "));
  Serial.println(noir.maximum);
}

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

MesuresCapteursSuiveurLigne lireCapteursSuiveurLigne() {
  return {
    analogRead(CAPTEUR_GAUCHE),
    analogRead(CAPTEUR_CENTRE),
    analogRead(CAPTEUR_DROITE)
  };
}

void CalibrationSuiveurLigne() {
  attendreEntreeSerie(F("Placez les capteurs sur le BLANC, puis appuyez sur Entree."));
  Serial.println(F("Mesure du blanc pendant 5 secondes..."));
  const StatistiquesCalibration blanc = mesurerStatistiquesCapteurs();

  attendreEntreeSerie(F("Placez les capteurs sur le NOIR, puis appuyez sur Entree."));
  Serial.println(F("Mesure du noir pendant 5 secondes..."));
  const StatistiquesCalibration noir = mesurerStatistiquesCapteurs();

  Serial.println();
  Serial.println(F("Calibration: moyenne, minimum et maximum"));
  afficherStatistiquesCapteur(F("Gauche"), blanc.gauche, noir.gauche);
  afficherStatistiquesCapteur(F("Centre"), blanc.centre, noir.centre);
  afficherStatistiquesCapteur(F("Droite"), blanc.droite, noir.droite);
}

int detecterPositionRobot() {
  const MesuresCapteursSuiveurLigne mesures = lireCapteursSuiveurLigne();
  const bool gauche = mesures.gauche >= SEUIL_GAUCHE;
  const bool centre = mesures.centre >= SEUIL_CENTRE;
  const bool droite = mesures.droite >= SEUIL_DROITE;
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
      rouler(VITESSE_EXTERIEURE, -VITESSE_INTERIEURE);
      break;
      
    case 2:   // ligne centrée (capteur centre seul)
      rouler(VITESSE_AVANCE, VITESSE_AVANCE);
      break;

    case 3:  // virage à droite léger (capteur centre et droit)
      dernierVirage = DROITE;
      rouler(0.17f, -0.17f);
      break
      ;
    case 4:   // virage à gauche fort (capteur gauche seul)
      dernierVirage = GAUCHE;
      rouler(-VITESSE_INTERIEURE, VITESSE_EXTERIEURE);
      break;

    case 5:     // traverser droit. (capteur gauche et droit)
      rouler(VITESSE_AVANCE, VITESSE_AVANCE);
      break;

    case 6:  // virage à gauche léger (capteur gauche et centre)
      dernierVirage = GAUCHE;
      rouler(-0.17f, 0.17f);
      break;

    case 7:  // avance (tous les capteurs actifs)
        rouler(VITESSE_AVANCE, VITESSE_AVANCE);
      break;
  }
}
