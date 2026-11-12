#ifndef SDDRIVER_H
#define SDDRIVER_H
//**************************************
// Projet: Labyrinthe-S1
// Fichiers: SDdriver.h
// Equipe: P13
// Auteurs: 
// Description: Breve description du script
// Date: 24/09/2026
//***************************************
#include <Arduino.h>
//*********DEFINES********//
#define CS_PIN 10
//*****VARIABLES GLOBALES*****//


//********FONCTIONS********//
void intitialisationSD(void);
String trouverLigne(int colonneCible, const String &valeurRecherchee);
void ecrireLigne(int numeroLigne, String nouvelleLigne);
void supprimerLigne(int numeroLigne);
void renommeFichier(void);
bool updateCSV(int numeroLigneCible, int colonneCible, String nouvelleValeur);
int trouverNo(void);
void fermerFichier(void);

#endif