#include <Arduino.h>
#include "Bluetooth.h"
#include "Commande.h"

// Variables d'origine. Les compteurs restent disponibles pour les essais D2/D3.
Commande commande;
int nb = 0;
int nb2 = 25;

// Démarre le journal USB et le lecteur de commandes du HC-05.
// Bluetooth.cpp configure Serial2. Éviter BluetoothInit() de LibRobus,
// qui reconfigure Serial2 à un autre débit.
void setup() {
  Serial.begin(9600);
  initialiserBluetooth();
  pinMode(2, INPUT);
  pinMode(3, INPUT);
}

// Reçoit une commande, la valide et renvoie le résultat à la manette.
void loop() {
  lireBluetooth();
}
