#include <Arduino.h>
#include <Servo.h>

const int SERVO_PIN = 6;

const int angleOuvert = 5;  // À ajuster avec nouvelle pince
const int angleFerme = 75;  // À ajuster avec nouvelle pince

extern Servo pince;

void setupServoPince();