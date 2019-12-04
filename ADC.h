/*
 * ADC.h
 *
 *  Created on: 26 nov. 2019
 *      Author: damien carrier
 */

#ifndef ADC_H_
#define ADC_H_
#endif /* ADC_H_ */

// Déclaration des fichiers d'en-têtes
//#include <avr/io.h>
//#include <util/delay.h>
//#include <avr/interrupt.h>
//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include "lcd_font_5x7.h"
#include "commun.h"

//8 tableaux pour chaque pin de mesure
//qui contient 12 mesures chacunes
//pin0 est la mesure du 4096mv
//unsigned long tension_pinf[8][12];
//unsigned long tension_pinf_moyenne[8];
typedef struct{
	unsigned long tension_pinf[12];//tension en mv
	unsigned long tension_pinf_moyenne;//tension moyenne en mv
	unsigned char status; //0x01 quand la moyenne est faite
}structure_tension;

// Les 8 structures pour chaque voltmetre
structure_tension base_donnees[8];
unsigned long puissance_E,puissance_A,puissance_entree_E,puissance_entree_A,rendement_E,rendement_A;

void mesure_tension(unsigned int numero_mesure);
//fait la moyenne sur 12 valeurs
//écarte les 2 extremes
void mesure_tension_moyennee();
//construit les chiffres pour l'afficher à l'écran
