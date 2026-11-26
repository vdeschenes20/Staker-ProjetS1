#include <Arduino.h>
#include "Bluetooth.h"
#include "Commande.h"
#include "DataBase.h"

namespace {
constexpr unsigned long DEBIT = 9600;
constexpr unsigned int MAX_LIGNE = 200;
String reception;
bool ligneTropLongue = false;
bool arretLogiciel = false;

// Répond avec le même identifiant, même si la commande est refusée.
void accuser(const String& id, const char* erreur = nullptr) {
    Serial1.print(F("ACK|"));
    Serial1.print(id);
    if (erreur) {
        Serial1.print(F("|ERROR|"));
        Serial1.println(erreur);
    } else {
        Serial1.println(F("|OK"));
    }
}

// Écrit une pièce sur le port série sans gérer son contenu.
void envoyerPiece(const char* genre, const Piece& piece) {
    Serial1.print(genre);
    Serial1.print('|');
    Serial1.print(piece.numero);
    Serial1.print('|');
    Serial1.print(piece.type);
    Serial1.print('|');
    Serial1.print(piece.position);
    Serial1.print('|');
    Serial1.print(piece.etat);
    Serial1.print('|');
    Serial1.println(piece.nom);
}

// Adaptateurs pour la liste complète et les résultats filtrés.
void envoyerCase(const Piece& piece) { envoyerPiece("PIECE", piece); }
void envoyerResultat(const Piece& piece) { envoyerPiece("MATCH", piece); }

// Termine la liste seulement si la lecture est complète.
const char* envoyerListe(const Commande* recherche = nullptr) {
    Serial1.println(recherche ? F("MATCH_BEGIN") : F("INV_BEGIN"));
    const char* erreur = listerInventaire(recherche, recherche ? envoyerResultat : envoyerCase);
    if (erreur) Serial1.println(recherche ? F("MATCH_ABORT") : F("INV_ABORT"));
    else Serial1.println(recherche ? F("MATCH_END") : F("INV_END"));
    return erreur;
}

// Route la commande. L'inventaire et les écritures restent dans DataBase.
void traiterLigne(const String& ligne) {
    Commande commande;
    if (!decoderCommande(ligne, commande)) {
        Serial1.println(F("ERROR|FORMAT_COMMANDE"));
        return;
    }

    const String& action = commande.action;
    if (action == "SYNC" && commande.nombreArgs == 0) {
        accuser(commande.id);
        Serial1.println(F("INFO|MODE_INVENTAIRE_SD"));
        Serial1.println(arretLogiciel ? F("ETAT|ARRET") : F("ETAT|PRET"));
        const char* erreur = envoyerListe();
        if (erreur) {
            Serial1.print(F("ERROR|"));
            Serial1.println(erreur);
        }
    } else if ((action == "EMERGENCY_STOP" || action == "RESET_ESTOP") && commande.nombreArgs == 0) {
        // Ce verrou empêche les écritures. Relier l'arrêt matériel dans le futur pilote.
        arretLogiciel = action == "EMERGENCY_STOP";
        accuser(commande.id);
        Serial1.println(arretLogiciel ? F("ETAT|ARRET") : F("ETAT|PRET"));
    } else if (action == "CHERCHER") {
        accuser(commande.id, envoyerListe(&commande));
    } else if (arretLogiciel) {
        accuser(commande.id, "ARRET_ACTIF");
    } else {
        const char* erreur = gestionDonnees(commande);
        accuser(commande.id, erreur);
        if (!erreur) {
            erreur = envoyerListe();
            if (erreur) {
                Serial1.print(F("ERROR|"));
                Serial1.println(erreur);
            }
        }
    }
}
}

// Ouvre le port utilisé par le HC-05.
void initialiserBluetooth() {
    Serial1.begin(DEBIT);
    reception.reserve(MAX_LIGNE);
}

// Un bouton local passe par les mêmes validations qu'une ligne du PC.
void commandeLocale(const char* ligne) {
    traiterLigne(ligne);
}

// Lit les octets disponibles. Attend la fin de ligne avant de lancer une action.
void lireBluetooth() {
    while (Serial1.available()) {
        char c = Serial1.read();
        if (c == '\r') continue;
        if (c == '\n') {
            if (ligneTropLongue) Serial1.println(F("ERROR|LIGNE_TROP_LONGUE"));
            else if (reception.length()) traiterLigne(reception);
            ligneTropLongue = false;
            reception = "";
        } else if (!ligneTropLongue) {
            if (reception.length() >= MAX_LIGNE || !reception.concat(c)) ligneTropLongue = true;
        }
    }
}
