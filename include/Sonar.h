#include <Arduino.h>
#include "SRF04Sonar.h"


// put function declarations here:
const uint8_t ECHO_PIN = 7;
const uint8_t TRIG_PIN = 8;

extern SRF04Sonar sonar;

const float DIST_LIMITE = 15; // Valeur du treshold de proximité en cm.

void mesureSonar(float p_distance, bool& p_clear);
