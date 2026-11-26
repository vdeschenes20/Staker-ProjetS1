//**************************************
// Projet: staker-S1
// Fichiers: DataBase.cpp
// Equipe: P13
// Auteurs: 
// Description: Breve description du script
// Date: 24/09/2026
//***************************************

#include "DataBase.h"
#include "SDdriver.h"

namespace {
// Accepte des chiffres, y compris les zéros devant un numéro.
bool chiffres(const String& valeur) {
    if (!valeur.length()) return false;
    for (unsigned int i = 0; i < valeur.length(); ++i) {
        if (valeur[i] < '0' || valeur[i] > '9') return false;
    }
    return true;
}

// Valide trois coordonnées positives dans la forme envoyée par le PC.
bool positionValide(const String& position) {
    int debut = 0;
    for (int i = 0; i < 3; ++i) {
        int fin = position.indexOf(',', debut);
        if ((i < 2) != (fin >= 0)) return false;
        if (fin < 0) fin = position.length();
        String valeur = position.substring(debut, fin);
        if (!chiffres(valeur) || valeur[0] == '0' || valeur.length() > 9) return false;
        debut = fin + 1;
    }
    return true;
}

// Valide nom et type avant écriture sur la SD.
bool texteValide(const String& texte) {
    if (!texte.length() || texte.length() > 48 || texte == "-") return false;
    for (unsigned int i = 0; i < texte.length(); ++i) {
        if (static_cast<uint8_t>(texte[i]) < 32 || texte[i] == ';' || texte[i] == '|') return false;
    }
    return true;
}

// Compte les lignes d'une position. -1 signale une erreur de lecture.
int compterPosition(const String& position) {
    File32 fichier;
    if (!ouvrirCSV(fichier)) return -1;
    int total = 0;
    Piece piece;
    Lecture lecture;
    while ((lecture = lirePiece(fichier, piece)) == LIGNE) {
        if (piece.position == position) ++total;
    }
    fichier.close();
    return lecture == FIN_CSV ? total : -1;
}

// Teste le critère de recherche. Les lettres ASCII sont comparées sans casse.
bool correspond(const Piece& piece, const Commande& recherche) {
    const String& critere = recherche.args[0];
    const String& valeur = recherche.args[1];
    if (critere == "NUMERO") return piece.numero == valeur;
    if (critere == "POSITION") return piece.position == valeur;
    if (critere == "TYPE_TEXTE") return piece.type.equalsIgnoreCase(valeur);
    String nom = piece.nom;
    String filtre = valeur;
    nom.toLowerCase();
    filtre.toLowerCase();
    return nom.indexOf(filtre) >= 0;
}
}

// Lit l'inventaire ou filtre les résultats. Une recherche ne modifie rien.
const char* listerInventaire(const Commande* recherche, SortiePiece sortie) {
    if (recherche) {
        const String& critere = recherche->args[0];
        if (recherche->nombreArgs != 2 || !recherche->args[1].length() ||
            (critere != "NUMERO" && critere != "POSITION" && critere != "TYPE_TEXTE" && critere != "NOM")) {
            return "CRITERE_INVALIDE";
        }
    }
    if (!initialiserSD()) return "SD_OU_CSV_INDISPONIBLE";
    File32 fichier;
    if (!ouvrirCSV(fichier)) return "LECTURE_CSV";
    Piece piece;
    Lecture lecture;
    while ((lecture = lirePiece(fichier, piece)) == LIGNE) {
        if (!recherche || (piece.etat == "1" && correspond(piece, *recherche))) sortie(piece);
    }
    fichier.close();
    return lecture == FIN_CSV ? nullptr : "CSV_INVALIDE";
}

// Valide la demande, puis réécrit le CSV. nullptr signifie succès.
const char* gestionDonnees(const Commande& commande) {
    const String& action = commande.action;
    bool stocker = action == "STOCKER";
    bool recuperer = action == "RECUPERER";
    bool ajouter = action == "AJOUTER_EMPLACEMENT";
    bool retirer = action == "RETIRER_EMPLACEMENT";
    bool vider = action == "RESET_INVENTAIRE";
    if (!stocker && !recuperer && !ajouter && !retirer && !vider) return "COMMANDE_INCONNUE";
    if (commande.nombreArgs != (stocker ? 4 : vider ? 0 : 1)) return "ARGUMENTS_INVALIDES";

    String cible;
    if (stocker) {
        cible = commande.args[2];
        if ((commande.args[0] != "-" && (!chiffres(commande.args[0]) || commande.args[0].length() > 20)) ||
            !texteValide(commande.args[1]) || !texteValide(commande.args[3])) return "OBJET_INVALIDE";
    } else if (!vider) {
        cible = commande.args[0];
    }
    bool automatique = stocker && cible == "-";
    if (!vider && !automatique && !positionValide(cible)) return "POSITION_INVALIDE";
    if (!initialiserSD()) return "SD_OU_CSV_INDISPONIBLE";

    // Premier passage : vérifier l'état, repérer la cible et refuser les doublons.
    File32 fichier;
    if (!ouvrirCSV(fichier)) return "LECTURE_CSV";
    Piece piece;
    Lecture lecture;
    int occurrences = 0;
    String etatCible;
    const char* erreur = nullptr;
    while ((lecture = lirePiece(fichier, piece)) == LIGNE) {
        if ((stocker || vider) && piece.etat == "2") {
            erreur = "PIECE_EN_TRAITEMENT";
            break;
        }
        if (stocker && commande.args[0] != "-" && piece.numero == commande.args[0]) {
            erreur = "NUMERO_DEJA_UTILISE";
            break;
        }
        if (automatique && cible == "-" && piece.etat == "-") {
            int total = compterPosition(piece.position);
            if (total < 0) {
                erreur = "LECTURE_CSV";
                break;
            }
            if (total == 1 && positionValide(piece.position)) cible = piece.position;
        }
        if (piece.position == cible) {
            ++occurrences;
            etatCible = piece.etat;
        }
    }
    fichier.close();
    if (erreur) return erreur;
    if (lecture != FIN_CSV) return "CSV_INVALIDE";
    if (!vider) {
        if (ajouter && occurrences) return "POSITION_DEJA_UTILISEE";
        if (!ajouter && occurrences != 1) return automatique ? "AUCUNE_CASE_LIBRE" : "POSITION_INCONNUE_OU_AMBIGUE";
        if ((stocker || retirer) && etatCible != "-") return "POSITION_OCCUPEE";
        if (recuperer && etatCible != "1") return "PIECE_NON_DISPONIBLE";
    }

    // Second passage : copier toutes les lignes sauf celle retirée.
    if (!ouvrirCSV(fichier)) return "LECTURE_CSV";
    File32 copie;
    bool ok = ouvrirCopieCSV(copie);
    while (ok && (lecture = lirePiece(fichier, piece)) == LIGNE) {
        bool cibleTrouvee = piece.position == cible;
        if (retirer && cibleTrouvee) continue;
        if (vider || (recuperer && cibleTrouvee)) {
            piece.numero = "---";
            piece.type = "-";
            piece.etat = "-";
            piece.nom = "";
        } else if (stocker && cibleTrouvee) {
            piece.numero = commande.args[0] == "-" ? "---" : commande.args[0];
            piece.type = commande.args[1];
            piece.etat = "1";
            piece.nom = commande.args[3];
        }
        ok = ecrirePiece(copie, piece);
    }
    fichier.close();
    if (ok && lecture == FIN_CSV && ajouter) {
        Piece libre = {"---", "-", cible, "-", ""};
        ok = ecrirePiece(copie, libre);
    }
    return terminerCopieCSV(copie, ok && lecture == FIN_CSV) ? nullptr : "ECRITURE_CSV";
}
