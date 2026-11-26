#include <Arduino.h>
#include "Bluetooth.h"
#include "Commande.h"
#include "DataBase.h"
#include "ActionsRobot.h"

namespace {
// HC-05 sur UART2 dans le projet principal.
HardwareSerial& portBluetooth = Serial2;
constexpr unsigned long DEBIT = 9600;
constexpr unsigned int MAX_LIGNE = 200;
String reception;
bool ligneTropLongue = false;
bool arretLogiciel = false;

// Répond avec le même identifiant, même si la commande est refusée.
void accuser(const String& id, const char* erreur = nullptr) {
    portBluetooth.print(F("ACK|"));
    portBluetooth.print(id);
    if (erreur) {
        portBluetooth.print(F("|ERROR|"));
        portBluetooth.println(erreur);
    } else {
        portBluetooth.println(F("|OK"));
    }
}

// Écrit une pièce sur le port série sans gérer son contenu.
void envoyerPiece(const char* genre, const Piece& piece) {
    portBluetooth.print(genre);
    portBluetooth.print('|');
    portBluetooth.print(piece.numero);
    portBluetooth.print('|');
    portBluetooth.print(piece.type);
    portBluetooth.print('|');
    portBluetooth.print(piece.position);
    portBluetooth.print('|');
    portBluetooth.print(piece.etat);
    portBluetooth.print('|');
    portBluetooth.println(piece.nom);
}

// Adaptateurs pour la liste complète et les résultats filtrés.
void envoyerCase(const Piece& piece) { envoyerPiece("PIECE", piece); }
void envoyerResultat(const Piece& piece) { envoyerPiece("MATCH", piece); }

// Termine la liste seulement si la lecture est complète.
const char* envoyerListe(const Commande* recherche = nullptr) {
    portBluetooth.println(recherche ? F("MATCH_BEGIN") : F("INV_BEGIN"));
    const char* erreur = listerInventaire(recherche, recherche ? envoyerResultat : envoyerCase);
    if (erreur) portBluetooth.println(recherche ? F("MATCH_ABORT") : F("INV_ABORT"));
    else portBluetooth.println(recherche ? F("MATCH_END") : F("INV_END"));
    return erreur;
}

// Route la commande. L'inventaire et les écritures restent dans DataBase.
void traiterLigne(const String& ligne) {
    Commande recue;
    if (!decoderCommande(ligne, recue)) {
        portBluetooth.println(F("ERROR|FORMAT_COMMANDE"));
        return;
    }
    commande = recue;

    // La copie locale reste valide si une autre commande est ajoutée plus tard.
    const String& action = recue.action;
    if (action == "SYNC" && recue.nombreArgs == 0) {
        accuser(recue.id);
        portBluetooth.println(F("INFO|MODE_INVENTAIRE_SD"));
        portBluetooth.println(F("INFO|PROTOCOLE_INVENTAIRE_2"));
        portBluetooth.println(arretLogiciel ? F("ETAT|ARRET") : F("ETAT|PRET"));
        const char* erreur = envoyerListe();
        if (erreur) {
            portBluetooth.print(F("ERROR|"));
            portBluetooth.println(erreur);
        }
    } else if ((action == "EMERGENCY_STOP" || action == "RESET_ESTOP") && recue.nombreArgs == 0) {
        // Ce verrou empêche les écritures. Relier l'arrêt matériel dans le futur pilote.
        arretLogiciel = action == "EMERGENCY_STOP";
        if (arretLogiciel) arretUrgence();
        accuser(recue.id);
        portBluetooth.println(arretLogiciel ? F("ETAT|ARRET") : F("ETAT|PRET"));
    } else if (action == "CHERCHER") {
        accuser(recue.id, envoyerListe(&recue));
    } else if (arretLogiciel) {
        accuser(recue.id, "ARRET_ACTIF");
    } else {
        const char* erreur = gestionDonnees(recue);
        accuser(recue.id, erreur);
        if (!erreur) {
            erreur = envoyerListe();
            if (erreur) {
                portBluetooth.print(F("ERROR|"));
                portBluetooth.println(erreur);
            }
        }
    }
}
}

// Ouvre le port utilisé par le HC-05.
void initialiserBluetooth() {
    portBluetooth.begin(DEBIT);
    reception.reserve(MAX_LIGNE);
}

// Envoie une ligne de texte dans la console Bluetooth de la manette.
void envoyerTexteBluetooth(const String& texte) {
    portBluetooth.println(texte);
}

// Un bouton local passe par les mêmes validations qu'une ligne du PC.
void commandeLocale(const char* ligne) {
    traiterLigne(ligne);
}

// Lit les octets disponibles. Attend la fin de ligne avant de lancer une action.
void lireBluetooth() {
    while (portBluetooth.available()) {
        char c = portBluetooth.read();
        if (c == '\r') continue;
        if (c == '\n') {
            if (ligneTropLongue) portBluetooth.println(F("ERROR|LIGNE_TROP_LONGUE"));
            else if (reception.length()) traiterLigne(reception);
            ligneTropLongue = false;
            reception = "";
        } else if (!ligneTropLongue) {
            if (reception.length() >= MAX_LIGNE || !reception.concat(c)) ligneTropLongue = true;
        }
    }
}
