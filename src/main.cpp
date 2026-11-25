#include "DataBase.h"
#include "Commande.h"
#include "SDdriver.h"
#include "capteurCouleur.h"
#include "DetecteurSuiveurLigne.h"
#include "LibRobus.h"
#include "Mouvement.h"

void setup() {
  BoardInit();
  Serial.begin(9600);

  initialiserCapteurCouleur();
  //modeCalibration();   // à appeler dans setup() pour calibrer
                         // (mettre en commentaire après calibration)

  initialiserSuiveurLigne();
  reinitialiserSuiveurLigne();
}

void loop() {
  static bool arrete = false;
  static CouleurId derniere = AUCUN;

  if (arrete) return;   // robot arrêté, on ne touche plus aux moteurs

  suivreLigne();

  CouleurId couleur = mesurerCouleur();

  // réagit seulement quand une nouvelle couleur est confirmée
  if (couleur != AUCUN && couleur != derniere) {
    Serial.println(nomCouleur(couleur));
  }
  derniere = couleur;

  // if (couleur == VERT || couleur == BLEU || couleur == ROUGE || couleur == JAUNE) {
  //   MOTOR_SetSpeed(LEFT, 0.0f);
  //   MOTOR_SetSpeed(RIGHT, 0.0f);
  //   arrete = true;
  // }

}