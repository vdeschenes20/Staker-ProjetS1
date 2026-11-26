#include "Sonar.h"
#include "DelRGB.h"
#include "ServoPince.h"

void mesureSonar(float p_distance, bool &p_clear) {
  if (p_clear && p_distance > 0 && p_distance <= DIST_LIMITE) { // Lorsque objet
    p_clear = false;
    Serial.println("Objet trop proche!");
    set_DELrgb_color(255, 255, 0);
    pince.write(angleOuvert);
    Serial.println("Pince ouverte");
  }
  else if (!p_clear && (p_distance == 0 || p_distance > DIST_LIMITE)) { // Lorsque redevient clear
    p_clear = true;
    Serial.println("Clear!");
    set_DELrgb_color(0, 0, 255);
    pince.write(angleFerme);
    Serial.println("Pince fermee");
  }
}