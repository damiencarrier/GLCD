/*
 * main.c
 *
 *  Created on: 12 nov. 2019
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
#include "GLCD.h"
#include "ADC.h"
#include "commun.h"

int main()
{
	initialiser_ecran();

	ecran_blanc(GAUCHE);
	_delay_ms(100);
	ecran_blanc(DROIT);
	_delay_ms(100);
	ecran_noir(GAUCHE);
	_delay_ms(100);
	ecran_noir(DROIT);
	_delay_ms(100);
	masque_ecran();

	do {
		//mesure toutes les tensions 12 fois
		for(int i=0;i<12;i++){
			mesure_tension(i);
		}
		mesure_tension_moyennee();

		//affiche toutes les valeurs sur le LCD

		//tension batterie
		glcd_write_nombre(GAUCHE,1,tension_pinf_moyenne[1]);
		//courant batterie élévateur
		glcd_write_nombre(GAUCHE,2,tension_pinf_moyenne[2]);
		//tension élévateur
		glcd_write_nombre(DROIT ,1,tension_pinf_moyenne[3]);
		//courant élévateur
		glcd_write_nombre(DROIT ,2,tension_pinf_moyenne[4]);
		//puissance entrée élévateur
		glcd_write_nombre(GAUCHE,3,puissance_entree_E);
		//puissance élévateur
		glcd_write_nombre(DROIT ,3,puissance_E);
		//rendement élévateur
		glcd_write_nombre(GAUCHE,4,rendement_E);
		//rendement abaisseur
		glcd_write_nombre(GAUCHE,5,rendement_A);
		//tension abaisseur
		glcd_write_nombre(DROIT ,5,tension_pinf_moyenne[5]);
		//courant entrée abaisseur
		glcd_write_nombre(GAUCHE,6,tension_pinf_moyenne[7]);
		//courant abaisseur
		glcd_write_nombre(DROIT ,6,tension_pinf_moyenne[6]);
		//puissance entrée abaisseur
		glcd_write_nombre(GAUCHE,7,puissance_entree_A);
		//puissance abaisseur
		glcd_write_nombre(DROIT ,7,puissance_A);

		_delay_ms(150);
		}while(1);
	return 0;
}
