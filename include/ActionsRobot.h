#pragma once

#include <Arduino.h>

bool allerPlacer(const String& numero, const String& type, const String& position,
                 const String& etat);
bool allerChercher(const String& numero, const String& type, const String& position,
                   const String& etat);
void arretUrgence();
