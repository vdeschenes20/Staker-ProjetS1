//**************************************
// Projet: staker-S1
// Fichiers: DataBase.cpp
// Equipe: P13
// Auteurs: 
// Description: Breve description du script
// Date: 24/09/2026
//***************************************

//*********INCLUDES********//
#include <Arduino.h>
#include "DataBase.h"
#include "Commande.h"
#include "SDdriver.h"

//********CONSTANTES*******//

//********VARIABLES********//
Piece pieceExistante;

//********FONCTIONS********//
void initialsationDonnees(void)
{
    intitialisationSD();    
}


void gestionDonnees(Commande c)
{
    switch (c.type)
    {
        case ATTENTE:
        scanAttente(&pieceExistante, &c);
            if(c.parametre1 != ATTENTE)
            {
                c.type = STOCKER;
                c.parametre2 = pieceExistante.type;
                for (int i = 0; i < POS_SIZE; i++) {
                    c.position[i] = pieceExistante.position[i];
                }
            }
            return;
            break;
        case CHERCHER:
            recherche(c.parametre1, c.parametre2, c.position);
            break;
        case TRAITER:

            break;
        case STOCKER:
            stockage(c.parametre1, c.parametre2, c.position);
            break;
        default:
            break;
    }

    
}

void recherche(int parametre, int info, int *position)
{
    switch (parametre)
    {
        case TYPE:
            pieceExistante.type = info;
            break;
        case POSITION:
            for (int i = 0; i < POS_SIZE; i++) {
                pieceExistante.position[i] = position[i];
            }
            break;
        case NUMERO:
            pieceExistante.numero = info;
            break;
            
    }
    bool etatScan = scanPiece(parametre, &pieceExistante);
    if(etatScan == false)
    {
        Serial.println("Piece inexistante");
        return;
    }
    else
    {
        dataBaseUpdate(CHERCHER, pieceExistante);
        afficheInfos(pieceExistante);
    }
}

void stockage(int numero, int type, int *position)
{
    Piece nouvellePiece;
    int etat;
    nouvellePiece.numero = numero;
    nouvellePiece.type = type;
    for (int i = 0; i < POS_SIZE; i++) {
        nouvellePiece.position[i] = position[i];
    }
    etat = scanStockage(&nouvellePiece);
    switch (etat)
    {
        case PIECE_EN_ATTENTE:
            Serial.println("Piece en attente");
            creationDemande(nouvellePiece);
            break;
        case PIECE_EXISTANTE:
            Serial.println("Piece deja existante");
            break;
        case AUCUNE_PLACE_DISPONIBLE:
            Serial.println("Aucune place disponible");
            break;
        case POSITION_OCCUPEE:
            Serial.println("Position deja occupee");
            break;
        case NOUVELLE_PIECE:
            creationPiece(nouvellePiece);
            break;
        case POSITION_DISPONIBLE:
            creationPiece(nouvellePiece);
            break;
        case ERREUR:
            Serial.println("Erreur de scan stockage");
            break;
    }
}
bool scanPiece(int parametre, Piece *p1)
{
    int colonneCible = 0;
    String valeurRecherchee = " ";
    String PieceDetail = " ";
    Piece detail;


    switch (parametre)
    {
        case TYPE:
            colonneCible = 1;
            valeurRecherchee = String((char)(p1->type + 64));  
            break;
        case POSITION:
            colonneCible = 2;
            valeurRecherchee = String(p1->position[0]) + "," + String(p1->position[1]) + "," + String(p1->position[2]);
            if(verificationPositionDisponible(p1->position)) return false;
            break;
        case NUMERO:
            colonneCible = 0;
            valeurRecherchee = "0" + String(p1->numero);
            if(p1->numero < 10) valeurRecherchee = "0" + valeurRecherchee;
            break;
    }
    PieceDetail = trouverLigne(colonneCible, valeurRecherchee);
    Serial.println(PieceDetail);
    if(PieceDetail == "")
    {
        Serial.println("Piece introuvable");
        return false;
    }

    parsePieceDetail(PieceDetail, &detail);
    *p1 = detail;

    return true;
}
void parsePieceDetail(String PieceDetail, Piece *p1)
{
    int debutIndex = 0;
    int indexFin = PieceDetail.indexOf(';', debutIndex);
    p1->ligne = (PieceDetail.substring(debutIndex, indexFin).toInt()) - 1;
    debutIndex = indexFin + 1;

    indexFin = PieceDetail.indexOf(';', debutIndex);
    p1->numero = PieceDetail.substring(debutIndex, indexFin).charAt(0);
    debutIndex = indexFin + 1;

    indexFin = PieceDetail.indexOf(';', debutIndex);
    p1->type = PieceDetail.substring(debutIndex, indexFin).charAt(0);
    debutIndex = indexFin + 1;

    // 3. Extraire le bloc de position (ex: "10,20,5") jusqu'au dernier ';' avant l'état
    indexFin = PieceDetail.lastIndexOf(';'); // On cherche le dernier point-virgule de la ligne
    String posStr = PieceDetail.substring(debutIndex, indexFin);
    debutIndex = indexFin + 1; // Le reste de la ligne appartiendra à l'état

    // 3.1 Découper les 3 coordonnées à l'intérieur de posStr (séparées par des virgules)
    int comma1 = posStr.indexOf(',');
    int comma2 = posStr.indexOf(',', comma1 + 1);

    p1->position[0] = posStr.substring(0, comma1).toInt();
    p1->position[1] = posStr.substring(comma1 + 1, comma2).toInt();
    p1->position[2] = posStr.substring(comma2 + 1).toInt();

    // 4. Extraire l'état (tout ce qui reste à la fin de la ligne)
    p1->etat = PieceDetail.substring(debutIndex).toInt();
}
void afficheInfos(Piece p)
{
    Serial.print("Numero: ");
    Serial.println(p.numero);
    Serial.print("Type: ");
    Serial.println(p.type);
    Serial.print("Position: ");
    for (int i = 0; i < POS_SIZE; i++) {
        Serial.print(p.position[i]);
        if (i < POS_SIZE - 1) {
            Serial.print(", ");
        }
    }
    Serial.println();
    Serial.print("Etat: ");
    Serial.println(p.etat);
}
void dataBaseUpdate(int type, Piece p)
{
    String numeroStr = "0" + String(p.numero);
    String typeStr = String((char)(p.type + 64));
    String positionStr = String(p.position[0]) + "," + String(p.position[1]) + "," + String(p.position[2]);
    String etatStr = String(p.etat);

    switch(type)
    {
        case CHERCHER:
            updateCSV((p.ligne), 0, "---");
            updateCSV((p.ligne), 1, "-");
            updateCSV((p.ligne), 3, "-");
            Serial.println("Mise à jour de la base de données après recherche");
            break;
        case STOCKER:
            if(p.numero < 10) numeroStr = "0" + numeroStr;
            if(p.numero == 0) numeroStr = "N/A";
            updateCSV((p.ligne), 0, numeroStr);
            updateCSV((p.ligne), 1, typeStr);
            updateCSV((p.ligne), 2, positionStr);
            updateCSV((p.ligne), 3, etatStr);
            Serial.println("Mise à jour de la base de données après stockage");
            break;
    }
}
int scanStockage(Piece *p1)
{
    String spaceDetail = trouverLigne(3, "0"); //trouve un etat d'attente
    if(spaceDetail != "")
    {
        Serial.println("Piece en attente");
        return PIECE_EN_ATTENTE;
    }
    if((p1->numero != 0) && (p1->position[0] != 0)) // si on recois un type un numero et une position 
    {
        if(!verificationNumeroDisponible(p1->numero))
        {
            Serial.println("Numero deja existant");
            return PIECE_EXISTANTE;
        }
        if(verificationPositionDisponible(p1->position))
        {
            return POSITION_DISPONIBLE;
        }
        else
        {
            Serial.println("Position deja occupee");
            return POSITION_OCCUPEE;
        } 
    }
    else if((p1->numero !=0 && p1->position[0] == 0)) // on recois un type et un numero
    {
        if(!verificationNumeroDisponible(p1->numero))
        {
            Serial.println("Numero deja existant");
            return PIECE_EXISTANTE;
        }
        else
        {
            Serial.println("Numero libre");
            return NOUVELLE_PIECE;
        }  
    }
    else if((p1->numero == 0 && p1->position[0] != 0)) // on recois un type et une position
    {
        if(verificationPositionDisponible(p1->position))
        {
            return POSITION_DISPONIBLE;
        }
        else
        {
            Serial.println("Position deja occupee");
            return POSITION_OCCUPEE;
        } 
    }
    else if((p1->numero == 0 && p1->position[0] == 0))
    {
        return NOUVELLE_PIECE;
    }
    return ERREUR;
}
void creationPiece(Piece p)
{
    String detail = "";
    Piece pieceTemp;

    pieceTemp.numero = p.numero;
    pieceTemp.type = p.type;

    if((p.position[0] != 0)) //Si on indique une position spécifique
    {
        detail = trouverLigne(2, String(p.position[0]) + "," + String(p.position[1]) + "," + String(p.position[2]));
        parsePieceDetail(detail, &p);
    }
    else if(p.position[0] == 0)
    {
        detail = trouverLigne(0, "---");
        parsePieceDetail(detail, &p);
    }

    p.numero = pieceTemp.numero;
    p.type = pieceTemp.type;
    p.etat = TRAITER;

    dataBaseUpdate(STOCKER, p);
}
void creationDemande(Piece p)
{
    return;
}
bool verificationNumeroDisponible(int numero)
{
    String numeroRecherche = "0" + String(numero);
    if(numero < 10) numeroRecherche = "0" + numeroRecherche;
    if(trouverLigne(0, numeroRecherche) == "") //Si la recherche indique aucun numero identique
    {
        return true;                           //numero disponible
    }
    else
    {
        return false;
    }
}
bool verificationPositionDisponible(int *position)
{
    Piece p;
    String detail = trouverLigne(2, String(position[0]) + "," + String(position[1]) + "," + String(position[2]));
    if(detail == "")
    {
        Serial.println("pos inexistante");
        return true;
    }
    else
    {
        parsePieceDetail(detail, &p);
        if(p.numero == 45 && p.type == 45 && p.etat == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}