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
#include "ActionsRobot.h"

//********VARIABLES********//
Piece pieceExistante;

namespace {
//********FONCTIONS********//
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

// Choisit sur la SD. Par type, l'ordre du CSV détermine la première pièce.
const char* scanPiece(const Commande& commande, String& position) {
    const String& critere = commande.args[0];
    const String& valeur = commande.args[1];
    bool parNumero = critere == "NUMERO";
    if ((!parNumero && critere != "TYPE_TEXTE") ||
        (parNumero && (!chiffres(valeur) || valeur.length() > 20)) ||
        (!parNumero && !texteValide(valeur))) return "CRITERE_INVALIDE";

    File32 fichier;
    if (!ouvrirCSV(fichier)) return "LECTURE_CSV";
    Piece piece;
    Lecture lecture;
    bool numeroTrouve = false;
    const char* erreur = nullptr;
    while ((lecture = lirePiece(fichier, piece)) == LIGNE) {
        if (parNumero && piece.etat != "-" && piece.numero == valeur) {
            if (numeroTrouve) {
                erreur = "NUMERO_AMBIGU";
                break;
            }
            numeroTrouve = true;
            if (piece.etat == "1") position = piece.position;
        } else if (!parNumero && !position.length() && piece.etat == "1" &&
                   piece.type.equalsIgnoreCase(valeur) && positionValide(piece.position)) {
            int total = compterPosition(piece.position);
            if (total < 0) {
                erreur = "LECTURE_CSV";
                break;
            }
            if (total == 1) position = piece.position;
        }
    }
    fichier.close();
    if (erreur) return erreur;
    if (lecture != FIN_CSV) return "CSV_INVALIDE";
    return position.length() ? nullptr : "PIECE_NON_DISPONIBLE";
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

// Prépare la pièce à stocker et demande la confirmation du dépôt.
bool creationPiece(const Commande& commande, Piece& piece) {
    piece.numero = commande.args[0] == "-" ? "---" : commande.args[0];
    piece.type = commande.args[1];
    piece.etat = "1";
    piece.nom = "";
    return allerPlacer(piece.numero, piece.type, piece.position, piece.etat);
}

// Met à jour le CSV après confirmation. La copie est écrite ligne par ligne.
bool dataBaseUpdate(const String& action, const String& cible, const Piece& pieceCible) {
    File32 fichier;
    if (!ouvrirCSV(fichier)) return false;
    File32 copie;
    Piece piece;
    Lecture lecture = ERREUR_CSV;
    bool ok = ouvrirCopieCSV(copie);
    while (ok && (lecture = lirePiece(fichier, piece)) == LIGNE) {
        bool cibleTrouvee = piece.position == cible;
        if (action == "RETIRER_EMPLACEMENT" && cibleTrouvee) continue;
        if (action == "RESET_INVENTAIRE" || (action == "RECUPERER" && cibleTrouvee)) {
            piece.numero = "---";
            piece.type = "-";
            piece.etat = "-";
            piece.nom = "";
        } else if (action == "STOCKER" && cibleTrouvee) {
            piece = pieceCible;
        }
        ok = ecrirePiece(copie, piece);
    }
    fichier.close();
    if (ok && lecture == FIN_CSV && action == "AJOUTER_EMPLACEMENT") {
        Piece libre = {"---", "-", cible, "-", ""};
        ok = ecrirePiece(copie, libre);
    }
    return terminerCopieCSV(copie, ok && lecture == FIN_CSV);
}

// Affiche la pièce récupérée dans le journal USB.
void afficheInfos(const Piece& p) {
    Serial.print(F("Numero: "));
    Serial.println(p.numero);
    Serial.print(F("Type: "));
    Serial.println(p.type);
    Serial.print(F("Position: "));
    Serial.println(p.position);
    Serial.print(F("Etat: "));
    Serial.println(p.etat);
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
    bool retraitParCritere = recuperer && commande.nombreArgs == 2;
    // Accepte aussi l'ancienne manette avec son champ supplémentaire ignoré.
    bool ancienStockage = stocker && commande.nombreArgs == 4;
    if (!retraitParCritere && !ancienStockage && commande.nombreArgs != (stocker ? 3 : vider ? 0 : 1)) {
        return "ARGUMENTS_INVALIDES";
    }

    String cible;
    if (stocker) {
        cible = commande.args[2];
        if ((commande.args[0] != "-" && (!chiffres(commande.args[0]) || commande.args[0].length() > 20)) ||
            !texteValide(commande.args[1])) return "OBJET_INVALIDE";
    } else if (!vider && !retraitParCritere) {
        cible = commande.args[0];
    }
    bool automatique = stocker && cible == "-";
    if (!initialiserSD()) return "SD_OU_CSV_INDISPONIBLE";
    if (retraitParCritere) {
        const char* erreurChoix = scanPiece(commande, cible);
        if (erreurChoix) return erreurChoix;
    }
    if (!vider && !automatique && !positionValide(cible)) return "POSITION_INVALIDE";

    // Premier passage : vérifier l'état, repérer la cible et refuser les doublons.
    File32 fichier;
    if (!ouvrirCSV(fichier)) return "LECTURE_CSV";
    Piece piece;
    Lecture lecture;
    int occurrences = 0;
    Piece pieceCible;
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
            pieceCible = piece;
        }
    }
    fichier.close();
    if (erreur) return erreur;
    if (lecture != FIN_CSV) return "CSV_INVALIDE";
    if (!vider) {
        if (ajouter && occurrences) return "POSITION_DEJA_UTILISEE";
        if (!ajouter && occurrences != 1) return automatique ? "AUCUNE_CASE_LIBRE" : "POSITION_INCONNUE_OU_AMBIGUE";
        if ((stocker || retirer) && pieceCible.etat != "-") return "POSITION_OCCUPEE";
        if (recuperer && pieceCible.etat != "1") return "PIECE_NON_DISPONIBLE";
    }

    // L'action physique se branche après validation et avant toute écriture.
    if (stocker) {
        if (!creationPiece(commande, pieceCible)) return "DEPOT_NON_CONFIRME";
    } else if (recuperer && !allerChercher(pieceCible.numero, pieceCible.type, pieceCible.position,
                                         pieceCible.etat)) {
        return "RETRAIT_NON_CONFIRME";
    }

    // Si la SD échoue après un mouvement réel, vérifier la case avant de réessayer.
    if (!dataBaseUpdate(action, cible, pieceCible)) return "ECRITURE_CSV";
    if (recuperer) {
        pieceExistante = pieceCible;
        afficheInfos(pieceExistante);
    }
    return nullptr;
}
