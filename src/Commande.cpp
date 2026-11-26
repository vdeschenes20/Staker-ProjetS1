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
#include <stdlib.h>
#include <string.h>
#include "Commande.h"

//********CONSTANTES*******//

//********VARIABLES********//
Commande cmd;
char tamponLigne[TAILLE_TAMPON];
size_t indexLigne = 0;

//********FONCTIONS********//
void initialisationCommande(void)
{
   hc05.begin(9600);
}

void reinitialiserCommande(Commande &cmd) {
  cmd.id = 0;
  cmd.type = CMD_INCONNUE;
  cmd.parametre1 = 0;
  cmd.parametre2 = 0;
  cmd.position[0] = 0;
  cmd.position[1] = 0;
  cmd.position[2] = 0;
}

// Parse une chaîne "x,y,z" dans position[3]. Retourne true si valide.
bool parserPosition(const char *texte, int posOut[3]) {
  if (texte == nullptr || texte[0] == '\0' || strcmp(texte, "-") == 0) {
    return false;
  }

  char copie[24];
  strncpy(copie, texte, sizeof(copie) - 1);
  copie[sizeof(copie) - 1] = '\0';

  char *tokX = strtok(copie, ",");
  char *tokY = strtok(nullptr, ",");
  char *tokZ = strtok(nullptr, ",");

  if (tokX == nullptr || tokY == nullptr || tokZ == nullptr) {
    return false;
  }

  posOut[0] = atoi(tokX);
  posOut[1] = atoi(tokY);
  posOut[2] = atoi(tokZ);
  return true;
}

int mapperAction(const char *action) {
  if (strcmp(action, "SYNC") == 0) return CMD_SYNC;
  if (strcmp(action, "STOCKER") == 0) return CMD_STOCKER;
  if (strcmp(action, "CHERCHER") == 0) return CMD_CHERCHER;
  if (strcmp(action, "RECUPERER") == 0) return CMD_RECUPERER;
  if (strcmp(action, "AJOUTER_EMPLACEMENT") == 0) return CMD_AJOUTER_EMPLACEMENT;
  if (strcmp(action, "RETIRER_EMPLACEMENT") == 0) return CMD_RETIRER_EMPLACEMENT;
  if (strcmp(action, "RESET_INVENTAIRE") == 0) return CMD_RESET_INVENTAIRE;
  if (strcmp(action, "EMERGENCY_STOP") == 0) return CMD_EMERGENCY_STOP;
  if (strcmp(action, "RESET_ESTOP") == 0) return CMD_RESET_ESTOP;
  return CMD_INCONNUE;
}

int mapperCritere(const char *critere) {
  if (critere == nullptr) return CRITERE_AUCUN;
  if (strcmp(critere, "NUMERO") == 0) return CRITERE_NUMERO;
  if (strcmp(critere, "TYPE_TEXTE") == 0) return CRITERE_TYPE;
  if (strcmp(critere, "POSITION") == 0) return CRITERE_POSITION;
  return CRITERE_AUCUN;
}

// Convertit une ligne "CMD|id|ACTION|..." en struct Commande.
bool parserLigneCommande(const char *ligne, Commande &cmd) {
  reinitialiserCommande(cmd);
  if (ligne == nullptr || ligne[0] == '\0') {
    return false;
  }
  char copie[TAILLE_TAMPON];
  strncpy(copie, ligne, sizeof(copie) - 1);
  copie[sizeof(copie) - 1] = '\0';
  Serial.println(ligne);
  // Découpe les champs séparés par '|'
  // Note : avec strtok, deux '||' consécutifs seraient fusionnés,
  // d'où l'usage de '-' côté Python pour les champs vides.
  const int MAX_CHAMPS = 8;
  char *champs[MAX_CHAMPS] = {nullptr};
  int nbChamps = 0;

  char *courant = copie;
  while (courant != nullptr && nbChamps < MAX_CHAMPS) {
    champs[nbChamps++] = courant;
    char *sep = strchr(courant, '|');
    if (sep != nullptr) {
      *sep = '\0';
      courant = sep + 1;
    } else {
      courant = nullptr;
    }
  }

  // Format attendu : CMD | <id> | <ACTION> | ...
  if (nbChamps < 3 || strcmp(champs[0], "CMD") != 0) {
    return false;
  }

  cmd.id = atoi(champs[1]);
  cmd.type = mapperAction(champs[2]);

  switch (cmd.type) {
    case CMD_STOCKER: {
      // Python envoie : STOCKER | <numero ou -> | <type_piece> | <position ou -> | <nom>
      const char *numero = (nbChamps > 3) ? champs[3] : "-";
      const char *type   = (nbChamps > 4) ? champs[4] : "-";
      const char *posStr = (nbChamps > 5) ? champs[5] : "-";
      // champs[6] = nom (texte)

      cmd.parametre1 = (strcmp(numero, "-") != 0) ? atoi(numero) : 0;
      if (type[0] == '-') {
        return false;
      }
      cmd.parametre2 = static_cast<unsigned char>(type[0]);
      parserPosition(posStr, cmd.position);
      break;
    }

    case CMD_CHERCHER: {
      // Python envoie : CHERCHER | <CRITERE> | <valeur>
      const char *critereStr = (nbChamps > 3) ? champs[3] : "";
      const char *valeurStr = (nbChamps > 4) ? champs[4] : "";

      int critere = mapperCritere(critereStr);
      cmd.parametre1 = critere;

      if (critere == CRITERE_NUMERO) {
        cmd.parametre2 = atoi(valeurStr);
      } else if (critere == CRITERE_POSITION) {
        parserPosition(valeurStr, cmd.position);
      }
      else if(critere == CRITERE_TYPE){
        if (valeurStr[0] == '\0') {
          return false;
        }
        cmd.parametre2 = static_cast<unsigned char>(valeurStr[0]);
        Serial.println(cmd.parametre2);

      }
      break;
    }

    case CMD_RECUPERER:
    case CMD_AJOUTER_EMPLACEMENT:
    case CMD_RETIRER_EMPLACEMENT: 
    case CMD_SYNC:
    case CMD_RESET_INVENTAIRE:
    case CMD_EMERGENCY_STOP:
    case CMD_RESET_ESTOP:
    default:
      break;
  }

  return (cmd.type != CMD_INCONNUE);
}

void executerCommande(const Commande &cmd) {

  hc05.print("ACK|");
  hc05.print(cmd.id);
  hc05.println("|OK");
}

// Lit les octets du HC-05 jusqu'à '\n' sans bloquer loop().
bool lireCommandeHC05(Commande &cmdOut) {
  while (hc05.available() > 0) {
    char c = (char)hc05.read();
    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      tamponLigne[indexLigne] = '\0';
      indexLigne = 0;

      if (strlen(tamponLigne) > 0) {
        return parserLigneCommande(tamponLigne, cmdOut);
      }
    } else if (indexLigne < TAILLE_TAMPON - 1) {
      tamponLigne[indexLigne++] = c;
    } else {
      // Ligne trop longue : on purge jusqu'au prochain '\n'
      indexLigne = 0;
    }
  }
  return false;
}