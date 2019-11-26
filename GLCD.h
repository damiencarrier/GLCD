/*
 * GLCD.h
 *
 *  Created on: 13 nov. 2019
 *      Author: damien carrier
 */

void ecran_noir_gauche();
void ecran_noir_droit();
void ecran_blanc_gauche();
void ecran_blanc_droit();
void initialiser_ecran();
void ecran_disponible_gauche();
void ecran_disponible_droit();
void afficher_carac_gauche(unsigned int ligne_font);
void afficher_carac_droit(unsigned int ligne_font);
unsigned int numero_lettre(unsigned char lettre);
void mesure_tension();

void GLCD_Code();
