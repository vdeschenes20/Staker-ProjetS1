#ifndef MOUVEMENT_H
#define MOUVEMENT_H

extern int etat;
extern int etatPast;
extern float vitesse_Gauche;
extern float vitesse_Droite;


enum EtatRobot {
  ETAT_ARRET = 0,
  ETAT_AVANCE = 1,
  ETAT_TOURNE_DROITE = 2,
  ETAT_TOURNE_GAUCHE = 3,
  ETAT_RECULE = 4
};

void arret();
void tourne(float angleDegres);
void mouvementAvant(float DistanceCM);
void mouvementArriere(float DistanceCM);
void annulerMouvement();
void avanceMaj();
void avancePlus();
void tourneUneRoue(int direction, int vitesse);
void continuer();
void avance(int vitesse);

#endif