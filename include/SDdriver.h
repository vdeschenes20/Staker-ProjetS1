//**************************************
// Projet: Labyrinthe-S1
// Fichiers: SDdriver.h
// Equipe: P13
// Auteurs: 
// Description: Breve description du script
// Date: 24/09/2026
//***************************************

#pragma once
#include <SdFat.h>
#include "DataBase.h"

constexpr uint8_t CS_PIN = 10;

// Résultat distinct pour fin de fichier et ligne invalide.
enum Lecture { LIGNE, FIN_CSV, ERREUR_CSV };

bool initialiserSD();
bool ouvrirCSV(File32& fichier);
Lecture lirePiece(File32& fichier, Piece& piece);
bool ouvrirCopieCSV(File32& copie);
bool ecrirePiece(File32& copie, const Piece& piece);
bool terminerCopieCSV(File32& copie, bool conserver);
