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
#define hc05 Serial2
//*****VARIABLES GLOBALES*****//

struct Commande
{
    int id;
    int type;
    int parametre1;
    int parametre2;
    int position[3];
};
enum TypeCommande {
  CMD_INCONNUE = 0,
  CMD_SYNC,
  CMD_STOCKER,
  CMD_CHERCHER,
  CMD_RECUPERER,
  CMD_AJOUTER_EMPLACEMENT,
  CMD_RETIRER_EMPLACEMENT,
  CMD_RESET_INVENTAIRE,
  CMD_EMERGENCY_STOP,
  CMD_RESET_ESTOP
};

enum CritereRecherche {
  CRITERE_AUCUN = 0,
  CRITERE_NUMERO,
  CRITERE_TYPE,
  CRITERE_POSITION
};

static const size_t TAILLE_TAMPON = 160;

extern Commande commande;

//********FONCTIONS********//
void initialisationCommande(void);
bool lireCommandeHC05(Commande &cmdOut);
void executerCommande(const Commande &cmd);
#endif