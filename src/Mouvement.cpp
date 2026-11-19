/*
 * Mouvement.cpp - P13
 * Commande des moteurs avec retour des encodeurs et correction PID.
 */

#include <Arduino.h>
#include <math.h>
#include <stdlib.h>
#include <LibRobus.h>
#include <Mouvement.h>

// Calibration mecanique : toutes les distances sont en centimetres.
static constexpr float DIAMETRE_ROUE_CM = 7.8f;
static constexpr float TICKS_PAR_TOUR_G = 3200.0f;
static constexpr float TICKS_PAR_TOUR_D = 3200.0f;
static constexpr float PI_APPROX = 3.14159265f;
static constexpr float ECART_ROUES_CM = 18.5f;
static constexpr float CM_PAR_TICK_G = PI_APPROX * DIAMETRE_ROUE_CM / TICKS_PAR_TOUR_G;
static constexpr float CM_PAR_TICK_D = PI_APPROX * DIAMETRE_ROUE_CM / TICKS_PAR_TOUR_D;
static constexpr float CIRCONFERENCE_TOURNER = PI_APPROX * ECART_ROUES_CM;

// Vitesses de base calibrées. A TWEAK SI ON EST DÉSESPÉRÉ
static constexpr float FACTEUR_VITESSE = 1.5f;
static constexpr float VITESSE_GAUCHE = 0.40f * FACTEUR_VITESSE;
static constexpr float VITESSE_DROITE = 0.40f * FACTEUR_VITESSE;
static constexpr float VITESSE_GAUCHE_TOURNE = 0.40f;
static constexpr float VITESSE_DROITE_TOURNE = 0.40f;

// Reglages PID initiaux.       ON TWEAK EN PRIO POUR ALLER EN LIGNE DROITE
static constexpr float KP = 0.0015f;
//  Kp corrige l'erreur entre le roues en temps réel. Plus grand chiffre = plus grande compensation
static constexpr float KI = 0.00008f;
//  Ki corrige un ecart persistant entre les moteurs et le résultat attendu a cause de la friction par exemple. ''
static constexpr float KD = 0.00015f;
//  Kd amortit les changements rapides. pour pas overshoot la correction. ''

static constexpr float CORRECTION_MAX = 0.22f;
static constexpr float INTEGRALE_MAX = 1500.0f;
static constexpr float ERREUR_TICKS_FIN = 8.0f;
static constexpr unsigned long PERIODE_PID_MS = 10;
static constexpr unsigned long DELAI_MAX_MOUVEMENT_MS = 15000;
static constexpr float COMMANDE_MIN = 0.18f;
static constexpr unsigned long DUREE_RAMPE_MS = 350;
static constexpr float VITESSE_DEPART_MIN = 0.16f;
static constexpr float DISTANCE_RALENTISSEMENT_AVANT_TICKS = 1500.0f;
static constexpr float FACTEUR_VITESSE_FIN_AVANT = 0.30f;



// Limite une valeur a l'intervalle [minimum, maximum].
static float limiter(float valeur, float minimum, float maximum) {
    if (valeur < minimum) return minimum;
    if (valeur > maximum) return maximum;
    return valeur;
}



// Convertit une distance en centimetres en ticks pour un encodeur.
static long distanceEnTicks(float distanceCM, bool gauche) {
    const float cmParTick = gauche ? CM_PAR_TICK_G : CM_PAR_TICK_D;
    return lroundf(distanceCM / cmParTick);
}



// Envoie une commande aux moteurs en conservant un minimum utile si necessaire.
static float commandeAvecMinimum(float commande, float minimum) {
    if (commande > 0.0f && commande < minimum) return minimum;
    if (commande < 0.0f && commande > -minimum) return -minimum;
    return commande;
}



void arret() {
    MOTOR_SetSpeed(LEFT, 0.0f);
    MOTOR_SetSpeed(RIGHT, 0.0f);
}










static void mouvementDistance(float distanceCM, float sens) {
    if (distanceCM <= 0.0f) {
        arret();
        return;
    }

    const long cibleG = distanceEnTicks(distanceCM, true);
    const long cibleD = distanceEnTicks(distanceCM, false);

    ENCODER_Reset(0);
    ENCODER_Reset(1);

    float integrale = 0.0f;
    float erreurPrecedente = 0.0f;
    unsigned long debut = millis();
    unsigned long dernierCalcul = debut;

    while (true) {
        const long ticksG = labs(ENCODER_Read(0));
        const long ticksD = labs(ENCODER_Read(1));
        const long restantG = cibleG - ticksG;
        const long restantD = cibleD - ticksD;

        // Termine seulement lorsque les deux roues sont proches de leur cible.
        if (restantG <= ERREUR_TICKS_FIN && restantD <= ERREUR_TICKS_FIN) break;

        const unsigned long maintenant = millis();
        if (maintenant - debut >= DELAI_MAX_MOUVEMENT_MS) break;
        if (maintenant - dernierCalcul < PERIODE_PID_MS) {
            delay(1);
            continue;
        }

        const float dt = (maintenant - dernierCalcul) / 1000.0f;
        dernierCalcul = maintenant;

        // Erreur positive : la roue droite a pris de l'avance; ralentir droite
        // et accelerer gauche. Le signe est volontairement basé sur ticksG-ticksD.
        const float erreur = (float)ticksD - (float)ticksG;
        integrale = limiter(integrale + erreur * dt, -INTEGRALE_MAX, INTEGRALE_MAX);
        const float derivee = (erreur - erreurPrecedente) / dt;
        erreurPrecedente = erreur;

        const float correction = limiter(
            KP * erreur + KI * integrale + KD * derivee,
            -CORRECTION_MAX, CORRECTION_MAX);

        // Rampe de demarrage : augmente progressivement la commande pour eviter
        // le coup initial, puis ralentit a l'approche de la cible.
        const float progressionRestante = (float)min(restantG, restantD);

        const float facteurApproche = limiter(progressionRestante / DISTANCE_RALENTISSEMENT_AVANT_TICKS, FACTEUR_VITESSE_FIN_AVANT, 1.0f);
        const float facteurDepart = limiter((maintenant - debut) / (float)DUREE_RAMPE_MS, 0.0f, 1.0f);
        const float facteurVitesse = VITESSE_DEPART_MIN + (1.0f - VITESSE_DEPART_MIN) * facteurDepart;

        float commandeG = VITESSE_GAUCHE * facteurApproche * facteurVitesse + correction;
        float commandeD = VITESSE_DROITE * facteurApproche * facteurVitesse - correction;

        // Une roue qui a atteint sa cible s'arrete pendant que l'autre termine.
        if (restantG <= ERREUR_TICKS_FIN) commandeG = 0.0f;
        else commandeG = commandeAvecMinimum(commandeG, COMMANDE_MIN);
        if (restantD <= ERREUR_TICKS_FIN) commandeD = 0.0f;
        else commandeD = commandeAvecMinimum(commandeD, COMMANDE_MIN);

        MOTOR_SetSpeed(LEFT, sens * limiter(commandeG, 0.0f, 1.0f));
        MOTOR_SetSpeed(RIGHT, sens * limiter(commandeD, 0.0f, 1.0f));
    }

    arret();
}

// Le PID synchronise les deux roues en marche avant comme en marche arriere.
void mouvementAvant(float distanceCM) {
    mouvementDistance(distanceCM, 1.0f);
}

void mouvementArriere(float distanceCM) {
    mouvementDistance(distanceCM, -1.0f);
}
/*
void tourne(int p_sens, float p_vitesse) {

}

void tournePlus(){

}

*/

// Tourne sur place : angle positif vers la droite, negatif vers la gauche.
// Le PID compare les distances parcourues en valeur absolue pour synchroniser
// les roues pendant la rotation, sans modifier le sens de chaque moteur.
void tourne(float angleDegres) {
    if (angleDegres == 0.0f) {
        arret();
        return;
    }

    const float distanceRoueCM = fabsf(angleDegres) * CIRCONFERENCE_TOURNER / 360.0f;
    const long cibleG = distanceEnTicks(distanceRoueCM, true);
    const long cibleD = distanceEnTicks(distanceRoueCM, false);

    ENCODER_Reset(0);
    ENCODER_Reset(1);

    const float sens = angleDegres > 0.0f ? 1.0f : -1.0f;
    float integrale = 0.0f;
    float erreurPrecedente = 0.0f;
    unsigned long debut = millis();
    unsigned long dernierCalcul = debut;

    while (true) {
        const long ticksG = labs(ENCODER_Read(0));
        const long ticksD = labs(ENCODER_Read(1));
        const long restantG = cibleG - ticksG;
        const long restantD = cibleD - ticksD;

        if (restantG <= ERREUR_TICKS_FIN && restantD <= ERREUR_TICKS_FIN) break;

        const unsigned long maintenant = millis();
        if (maintenant - debut >= DELAI_MAX_MOUVEMENT_MS) break;
        if (maintenant - dernierCalcul < PERIODE_PID_MS) {
            delay(1);
            continue;
        }

        const float dt = (maintenant - dernierCalcul) / 1000.0f;
        dernierCalcul = maintenant;

        const float erreur = (float)ticksD - (float)ticksG;
        integrale = limiter(integrale + erreur * dt, -INTEGRALE_MAX, INTEGRALE_MAX);
        const float derivee = (erreur - erreurPrecedente) / dt;
        erreurPrecedente = erreur;

        const float correction = limiter(
            KP * erreur + KI * integrale + KD * derivee,
            -CORRECTION_MAX, CORRECTION_MAX);

        // Rampe de demarrage pour eviter une rotation brusque au premier instant.
        const float progressionRestante = (float)min(restantG, restantD);
        const float facteurApproche = limiter(progressionRestante / 250.0f, 0.35f, 1.0f);
        const float facteurDepart = limiter(
            (maintenant - debut) / (float)DUREE_RAMPE_MS, 0.0f, 1.0f);
        const float facteurVitesse = VITESSE_DEPART_MIN +
            (1.0f - VITESSE_DEPART_MIN) * facteurDepart;
        float commandeG = VITESSE_GAUCHE_TOURNE * facteurApproche * facteurVitesse + correction;
        float commandeD = VITESSE_DROITE_TOURNE * facteurApproche * facteurVitesse - correction;

        if (restantG <= ERREUR_TICKS_FIN) commandeG = 0.0f;
        else commandeG = commandeAvecMinimum(commandeG, COMMANDE_MIN);
        if (restantD <= ERREUR_TICKS_FIN) commandeD = 0.0f;
        else commandeD = commandeAvecMinimum(commandeD, COMMANDE_MIN);

        MOTOR_SetSpeed(LEFT, sens * limiter(commandeG, 0.0f, 1.0f));
        MOTOR_SetSpeed(RIGHT, -sens * limiter(commandeD, 0.0f, 1.0f));
    }

    arret();
}