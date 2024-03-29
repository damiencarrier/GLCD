/*
 * GLCD.h
 *
 *  Created on: 13 nov. 2019
 *      Author: damien carrier
 */
// Déclaration des fichiers d'en-têtes
#include "commun.h"

//met en noir l'écran sélectionné
void ecran_noir(GAUCHE_DROIT cote);
//met en blanc l'écran sélectionné
void ecran_blanc(GAUCHE_DROIT cote);
//initialise l'écran avant de commencer
void initialiser_ecran();
//attend que l'écran sélectionné soit disponible
void ecran_disponible(GAUCHE_DROIT cote);
//affiche sur l'écran le caractère donnée
void afficher_carac(GAUCHE_DROIT cote ,unsigned int ligne_font);
//construit les chiffres pour l'afficher à l'écran
void glcd_write_nombre(GAUCHE_DROIT cote, u8 ligne, u32 nombre);
//donne la valeur de la lettre dans la font
unsigned int numero_lettre(unsigned char lettre);
// le masque de l'écran qui affiche les information fixes de l'écran
void masque_ecran();
//positionne la ligne sur laquelle s'affichera les caractères
void glcd_y (GAUCHE_DROIT cote, u8 no_ligne);
//positionne le "curseur" sur le bon caractere
void glcd_x (GAUCHE_DROIT cote, u8 no_caractere);
