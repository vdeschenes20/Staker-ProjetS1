#include <Arduino.h>
#include "inventaire_bluetooth/Bluetooth.h"

const uint8_t BOUTON_CHERCHER = 2;
const uint8_t BOUTON_STOCKER = 3;
const unsigned long DELAI_ANTI_REBOND_MS = 150;

struct Bouton {
  uint8_t broche;
  int etatLu;
  int etatStable;
  unsigned long dernierChangement;
};

Bouton chercher = {BOUTON_CHERCHER, HIGH, HIGH, 0};
Bouton stocker = {BOUTON_STOCKER, HIGH, HIGH, 0};

// Les boutons relient leur broche à GND pendant l'appui.
void initialiserBouton(Bouton& bouton) {
  pinMode(bouton.broche, INPUT_PULLUP);
  bouton.etatLu = digitalRead(bouton.broche);
  bouton.etatStable = bouton.etatLu;
  bouton.dernierChangement = millis();
}

// Détecte un seul appui sans ralentir la lecture Bluetooth.
bool nouvelAppui(Bouton& bouton) {
  int lecture = digitalRead(bouton.broche);
  unsigned long maintenant = millis();
  if (lecture != bouton.etatLu) {
    bouton.etatLu = lecture;
    bouton.dernierChangement = maintenant;
  }
  if (lecture == bouton.etatStable ||
      maintenant - bouton.dernierChangement < DELAI_ANTI_REBOND_MS) {
    return false;
  }
  bouton.etatStable = lecture;
  return lecture == LOW;
}

// Prépare la liaison et les deux entrées locales.
void setup() {
  initialiserBluetooth();
  initialiserBouton(chercher);
  initialiserBouton(stocker);
}

// La manette et les boutons passent par les mêmes fonctions d'inventaire.
void loop() {
  lireBluetooth();
  if (nouvelAppui(chercher)) {
    commandeLocale("CMD|LOCAL|CHERCHER|TYPE_TEXTE|A");
  }
  if (nouvelAppui(stocker)) {
    commandeLocale("CMD|LOCAL|STOCKER|-|A|4,1,1|Piece A");
  }
}
