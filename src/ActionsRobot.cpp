/*
 * Projet Stack3r | Équipe P13
 * ActionsRobot.cpp | Intégration du déplacement et de la pince
 */

#include "ActionsRobot.h"

// La base valide la case avant l'appel. Position : colonne,rang,hauteur.
// Numéro : texte, par exemple "001". "---" signifie aucun numéro.
// État : "1" = placé.
// En mode réel, retourner true après confirmation physique, false si échec.
// Le CSV est modifié seulement après true.
// Pour un trajet long, prévoir une machine à états pour continuer à lire
// la manette et l'arrêt. Ces fonctions synchrones servent au test.

bool allerPlacer(const String& numero, const String& type, const String& position,
                 const String& etat) {
    // À compléter — Olivier
    (void)numero;
    (void)type;
    (void)position;
    (void)etat;
    return true;  // Test d'inventaire : dépôt simulé.
}

bool allerChercher(const String& numero, const String& type, const String& position,
                   const String& etat) {
    // À compléter — Olivier 
    (void)numero;
    (void)type;
    (void)position;
    (void)etat;
    return true;  // Test d'inventaire : retrait simulé.
}

void arretUrgence() {
    // À compléter — Olivier : arrêter les moteurs et annuler le trajet actif.
    // Le réarmement ne doit pas relancer automatiquement le trajet annulé.
    // Test d'inventaire : aucun pilote moteur n'est appelé.
}
