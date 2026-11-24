#include <Arduino.h>
#include "SRF04Sonar/SRF04Sonar.cpp"

// put function declarations here:
const uint8_t ECHO_PIN = 7;
const uint8_t TRIG_PIN = 8;

const uint8_t RED_PIN = 9;
const uint8_t GREEN_PIN = 11;
const uint8_t BLUE_PIN = 10;

const float DIST_TRESHOLD = 15; // Valeur du treshold de proximité en cm.

SRF04Sonar sonar;
bool clear = true;

void set_color(uint8_t r, uint8_t g, uint8_t b) {
  r = 255 - r;
  g = 255 - g;
  b = 255 - b;
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  sonar.init(ECHO_PIN, TRIG_PIN);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  set_color(0, 0, 255); // Bleu au départ
}

void loop() {
  // put your main code here, to run repeatedly:
  float distance = sonar.getRange();

  if (clear && distance > 0 && distance <= DIST_TRESHOLD) {
    clear = false;
    Serial.println("Objet trop proche!");
    set_color(255, 255, 0);
  }
  else if (!clear && (distance == 0 || distance > DIST_TRESHOLD)) {
    clear = true;
    Serial.println("Clear!");
    set_color(0, 0, 255);
  }

  delay(100);
}
