#include <Arduino.h>
#include <Servo.h>

Servo pince;

const int brocheServo = 6;
const int angleOuvert = 5;  // À ajuster selon votre pince
const int angleFerme = 75;     // À ajuster selon votre pince

void setup() {
  pince.attach(brocheServo);
  pince.write(angleOuvert);

  Serial.begin(9600);
  Serial.println("Entrez o pour ouvrir ou f pour fermer :");
}

void loop() {
  if (Serial.available() > 0) {
    char commande = Serial.read();

    if (commande == 'o' || commande == 'O') {
      pince.write(angleOuvert);
      Serial.println("Pince ouverte");
    } else if (commande == 'f' || commande == 'F') {
      pince.write(angleFerme);
      Serial.println("Pince fermee");
    }
  }
}