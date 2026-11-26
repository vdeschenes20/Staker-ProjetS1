#include <Arduino.h>
#include "Sonar.h"
#include "DelRGB.h"
#include "SRF04Sonar.h"

SRF04Sonar sonar;
bool clear;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //SRF04Sonar sonar = creerSonar();
  sonar.init(ECHO_PIN, TRIG_PIN);
  setupDELrgb();
  clear = true;
}

void loop() {
  // put your main code here, to run repeatedly:
  float distance = sonar.getRange();
  mesureSonar(distance, clear);
  delay(100);
}
