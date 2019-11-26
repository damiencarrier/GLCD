/*
 * GLCD.h
 *
 *  Created on: 13 nov. 2019
 *      Author: damien carrier
 */
// Déclaration des fichiers d'en-têtes
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lcd_font_5x7.h"
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
//mesure toutes les tensions
void mesure_tension();
//fait la moyenne sur 12 valeurs
//écarte les 2 extremes
void mesure_tension_moyennee();
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
//fonction principale affichant toutes les tensions
void GLCD_Code();
