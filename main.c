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
//#include "lcd_font_5x7.h"
#include "GLCD.h"
#include "ADC.h"
#include "commun.h"
//#include "timer.h"


//en fonction de la carte utilisée, on utilise pas les memes ports
//pour la carte uni_ds6 CTRL=PORTA DATA=PORTC
#define CTRL PORTA
#define CTRL_DIR DDRA
#define DATAW PORTC
#define DATAR PINC
#define DATA_DIR DDRC
// pour la bigavr6

// envoi sur PORTA
// rest l'écran apresz avoir choisi l'un des 2 écrans
#define RESET 0x3C
//
#define IDLE 0xBC

#define WRITE_CDE 0x80
#define WRITE_CDE_E 0xC0
#define READ_STATUS 0xA0
#define READ_STATUS_E 0xE0
#define WRITE_RAM 0x90
#define WRITE_RAM_E 0xD0
// lis les
#define READ_RAM 0xB0
#define READ_RAM_E 0xF0

// envoi sur PORTC
//allume l'écran
#define DISPLAY_ON 0x3F
//éteint l'écran
#define DISPLAY_OFF 0x3E
// définis la colonne a afficher de 0à63
#define COLUMN 0x40
// définis la page a afficher de 0à7
#define PAGE 0xB8

// création d'une commande assembleur pour faire un délai de 100ns (1 coup d'horloge)
#define NOP __asm__("nop");
//#define NOP asm("nop");

	//on veut un top tout les 8.5ms
	// on met le prescaler a 1024
	// on a donc besoin de compte 8.5/102.4 > 83
//	ISR(TIMER0_OVF_vect){
//		//flag_timer0_0VF = 1;
//	}

static char mode_affichage=0;//,compteur_affichage=0,flag_affichage=0;

//ecriture du N° de colonne dans la colonne, a gauche, puis l'inverse a droite
unsigned char no_colonne, no_page;

unsigned long reglage_quantum = 409100;

//la routine d'intéruption
ISR(TIMER0_COMP_vect){
	PORTB |= 0b00000001;
	static int interuption = 0;
	//TCNT0=0x00;
	//PORTB=0xF0;
	if(interuption < 12){
		mesure_tension(interuption);
		interuption++;
	}else{
		mesure_tension_moyennee();
		interuption = 0;
	}

//	if (compteur_affichage<20){
//		compteur_affichage ++;
//		//PORTB |= 0b00000010;
//		PORTB &= ~0b00000100;
//		flag_affichage=0;
//	}else{
//		compteur_affichage=0;
//		flag_affichage=1;
//		PORTB |= 0b00000100;
//	}
//	PORTB &= ~0b00000001;
}

ISR(INT7_vect)
{
	//PORTB = 0b11111111;
	_delay_ms(100);
	if (mode_affichage ==2){
	mode_affichage = 0;
	}else mode_affichage = 2;
}

ISR(INT5_vect)
{
	_delay_ms(100);
	reglage_quantum= reglage_quantum -100;
	PORTB |= 0b00000100;
}

ISR(INT6_vect)
{
	_delay_ms(100);
	reglage_quantum= reglage_quantum +100;
}

//initialisation du timer
void timer0_init() {
	TCCR0=0b00001111;
	OCR0=82;
	TCNT0=0;
	TIMSK=0x02;
}


int main(){
	//pin de débug pour l'analyseur logique
	DDRB = 0xFF;
	PORTB = 0b00000000;

	//intéruption sur le porte7
	// Change pin 7 on bus E to an input by changing bit 7 to zero
	//DDRE &= ~(1 << PIN7);
	DDRE = 0x00;

	// Defining a pull-up resistor to to pin 7 on bus E
	// to prevent input floating
	//PORTE |= (1 << PIN7);
	PORTE = 0b00000000;

	// Set the interrupt mode to logical change for interrupt 7
	// in the external interrupt configuration register
	EICRB = 0b11111100;//(1 << ISC70);
	//EICRA = 0b00001111;//(1 << ISC70);

	// Allow external interrupt 7
	//EIMSK |= (1 << INT7);
	EIMSK = 0b11100000;


	timer0_init();
	sei();
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
		//if( flag_affichage==0){
			//flag_affichage=0;


			if(mode_affichage == 0){
				masque_ecran();
				mode_affichage ++;
			}else if(mode_affichage == 1){
				//mesure toutes les tensions 12 fois
				//		for(int i=0;i<12;i++){
				//			mesure_tension(i);
				//		}
				//		mesure_tension_moyennee();

				//affiche toutes les valeurs sur le LCD

				//tension batterie
				glcd_write_nombre(GAUCHE,1,base_donnees[1].tension_pinf_moyenne);
				//courant batterie élévateur
				glcd_write_nombre(GAUCHE,2,base_donnees[2].tension_pinf_moyenne);
				//tension élévateur
				glcd_write_nombre(DROIT ,1,base_donnees[3].tension_pinf_moyenne);
				//courant élévateur
				glcd_write_nombre(DROIT ,2,base_donnees[4].tension_pinf_moyenne);
				//puissance entrée élévateur
				glcd_write_nombre(GAUCHE,3,puissance_entree_E);
				//puissance élévateur
				glcd_write_nombre(DROIT ,3,puissance_E);
				//rendement élévateur
				glcd_write_nombre(GAUCHE,4,rendement_E);
				//rendement abaisseur
				glcd_write_nombre(GAUCHE,5,rendement_A);
				//tension abaisseur
				glcd_write_nombre(DROIT ,5,base_donnees[5].tension_pinf_moyenne);
				//courant entrée abaisseur
				glcd_write_nombre(GAUCHE,6,base_donnees[7].tension_pinf_moyenne);
				//courant abaisseur
				glcd_write_nombre(DROIT ,6,base_donnees[6].tension_pinf_moyenne);
				//puissance entrée abaisseur
				glcd_write_nombre(GAUCHE,7,puissance_entree_A);
				//puissance abaisseur
				glcd_write_nombre(DROIT ,7,puissance_A);
			}else{
				ecran_noir(GAUCHE);
				ecran_noir(DROIT);
				// affiche le mode technicien
				// affiche la température
				// affiche le quantum
				// permet la modification

				ecran_disponible(GAUCHE);

				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 0;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| GAUCHE);
				CTRL = (WRITE_CDE_E	| GAUCHE);
				CTRL = (WRITE_CDE 	| GAUCHE);
				CTRL = IDLE;

				ecran_disponible(GAUCHE);

				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 0;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| GAUCHE);
				CTRL = (WRITE_CDE_E	| GAUCHE);
				CTRL = (WRITE_CDE 	| GAUCHE);
				CTRL = IDLE;

				afficher_carac(GAUCHE,numero_lettre(' '));
				afficher_carac(GAUCHE,numero_lettre('R'));
				afficher_carac(GAUCHE,numero_lettre('E'));
				afficher_carac(GAUCHE,numero_lettre('G'));
				afficher_carac(GAUCHE,numero_lettre('L'));
				afficher_carac(GAUCHE,numero_lettre('A'));
				afficher_carac(GAUCHE,numero_lettre('G'));
				afficher_carac(GAUCHE,numero_lettre('E'));
				afficher_carac(GAUCHE,numero_lettre(' '));
				afficher_carac(GAUCHE,numero_lettre(' '));

				ecran_disponible(GAUCHE);

				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 1;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| GAUCHE);
				CTRL = (WRITE_CDE_E	| GAUCHE);
				CTRL = (WRITE_CDE 	| GAUCHE);
				CTRL = IDLE;
				ecran_disponible(GAUCHE);

				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 0;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| GAUCHE);
				CTRL = (WRITE_CDE_E	| GAUCHE);
				CTRL = (WRITE_CDE 	| GAUCHE);
				CTRL = IDLE;

				//tension V batterie
				afficher_carac(GAUCHE,numero_lettre(' '));
				afficher_carac(GAUCHE,numero_lettre('q'));
				afficher_carac(GAUCHE,numero_lettre('u'));
				afficher_carac(GAUCHE,numero_lettre('a'));
				afficher_carac(GAUCHE,numero_lettre('n'));
				afficher_carac(GAUCHE,numero_lettre('t'));
				afficher_carac(GAUCHE,numero_lettre('u'));
				afficher_carac(GAUCHE,numero_lettre('m'));
				afficher_carac(GAUCHE,numero_lettre(' '));
				afficher_carac(GAUCHE,numero_lettre(' '));

				glcd_write_nombre(GAUCHE ,2,((reglage_quantum/10)-40000));

				ecran_disponible(GAUCHE);

				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 3;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| GAUCHE);
				CTRL = (WRITE_CDE_E	| GAUCHE);
				CTRL = (WRITE_CDE 	| GAUCHE);
				CTRL = IDLE;

				ecran_disponible(GAUCHE);

				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 0;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| GAUCHE);
				CTRL = (WRITE_CDE_E	| GAUCHE);
				CTRL = (WRITE_CDE 	| GAUCHE);
				CTRL = IDLE;

				//tension référence 4096mv
				afficher_carac(GAUCHE,numero_lettre(' '));
				afficher_carac(GAUCHE,numero_lettre('r'));
				afficher_carac(GAUCHE,numero_lettre('e'));
				afficher_carac(GAUCHE,numero_lettre('f'));
				afficher_carac(GAUCHE,numero_lettre('e'));
				afficher_carac(GAUCHE,numero_lettre('r'));
				afficher_carac(GAUCHE,numero_lettre('e'));
				afficher_carac(GAUCHE,numero_lettre('n'));
				afficher_carac(GAUCHE,numero_lettre('c'));
				afficher_carac(GAUCHE,numero_lettre('e'));

				glcd_write_nombre(GAUCHE ,4,base_donnees[0].tension_pinf_moyenne*10);

				//afficher les tension sur le panneau de droite

				ecran_disponible(DROIT);

				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 0;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;

				ecran_disponible(DROIT);

				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 0;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;

				afficher_carac(DROIT,numero_lettre(' '));
				afficher_carac(DROIT,numero_lettre('T'));
				afficher_carac(DROIT,numero_lettre('e'));
				afficher_carac(DROIT,numero_lettre('n'));
				afficher_carac(DROIT,numero_lettre('s'));
				afficher_carac(DROIT,numero_lettre('i'));
				afficher_carac(DROIT,numero_lettre('o'));
				afficher_carac(DROIT,numero_lettre('n'));
				afficher_carac(DROIT,numero_lettre('s'));
				afficher_carac(DROIT,numero_lettre(' '));

				ecran_disponible(DROIT);

				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 1;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;

				ecran_disponible(DROIT);

				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 0;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;

				afficher_carac(DROIT,numero_lettre('V'));
				afficher_carac(DROIT,numero_lettre('1'));

				ecran_disponible(DROIT);
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 54;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;
				afficher_carac(DROIT,numero_lettre('V'));

				glcd_write_nombre(DROIT ,1,base_donnees[1].tension_pinf_moyenne);

				ecran_disponible(DROIT);
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 2;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;

				ecran_disponible(DROIT);
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 0;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;

				afficher_carac(DROIT,numero_lettre('V'));
				afficher_carac(DROIT,numero_lettre('2'));

				ecran_disponible(DROIT);
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 54;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;
				afficher_carac(DROIT,numero_lettre('V'));

				glcd_write_nombre(DROIT ,2,base_donnees[2].tension_pinf_moyenne);

				ecran_disponible(DROIT);
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 3;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;

				ecran_disponible(DROIT);

				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 0;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;

				afficher_carac(DROIT,numero_lettre('V'));
				afficher_carac(DROIT,numero_lettre('3'));

				ecran_disponible(DROIT);
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 54;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;
				afficher_carac(DROIT,numero_lettre('V'));

				glcd_write_nombre(DROIT ,3,base_donnees[3].tension_pinf_moyenne);

				ecran_disponible(DROIT);
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 4;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;

				ecran_disponible(DROIT);

				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 0;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;

				afficher_carac(DROIT,numero_lettre('V'));
				afficher_carac(DROIT,numero_lettre('4'));

				ecran_disponible(DROIT);
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 54;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;
				afficher_carac(DROIT,numero_lettre('V'));

				glcd_write_nombre(DROIT ,4,base_donnees[4].tension_pinf_moyenne);

				ecran_disponible(DROIT);
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 5;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;

				ecran_disponible(DROIT);

				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 0;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;

				afficher_carac(DROIT,numero_lettre('V'));
				afficher_carac(DROIT,numero_lettre('5'));

				ecran_disponible(DROIT);
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 54;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;
				afficher_carac(DROIT,numero_lettre('V'));

				glcd_write_nombre(DROIT ,5,base_donnees[5].tension_pinf_moyenne);

				ecran_disponible(DROIT);
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 6;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;

				ecran_disponible(DROIT);

				ecran_disponible(DROIT);
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 0;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;

				afficher_carac(DROIT,numero_lettre('V'));
				afficher_carac(DROIT,numero_lettre('6'));

				ecran_disponible(DROIT);
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 54;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;
				afficher_carac(DROIT,numero_lettre('V'));

				glcd_write_nombre(DROIT ,6,base_donnees[6].tension_pinf_moyenne);

				ecran_disponible(DROIT);
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 7;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;

				ecran_disponible(DROIT);
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 0;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;

				afficher_carac(DROIT,numero_lettre('V'));
				afficher_carac(DROIT,numero_lettre('7'));

				ecran_disponible(DROIT);
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = COLUMN | 54;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = (WRITE_CDE_E	| DROIT);
				CTRL = (WRITE_CDE 	| DROIT);
				CTRL = IDLE;
				afficher_carac(DROIT,numero_lettre('V'));

				glcd_write_nombre(DROIT ,7,base_donnees[7].tension_pinf_moyenne);

				//}
		}
		_delay_ms(200);
	}while(1);
	return 0;
}
