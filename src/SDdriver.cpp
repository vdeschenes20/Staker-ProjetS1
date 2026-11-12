//**************************************
// Projet: staker-S1
// Fichiers: SDdriver.cpp
// Equipe: P13
// Auteurs: 
// Description: Breve description du script
// Date: 24/09/2026
//***************************************

//*********INCLUDES********//
#include <Arduino.h>
#include "SdFat.h"
#include "SDdriver.h"

//********CONSTANTES*******//

//********VARIABLES********//
SdFat sd;
File32 dataFile;

//********FONCTIONS********//
void intitialisationSD(void)
{
  while(!sd.begin(CS_PIN, SD_SCK_MHZ(4))) {
    Serial.println("Échec de l'initialisation");
  }
  Serial.println("Initialisation réussie");
}

String trouverLigne(int colonneCible, const String &valeurRecherchee)
{
  File32 f;
  if (!f.open("/data.csv", O_RDONLY)) {
    Serial.println(F("Erreur : impossible d'ouvrir le fichier en lecture."));
    while(!sd.begin(CS_PIN, SD_SCK_MHZ(4))) {
    Serial.println("Échec de l'initialisation");
    }
    return "";
  }
 
  char ligne[100];
  char copie[100];
  String resultat = "";
  int numeroLigne = 0;
 
  while (f.available()) {
    int n = f.fgets(ligne, sizeof(ligne));
    numeroLigne++;
    if (n <= 0) break;
 
    // Retire \r et \n à la fin
    while (n > 0 && (ligne[n - 1] == '\n' || ligne[n - 1] == '\r')) {
      ligne[--n] = '\0';
    }
    if (n == 0) continue;  // ligne vide
 
    // On découpe une copie pour garder la ligne intacte
    strcpy(copie, ligne);
 
    int col = 0;
    char *debut = copie;
    while (true) {
      char *fin = strchr(debut, ';');
      if (fin) *fin = '\0';  // termine le champ courant
 
      if (col == colonneCible) {
        if (strcmp(debut, valeurRecherchee.c_str()) == 0) {
          resultat = ligne;  // ligne complète d'origine
        }
        break;  // inutile d'aller plus loin sur cette ligne
      }
 
      if (!fin) break;  // dernière colonne atteinte
      debut = fin + 1;
      col++;
    }
 
    if (resultat.length() > 0) break;  // trouvé
  }
 
  f.close();
  if(resultat == "")
  {
    return "";
  }
  return String(numeroLigne) + ";" + resultat;
}

bool updateCSV(int ligneCible, int colonneCible, String nouvelleValeur)
{
  File32 fmodif;
  if (!fmodif.open("/data.csv", O_RDWR)) {
    Serial.println(F("Erreur : impossible d'ouvrir data.csv en lecture/écriture."));
    // fmodif.close();
    while(!sd.begin(CS_PIN, SD_SCK_MHZ(4))) {
    Serial.println("Échec de l'initialisation");
    }
    return false;
  }
 
  char ligne[100];
  int numeroLigne = 0;
 
  while (fmodif.available()) {
    uint32_t debutLigne = fmodif.curPosition();  // position de cette ligne dans le fichier
    int n = fmodif.fgets(ligne, sizeof(ligne));
    if (n <= 0) break;
 
    // Retire \r et \n à la fin (n'affecte pas les positions calculées plus bas)
    while (n > 0 && (ligne[n - 1] == '\n' || ligne[n - 1] == '\r')) {
      ligne[--n] = '\0';
    }
 
    if (numeroLigne == ligneCible) {
      int col = 0;
      char *debut = ligne;
      while (true) {
        char *fin = strchr(debut, ';');
 
        if (col == colonneCible) {
          // Position exacte de la cellule dans le fichier
          // (la longueur de la nouvelle valeur doit être vérifiée par l'appelant)
          fmodif.seekSet(debutLigne + (uint32_t)(debut - ligne));
          fmodif.write(nouvelleValeur.c_str(), nouvelleValeur.length());
          fmodif.flush();
          fmodif.close();  // close() vide aussi le tampon vers la carte
          return true;
        }
 
        if (!fin) break;  // dernière colonne atteinte sans trouver la cible
        debut = fin + 1;
        col++;
      }
      fmodif.flush();
      fmodif.close();
      Serial.println(F("Colonne introuvable sur cette ligne."));
      return false;
    }
    numeroLigne++;
  }
 
  fmodif.flush();
  fmodif.close();
  Serial.println(F("Ligne introuvable."));
  return false;
}

int trouverNo(void)
{
  File32 f;
  if (!f.open("/data.csv", O_RDONLY)) {
    Serial.println(F("Erreur : impossible d'ouvrir le fichier en lecture."));
    return -1;
  }
 
  char ligne[100];
  int maximum = -1;
  bool trouve = false;
 
  while (f.available()) {
    int n = f.fgets(ligne, sizeof(ligne));
    if (n <= 0) break;
 
    // Isole la première colonne (jusqu'au premier ';')
    char *fin = strchr(ligne, ';');
    if (fin) *fin = '\0';
 
    // Conversion: on ignore si ce n'est pas un nombre (en-tête, ligne vide...)
    char *finNombre;
    int valeur = strtol(ligne, &finNombre, 10);
    if (finNombre == ligne) continue;
 
    if (!trouve || valeur > maximum) {
      maximum = valeur;
      trouve = true;
    }
  }
 
  f.close();
  return trouve ? maximum : -1;
}