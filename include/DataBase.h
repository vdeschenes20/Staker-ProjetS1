#ifndef DATABASE_H
#define DATABASE_H
//**************************************
// Projet: Labyrinthe-S1
// Fichiers: DataBase.h
// Equipe: P13
// Auteurs: 
// Description: Breve description du script
// Date: 24/09/2026
//***************************************
#include "Commande.h"

//*********DEFINES********//
#define ERREUR 13

#define ATTENTE  0
#define CHERCHER 1
#define TRAITER  2
#define STOCKER  3

#define PLACE         0
#define EN_TRAITEMENT 1

#define NUMERO   0 
#define POSITION 1
#define TYPE     2
#define ETAT     3

#define PIECE_EXISTANTE 0
#define PIECE_EN_ATTENTE 1
#define AUCUNE_PLACE_DISPONIBLE 2
#define NOUVELLE_PIECE 3
#define POSITION_DISPONIBLE 4
#define POSITION_OCCUPEE 5

#define POS_SIZE 3
//*****VARIABLES GLOBALES*****//
struct Piece
{
    int ligne;
    int numero;
    int type;
    int position[3];
    int etat;
};

//********FONCTIONS********//
void initialsationDonnees(void);
void gestionDonnees(Commande c);
void recherche(int parametre, int info, int *position);
void stockage(int parametre, int info, int *position);
bool scanPiece(int parametre, Piece *p1);
int scanStockage(Piece *p1);
void afficheInfos(Piece p);
void dataBaseUpdate(int type, Piece p);
void creationPiece(Piece p);
void parsePieceDetail(String PieceDetail, Piece *p1);
bool verificationNumeroDisponible(int numero);
bool verificationPositionDisponible(int *position);

#endif