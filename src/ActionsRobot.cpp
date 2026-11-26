/*
 * Projet Stack3r | Équipe P13
 * ActionsRobot.cpp | Intégration du déplacement et de la pince
 */

#include "ActionsRobot.h"
#include "DataBase.h"

// POUR LA PERSONNE QUI CODE LE MOUVEMENT
// Le robot a déjà validé l'objet et choisi une case unique.
// objet.position contient colonne,rang,hauteur, par exemple "4,1,1".
// objet.nom, objet.type et objet.numero donnent les infos de la pièce.
// Retourner true seulement quand le dépôt ou le retrait physique est confirmé.
// Retourner false si le trajet, la pince ou la vérification échoue.
// DataBase écrit le CSV après true. Un échec laisse la case telle quelle.
// Pour un trajet long, prévoir une machine à états dans loop() pour continuer
// de lire le Bluetooth et l'arrêt. Ces fonctions synchrones servent au test.

// STOCKER : prendre l'objet, aller à la case, déposer puis vérifier.
bool rangerObjet(const Piece& objet) {
    (void)objet;
    return true;  // Test d'inventaire : dépôt simulé.
}

// RECUPERER : aller à la case, saisir l'objet puis vérifier la prise.
bool recupererObjet(const Piece& objet) {
    (void)objet;
    return true;  // Test d'inventaire : retrait simulé.
}

// ARRÊT : brancher ici l'arrêt des moteurs et l'annulation du trajet actif.
// Le réarmement ne doit pas relancer automatiquement un trajet annulé.
void arreterMouvement() {
    // Test d'inventaire : aucun pilote moteur n'est appelé.
}
