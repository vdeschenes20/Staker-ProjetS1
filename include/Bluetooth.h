#pragma once

#include <Arduino.h>

void initialiserBluetooth();
void lireBluetooth();
void commandeLocale(const char* ligne);

// Exemple après initialiserBluetooth() : envoyerTexteBluetooth("Objet saisi");
void envoyerTexteBluetooth(const String& texte);
