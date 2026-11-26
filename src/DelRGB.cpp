#include "DelRGB.h"

void set_DELrgb_color(uint8_t r, uint8_t g, uint8_t b) {
  r = 255 - r;
  g = 255 - g;
  b = 255 - b;
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}

void setupDELrgb(){

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  set_DELrgb_color(0, 0, 255); // Bleu au départ
}