#include <Arduino.h>
#include "DataBase.h"
#include "Commande.h"
#include "SDdriver.h"

// put function declarations here:
Commande commande;
int nb = 0;
int nb2 = 25;
void setup() {
  initialisationCommande();
  initialsationDonnees();
  pinMode(2, INPUT);
  pinMode(3, INPUT);
}

void loop() {
  static int lastButton1State = HIGH;
  static int lastButton2State = HIGH;
  int buttonState = digitalRead(2);
  int buttonState2 = digitalRead(3);
  if (buttonState == LOW && lastButton1State == HIGH) {
    delay(150);
    if (digitalRead(2) != LOW) {
      lastButton1State = digitalRead(2);
      return;
    }

    nb++;
    commande.type = CHERCHER;
    commande.parametre1 = TYPE;
    commande.parametre2 = 1;
    commande.position[0] = 0;
    commande.position[1] = 0;
    commande.position[2] = 0;
    gestionDonnees(commande);
  }
  if(buttonState2 == LOW && lastButton2State == HIGH) {
    delay(150);
    if (digitalRead(3) != LOW) {
      lastButton2State = digitalRead(3);
      return;
    }
    nb2++;
    commande.type = STOCKER;
    commande.parametre1 = 0;
    commande.parametre2 = 1;
    commande.position[0] = 4;
    commande.position[1] = 1;
    commande.position[2] = 1;
    gestionDonnees(commande);
  }
  lastButton1State = digitalRead(2);
  lastButton2State = digitalRead(3);
}
