#include "Sonar.h"
#include "DelRGB.h"

void mesureSonar(float p_distance, bool &p_clear) {
  if (p_clear && p_distance > 0 && p_distance <= DIST_LIMITE) {
    p_clear = false;
    Serial.println("Objet trop proche!");
    set_DELrgb_color(255, 255, 0);
  }
  else if (!p_clear && (p_distance == 0 || p_distance > DIST_LIMITE)) {
    p_clear = true;
    Serial.println("Clear!");
    set_DELrgb_color(0, 0, 255);
  }
}