#include <Arduino.h>
#include <Servo.h>
#include "Sonar.h"
#include "DelRGB.h"
#include "SRF04Sonar.h"
#include "ServoPince.h"

SRF04Sonar sonar;
Servo pince;
bool clear;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  sonar.init(ECHO_PIN, TRIG_PIN);
  setupDELrgb();
  setupServoPince();
  clear = true;
}

void loop() {
  // put your main code here, to run repeatedly:
  float distance = sonar.getRange();
  mesureSonar(distance, clear);
  delay(100);
}