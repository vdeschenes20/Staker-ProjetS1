//**************************************
// Projet: Labyrinthe-S1
// Fichiers: Commande.h
// Equipe: P13
// Auteurs: 
// Description: Breve description du script
// Date: 24/09/2026
//***************************************

#pragma once
#include <Arduino.h>

// Ligne reçue du PC. Les arguments restent du texte pour garder les zéros.
struct Commande {
    String id;
    String action;
    String args[4];
    uint8_t nombreArgs = 0;
};

bool decoderCommande(const String& ligne, Commande& commande);
