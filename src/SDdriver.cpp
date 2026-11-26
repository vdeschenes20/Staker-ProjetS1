//**************************************
// Projet: staker-S1
// Fichiers: SDdriver.cpp
// Equipe: P13
// Auteurs: 
// Description: Breve description du script
// Date: 24/09/2026
//***************************************

#include "SDdriver.h"

namespace {
SdFat sd;
const char* CSV = "/data.csv";
const char* COPIE = "/data.tmp";
const char* SAUVEGARDE = "/data.bak";

// Lit une ligne bornée pour éviter de charger tout le CSV en RAM.
Lecture lireLigne(File32& fichier, String& ligne) {
    ligne = "";
    int octet;
    while ((octet = fichier.read()) >= 0) {
        if (octet == '\n') return LIGNE;
        if (octet == '\r') continue;
        if (ligne.length() >= 255 || !ligne.concat(static_cast<char>(octet))) return ERREUR_CSV;
    }
    if (fichier.getError()) return ERREUR_CSV;
    return ligne.length() ? LIGNE : FIN_CSV;
}

// Sépare les champs CSV. Deux guillemets dans un champ représentent un guillemet.
bool decoderLigne(const String& ligne, Piece& piece) {
    String* champs[] = {&piece.numero, &piece.type, &piece.position, &piece.etat, &piece.nom};
    for (String* champ : champs) *champ = "";
    uint8_t colonne = 0;
    bool entreGuillemets = false;
    bool guillemetsFermes = false;
    for (unsigned int i = 0; i < ligne.length(); ++i) {
        char c = ligne[i];
        if (c == '|') return false;
        if (entreGuillemets && c == '"') {
            if (i + 1 < ligne.length() && ligne[i + 1] == '"') ++i;
            else {
                entreGuillemets = false;
                guillemetsFermes = true;
                continue;
            }
        } else if (!entreGuillemets && c == ';') {
            if (++colonne >= 5) return false;
            guillemetsFermes = false;
            continue;
        } else if (guillemetsFermes) {
            return false;
        } else if (!entreGuillemets && c == '"') {
            if (champs[colonne]->length()) return false;
            entreGuillemets = true;
            continue;
        }
        if (!champs[colonne]->concat(c)) return false;
    }
    return !entreGuillemets && colonne >= 3 && piece.position.length() &&
           (piece.etat == "-" || piece.etat == "0" || piece.etat == "1" || piece.etat == "2");
}

// Protège les guillemets et points-virgules lors de la réécriture.
void ecrireChamp(File32& copie, const String& champ) {
    if (champ.indexOf('"') < 0 && champ.indexOf(';') < 0) {
        copie.print(champ);
        return;
    }
    copie.print('"');
    for (unsigned int i = 0; i < champ.length(); ++i) {
        if (champ[i] == '"') copie.print('"');
        copie.print(champ[i]);
    }
    copie.print('"');
}
}

// Récupère l'ancien CSV si le robot a redémarré entre les deux renommages.
bool initialiserSD() {
    if (!sd.begin(CS_PIN, SD_SCK_MHZ(4))) return false;
    if (!sd.exists(CSV) && sd.exists(SAUVEGARDE) && !sd.rename(SAUVEGARDE, CSV)) return false;
    return sd.exists(CSV);
}

// Ouvre le CSV et saute la première ligne, même avec un BOM UTF-8.
bool ouvrirCSV(File32& fichier) {
    String entete;
    if (!fichier.open(CSV, O_RDONLY)) return false;
    if (lireLigne(fichier, entete) == LIGNE &&
        (entete.startsWith("No;Type;") || entete.startsWith("\xEF\xBB\xBF" "No;Type;"))) return true;
    fichier.close();
    return false;
}

// Lit une pièce. Saute les lignes vides.
Lecture lirePiece(File32& fichier, Piece& piece) {
    String ligne;
    Lecture resultat;
    do {
        resultat = lireLigne(fichier, ligne);
    } while (resultat == LIGNE && !ligne.length());
    if (resultat != LIGNE) return resultat;
    return decoderLigne(ligne, piece) ? LIGNE : ERREUR_CSV;
}

// Prépare une copie indépendante. Le CSV original reste intact pendant l'écriture.
bool ouvrirCopieCSV(File32& copie) {
    if (!copie.open(COPIE, O_WRONLY | O_CREAT | O_TRUNC)) return false;
    copie.println(F("No;Type;Position;Etat;Nom"));
    return !copie.getWriteError();
}

// Écrit une ligne complète, quelle que soit la longueur des champs.
bool ecrirePiece(File32& copie, const Piece& piece) {
    const String* champs[] = {&piece.numero, &piece.type, &piece.position, &piece.etat, &piece.nom};
    for (uint8_t i = 0; i < 5; ++i) {
        if (i) copie.print(';');
        ecrireChamp(copie, *champs[i]);
    }
    copie.println();
    return !copie.getWriteError();
}

// Remplace le fichier seulement si la copie a été écrite jusqu'au bout.
bool terminerCopieCSV(File32& copie, bool conserver) {
    bool complete = conserver && !copie.getWriteError() && copie.sync();
    complete = copie.close() && complete;
    if (!complete) {
        sd.remove(COPIE);
        return false;
    }
    if (sd.exists(SAUVEGARDE) && !sd.remove(SAUVEGARDE)) return false;
    if (!sd.rename(CSV, SAUVEGARDE)) return false;
    if (!sd.rename(COPIE, CSV)) {
        sd.rename(SAUVEGARDE, CSV);
        return false;
    }
    return true;
}
