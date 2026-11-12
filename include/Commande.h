#ifndef COMMANDE_H
#define COMMANDE_H
//**************************************
// Projet: Labyrinthe-S1
// Fichiers: Commande.h
// Equipe: P13
// Auteurs: 
// Description: Breve description du script
// Date: 24/09/2026
//***************************************

//*********DEFINES********//

//*****VARIABLES GLOBALES*****//

struct Commande
{
    int type;
    int parametre1;
    int parametre2;
    int position[3];
};

extern Commande commande;
//********FONCTIONS********//
void initialisationCommande(void);

#endif