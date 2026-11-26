//**************************************
// Projet: staker-S1
// Fichiers: DataBase.cpp
// Equipe: P13
// Auteurs: 
// Description: Breve description du script
// Date: 24/09/2026
//***************************************

#include "Commande.h"

// Découpe CMD|id|action|arg... en conservant les champs vides.
bool decoderCommande(const String& ligne, Commande& commande) {
    String champs[7];
    uint8_t nombre = 0;
    unsigned int debut = 0;
    for (unsigned int i = 0; i <= ligne.length(); ++i) {
        if (i != ligne.length() && ligne[i] != '|') continue;
        if (nombre == 7) return false;
        champs[nombre++] = ligne.substring(debut, i);
        debut = i + 1;
    }
    if (nombre < 3 || champs[0] != "CMD" || !champs[1].length() || !champs[2].length()) {
        return false;
    }
    commande.id = champs[1];
    commande.action = champs[2];
    commande.nombreArgs = nombre - 3;
    for (uint8_t i = 0; i < commande.nombreArgs; ++i) commande.args[i] = champs[i + 3];
    return true;
}
