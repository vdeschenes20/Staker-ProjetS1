#pragma once

struct MesuresCapteursSuiveurLigne {
  int gauche;
  int centre;
  int droite;
};

void initialiserSuiveurLigne();
MesuresCapteursSuiveurLigne lireCapteursSuiveurLigne();
void CalibrationSuiveurLigne();
int detecterPositionRobot();
void reinitialiserSuiveurLigne();
bool ligneCentree();
void suivreLigne();
