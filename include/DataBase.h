//**************************************
// Projet: Labyrinthe-S1
// Fichiers: DataBase.h
// Equipe: P13
// Auteurs: 
// Description: Breve description du script
// Date: 24/09/2026
//***************************************

#pragma once
#include <Arduino.h>
#include "Commande.h"

// Une ligne du CSV. '-' dans etat indique une case libre.
struct Piece {
    String numero, type, position, etat, nom;
};

using SortiePiece = void (*)(const Piece&);
const char* listerInventaire(const Commande* recherche, SortiePiece sortie);
const char* gestionDonnees(const Commande& commande);
