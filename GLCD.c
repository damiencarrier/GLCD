/*
 * GLCD.c
 *
 *  Created on: 21 nov. 2019
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

#ifndef GLCD_H_
#define GLCD_H_

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

#define LEFT 0x08
#define RIGHT 0x04

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

//initialiser le LCD
//mettre CTRL_DIR en sortie
//mettre CTRL a RESET
//mettre DATA_DIR en sortie
//mettre DATA à 0xFF
//quitter le mode RESET  en mettant CTRL = IDLE
//
//tester que GLCD gauche est présent
//mettre DATA_DIR en entrée (0x00)
//mettre CTRL = READ_STATUS | LEFT
//mettre CTRL = READ-STATUS_E | LEFT
//mettre CTRL = READ_STATUS | LEFT
//lire le port de donnée du GLCD et mettre le résultat dans status_left
//status_left = DATAR ce qui signifie: lire le bus de donnée et le sauvegarder en status_left
//mettre CTRL_CTRL en IDLE car on a fini le cycle et on va traiter status_left
//vérifier que les 4 LSB des status_left sont a 0. Si ce n'est pas le cas, pas de glcd qui répond.
//debug: on écrit status_left sur le portb et on boucle sur un NOP while(1);
//
//si les 4 LSB sont à 1, tester le reset en vérifiant que le bit 4 de status_left est à 0
//si ce n'est pas le cas, faire une boucle do while pour faire du pooling sur ce bit
//
//quand la phase de reset est terminée, faire de meme pour le coté droit
//
//quand les 2 NT7108C ont terminé la phase de reset, on les met on
//mettre DATA_DIR en sortie
//CTRL=WRITE_CDE | LEFT
//DATA=DISPLAY_ON
//CTRL=WRITE_CDE_E |LEFT
//CTRL=WRITE_CDE |LEFT
//CTRL=IDLE
//tester le bit display_on_off (bit 5) a 0 en réalisant a nouveau une lecture du status
//poller sur la lecture jusqu'a obtention du display ON et du bit busy à 0
//a ce stade on attend un status a 0x00. Si c'est le cas, on peu travailler
//écrire 0x55, puis 0xAA, puis 0x00 puis 0xFF dans la ram de gauche pouis dans la ram de droite

#endif /* GLCD_H_ */

// Ces deux variables recevront le registre de status des deux 1/2 ecrans.
unsigned char status_left , status_right;

//ecriture du N° de colonne dans la colonne, a gauche, puis l'inverse a droite
unsigned char no_colonne, no_page;

unsigned char ligne,decalage,ligne_carac;

//variable permettant de faire des mesures de tension
unsigned long tension_pinF0,tension_pinf1,tension_pinf2,tension_pinf3,tension_pinf4;
unsigned long tension_pinf5,tension_pinf6,tension_pinf7;
unsigned long puissance_E,puissance_A,puissance_entree_E,puissance_entree_A,rendement_E,rendement_A;

//tableaux ou sont stocké en décimal les valeurs à afficher
static char tension_string1[] = {'0','0','0','0','0'};
static char tension_string2[] = {'0','0','0','0','0'};
static char tension_string3[] = {'0','0','0','0','0'};
static char tension_string4[] = {'0','0','0','0','0'};
static char tension_string5[] = {'0','0','0','0','0'};
static char tension_string6[] = {'0','0','0','0','0'};
static char tension_string7[] = {'0','0','0','0','0'};
static char puissance_E_string[] = {'0','0','0','0','0'};
static char puissance_A_string[] = {'0','0','0','0','0'};

//ne sert a rien, mais fait une erreur sinon WTF
static char puissance_entree_E_string[] = {'1','2','3','4','5','6'};

static char puissance_entree_F_string[] = {'0','0','0','0','0'};

static char puissance_entree_A_string[] = {'0','0','0','0','0'};
static char rendement_A_string[] = {'0','0','0','0','0'};

//ne sert a rien, mais fait une erreur sinon WTF
static char rendement_E_string[] = {'0','0','0','0','0'};

static char rendement_F_string[] = {'0','0','0','0','0'};

//met en noir tout l'écran gauche
void ecran_noir_gauche(){
	//parcourt les 8 ligne a mettre en noir
	for (no_page = 0; no_page < 8; no_page ++) {
		// Quand on a rempli une page (64 ecritures), on change de page
		ecran_disponible_gauche();
		// on met sur DATA la commande PAGE + le N° de colonne
		DATAW = PAGE | no_page;
		// et on ecrit la commande
		CTRL = (WRITE_CDE 	| LEFT);
		CTRL = (WRITE_CDE_E	| LEFT);
		CTRL = (WRITE_CDE 	| LEFT);
		CTRL = IDLE;

		//met en noir toute une ligne
		for (no_colonne = 0; no_colonne < 64; no_colonne ++) {
			ecran_disponible_gauche();
			// on met sur DATA le N° de colonne
			DATAW = 0x00;
			CTRL = (WRITE_RAM 	| LEFT);
			CTRL = (WRITE_RAM_E	| LEFT);
			CTRL = (WRITE_RAM 	| LEFT);
			CTRL = IDLE;
			}
		}
}
//met en noir tout l'écran droit
void ecran_noir_droit(){
	//parcourt les 8 ligne a mettre en noir
	for (no_page = 0; no_page < 8; no_page ++) {
		// Quand on a rempli une page (64 ecritures), on change de page
		ecran_disponible_droit();
		// on met sur DATA la commande PAGE + le N° de colonne
		DATAW = PAGE | no_page;
		// et on ecrit la commande
		CTRL = (WRITE_CDE 	| RIGHT);
		CTRL = (WRITE_CDE_E	| RIGHT);
		CTRL = (WRITE_CDE 	| RIGHT);
		CTRL = IDLE;
		//met en noir toute une ligne
		for (no_colonne = 0; no_colonne < 64; no_colonne ++) {
			ecran_disponible_droit();
			// on met sur DATA le N° de colonne
			DATAW = 0x00;
			CTRL = (WRITE_RAM 	| RIGHT);
			CTRL = (WRITE_RAM_E	| RIGHT);
			CTRL = (WRITE_RAM 	| RIGHT);
			CTRL = IDLE;
			}
		}
}

void ecran_blanc_gauche(){
	for (no_page = 0; no_page < 8; no_page ++) {
		// Quand on a rempli une page (64 ecritures), on change de page
		ecran_disponible_gauche();
		// on met sur DATA la commande PAGE + le N° de colonne
		DATAW = PAGE | no_page;
		// et on ecrit la commande
		CTRL = (WRITE_CDE 	| LEFT);
		CTRL = (WRITE_CDE_E	| LEFT);
		CTRL = (WRITE_CDE 	| LEFT);
		CTRL = IDLE;
		for (no_colonne = 0; no_colonne < 64; no_colonne ++) {
			ecran_disponible_gauche();
			// on met sur DATA le N° de colonne
			DATAW = 0xFF;
			CTRL = (WRITE_RAM 	| LEFT);
			CTRL = (WRITE_RAM_E	| LEFT);
			CTRL = (WRITE_RAM 	| LEFT);
			CTRL = IDLE;
			}
		}
}
void ecran_blanc_droit(){
	for (no_page = 0; no_page < 8; no_page ++) {
		// Quand on a rempli une page (64 ecritures), on change de page
		ecran_disponible_droit();
		// on met sur DATA la commande PAGE + le N° de colonne
		DATAW = PAGE | no_page;
		// et on ecrit la commande
		CTRL = (WRITE_CDE 	| RIGHT);
		CTRL = (WRITE_CDE_E	| RIGHT);
		CTRL = (WRITE_CDE 	| RIGHT);
		CTRL = IDLE;
		for (no_colonne = 0; no_colonne < 64; no_colonne ++) {
			ecran_disponible_droit();
			// on met sur DATA le N° de colonne
			DATAW = 0xFF;
			CTRL = (WRITE_RAM 	| RIGHT);
			CTRL = (WRITE_RAM_E	| RIGHT);
			CTRL = (WRITE_RAM 	| RIGHT);
			CTRL = IDLE;
			}
		}
}

void initialiser_ecran(){
	// initialisation du GLCD
		// Les ports de contrôle et de donnée sont positionnés en sortie.
		CTRL_DIR = 0xFF; // en sortie
		DATA_DIR = 0xFF;
		// activation du RESET
		CTRL = RESET;
		// des-activation du RESET
		CTRL = IDLE;

		// Test de présence des deux NT7108C. Vérifier que les pullups sont actifs sur les deux ports A et C
		// mise en entrée du port de donnée
		DATA_DIR = 0x00;
		// lecture du registre de status GAUCHE et verification que les 4 bits de poids faible sont a 0, que le bit Reset est à 0
		// et que le bit busy est lui aussi à 0. Seul le bit ON/OFF doit être à 1. On fait un ET entre la valeur du status et la valeur
		// EF (bit 5 à 0) et si le resultat est VRAI (/= 0), on boucle
		do {
			CTRL = (READ_STATUS 	| LEFT);
			CTRL = (READ_STATUS_E	| LEFT);
			NOP;
			CTRL = (READ_STATUS 	| LEFT);
			CTRL = (READ_STATUS_E	| LEFT);
			NOP;
			status_left = DATAR;
			CTRL = IDLE;
		} while (status_left & 0xDF);

		// lecture du registre de status DROITE et verification que les 4 bits de poids faible sont a 0
		do {
			CTRL = (READ_STATUS 	| RIGHT);
			CTRL = (READ_STATUS_E	| RIGHT);
			NOP;
			CTRL = (READ_STATUS 	| RIGHT);
			CTRL = (READ_STATUS_E	| RIGHT);
			NOP;
			status_right = DATAR;
			CTRL = IDLE;
		} while (status_right & 0xDF);


		// Quand les 2 NT7108C ont termine la phase de reset, on les met ON
		// remettre DATA_DIR en sortie
		DATA_DIR = 0xFF;
		// mettre la valeur DIPLAY_ON sur le bus de DATA
		DATAW = DISPLAY_ON;
		// ecrire la commande Display_ON sur l'ecran gauche
		CTRL = (WRITE_CDE 	| LEFT);
		CTRL = (WRITE_CDE_E	| LEFT);
		CTRL = (WRITE_CDE 	| LEFT);
		CTRL = IDLE;




		// ecrire la commande Display_ON sur l'ecran droite
		CTRL = (WRITE_CDE 	| RIGHT);
		CTRL = (WRITE_CDE_E	| RIGHT);
		CTRL = (WRITE_CDE 	| RIGHT);
		CTRL = IDLE;

		//verifier que les deux côtés sont ON (Tous les bits du registre de status à 0)

		// lecture du registre de status GAUCHE et verification que les 8 bits sont a 0
		// La lecture se fait en deux temps: Un premier front descendant sur E permet
		// de raffraichir le registre de données en sortie, puis on remet E a 1 pour
		// ouvrir le buffer tri-state et lire la donnée sur le bus.
}

void ecran_disponible_gauche(){
	// on commence par verifier que le bit busy est a 0
	// mise en entree du bus de DATA
	DATA_DIR = 0x00;
	do {
		CTRL = (READ_STATUS 	| LEFT);
		CTRL = (READ_STATUS_E	| LEFT);
		NOP;
		CTRL = (READ_STATUS 	| LEFT);
		CTRL = (READ_STATUS_E	| LEFT);
		NOP;
		status_left = DATAR;
		CTRL = IDLE;
	} while (status_left);
	DATA_DIR = 0xFF;
}
void ecran_disponible_droit(){
	// on commence par verifier que le bit busy est a 0
	// mise en entree du bus de DATA
	DATA_DIR = 0x00;
	do {
		CTRL = (READ_STATUS 	| RIGHT);
		CTRL = (READ_STATUS_E	| RIGHT);
		NOP;
		CTRL = (READ_STATUS 	| RIGHT);
		CTRL = (READ_STATUS_E	| RIGHT);
		NOP;
		status_right = DATAR;
		CTRL = IDLE;
	} while (status_right);
	DATA_DIR = 0xFF;
}

void afficher_carac_gauche(unsigned int ligne_font){
	for(ligne_carac=0;ligne_carac<5;ligne_carac++){
		//_delay_ms(1);
		ecran_disponible_gauche();
		// on remet le bus DATA en sortie
		DATA_DIR = 0xFF;
		// on met sur DATA le N° de colonne
		DATAW = Font5x7[ligne_font+ligne_carac];
		CTRL = (WRITE_RAM 	| LEFT);
		CTRL = (WRITE_RAM_E	| LEFT);
		CTRL = (WRITE_RAM 	| LEFT);
		CTRL = IDLE;
	}
		//_delay_ms(1);
		ecran_disponible_gauche();
		// on remet le bus DATA en sortie
		DATA_DIR = 0xFF;
		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| LEFT);
		CTRL = (WRITE_RAM_E	| LEFT);
		CTRL = (WRITE_RAM 	| LEFT);
		CTRL = IDLE;

}
void afficher_carac_droit(unsigned int ligne_font){
	for(ligne_carac=0;ligne_carac<5;ligne_carac++){
		//_delay_ms(1);
		ecran_disponible_droit();
		// on remet le bus DATA en sortie
		DATA_DIR = 0xFF;
		// on met sur DATA le N° de colonne
		DATAW = Font5x7[ligne_font+ligne_carac];
		CTRL = (WRITE_RAM 	| RIGHT);
		CTRL = (WRITE_RAM_E	| RIGHT);
		CTRL = (WRITE_RAM 	| RIGHT);
		CTRL = IDLE;
	}
		//_delay_ms(1);
		ecran_disponible_droit();
		// on remet le bus DATA en sortie
		DATA_DIR = 0xFF;
		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| RIGHT);
		CTRL = (WRITE_RAM_E	| RIGHT);
		CTRL = (WRITE_RAM 	| RIGHT);
		CTRL = IDLE;
}

unsigned int numero_lettre(unsigned char lettre){
	return((lettre*5)-(32*5));
	//0> 80
	//A>165
	//a>345
}

void mesure_tension(){
	//mesure la tension en millivolt

	ADMUX = 0b010000001;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC);
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC;
	tension_pinf1 = tension_pinF0 *5*3;
	dtostrf(tension_pinf1,5,0,tension_string1);

	ADMUX = 0b010000010;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC);
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC;
	tension_pinf2 = tension_pinF0 *5*2;
	dtostrf(tension_pinf2,5,0,tension_string2);



	ADMUX = 0b010000011;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC);
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC;
	tension_pinf3 = tension_pinF0 *5*5;
	dtostrf(tension_pinf3,5,0,tension_string3);

	ADMUX = 0b010000100;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC);
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC;
	tension_pinf4 = tension_pinF0 *5;
	dtostrf(tension_pinf4,5,0,tension_string4);

	ADMUX = 0b010000101;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC);
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC;
	tension_pinf5 = tension_pinF0 *5;
	dtostrf(tension_pinf5,5,0,tension_string5);

	ADMUX = 0b010000110;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC);
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC;
	tension_pinf6 = tension_pinF0 *5;
	dtostrf(tension_pinf6,5,0,tension_string6);

	ADMUX = 0b010000111;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC);
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC;
	tension_pinf7 = tension_pinF0 *5;
	dtostrf(tension_pinf7,5,0,tension_string7);

	puissance_entree_E = (tension_pinf1/10) * (tension_pinf2/100);
	puissance_entree_A = (tension_pinf1/10) * (tension_pinf7/100);
	puissance_E = (tension_pinf3/10) * (tension_pinf4/100);
	puissance_A = (tension_pinf5/10) * (tension_pinf6/100);
	if (puissance_E<puissance_entree_E){
		rendement_E = (puissance_E*1000)/ (puissance_entree_E/100);
	}else rendement_E = 99999;
	if (puissance_A<puissance_entree_A){
		rendement_A = (puissance_A*1000)/ (puissance_entree_A/100);
	}else rendement_A = 99999;

	if (puissance_entree_E>100000){
		puissance_entree_E = 99999;
		rendement_E =0;
	}
	if (puissance_entree_A>100000){
		puissance_entree_A = 99999;
		rendement_A =0;
	}
	if (puissance_E>100000){
		puissance_E = 99999;
		rendement_E =0;
	}
	if (puissance_A>100000){
		puissance_A = 99999;
		rendement_A =0;
	}
	dtostrf(puissance_entree_E,5,0,puissance_entree_F_string);
	dtostrf(puissance_entree_A,5,0,puissance_entree_A_string);
	dtostrf(puissance_E,6,0,puissance_E_string);
	dtostrf(puissance_A,5,0,puissance_A_string);
	dtostrf(rendement_E,5,0,rendement_F_string);
	dtostrf(rendement_A,5,0,rendement_A_string);
}

void glcd_write_nombre(GAUCHE_DROIT cote, u8 ligne, u32 nombre){

}

void GLCD_Code(){
	initialiser_ecran();


		ecran_blanc_gauche();
		_delay_ms(100);
		ecran_blanc_droit();
		_delay_ms(100);
		ecran_noir_gauche();
		_delay_ms(100);
		ecran_noir_droit();
		_delay_ms(100);

		do {

			mesure_tension();

			ecran_disponible_gauche();

			// on remet le bus DATA en sortie
			DATA_DIR = 0xFF;
			// on met sur DATA la commande PAGE + le N° de colonne
			DATAW = COLUMN | 0;
			// et on ecrit la commande
			CTRL = (WRITE_CDE 	| LEFT);
			CTRL = (WRITE_CDE_E	| LEFT);
			CTRL = (WRITE_CDE 	| LEFT);
			CTRL = IDLE;

			ecran_disponible_droit();

			// on remet le bus DATA en sortie
			DATA_DIR = 0xFF;
			// on met sur DATA la commande PAGE + le N° de colonne
			DATAW = COLUMN | 0;
			// et on ecrit la commande
			CTRL = (WRITE_CDE 	| RIGHT);
			CTRL = (WRITE_CDE_E	| RIGHT);
			CTRL = (WRITE_CDE 	| RIGHT);
			CTRL = IDLE;

			//for (no_page = 0; no_page < 8; no_page ++) {
				ecran_disponible_gauche();

				// on remet le bus DATA en sortie
				DATA_DIR = 0xFF;
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 0;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| LEFT);
				CTRL = (WRITE_CDE_E	| LEFT);
				CTRL = (WRITE_CDE 	| LEFT);
				CTRL = IDLE;

				ecran_disponible_droit();

				// on remet le bus DATA en sortie
				DATA_DIR = 0xFF;
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 0;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| RIGHT);
				CTRL = (WRITE_CDE_E	| RIGHT);
				CTRL = (WRITE_CDE 	| RIGHT);
				CTRL = IDLE;


	//			for(ligne = 0; ligne <8; ligne ++){
	//				decalage=0b11111111;
	//				decalage=decalage <<1;

					afficher_carac_gauche(numero_lettre(' '));
					afficher_carac_gauche(numero_lettre('B'));
					afficher_carac_gauche(numero_lettre('a'));
					afficher_carac_gauche(numero_lettre('t'));
					afficher_carac_gauche(numero_lettre('t'));
					afficher_carac_gauche(numero_lettre('e'));
					afficher_carac_gauche(numero_lettre('r'));
					afficher_carac_gauche(numero_lettre('i'));
					afficher_carac_gauche(numero_lettre('e'));
					afficher_carac_gauche(numero_lettre(' '));



				for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
					//_delay_ms(1);
					ecran_disponible_gauche();
					// on remet le bus DATA en sortie
					DATA_DIR = 0xFF;
					// on met sur DATA le N° de colonne
					DATAW = 0b00000000;
					CTRL = (WRITE_RAM 	| LEFT);
					CTRL = (WRITE_RAM_E	| LEFT);
					CTRL = (WRITE_RAM 	| LEFT);
					CTRL = IDLE;

				}

				afficher_carac_droit(numero_lettre(' '));
				afficher_carac_droit(numero_lettre('E'));
				afficher_carac_droit(numero_lettre('l'));
				afficher_carac_droit(numero_lettre('e'));
				afficher_carac_droit(numero_lettre('v'));
				afficher_carac_droit(numero_lettre('a'));
				afficher_carac_droit(numero_lettre('t'));
				afficher_carac_droit(numero_lettre('e'));
				afficher_carac_droit(numero_lettre('u'));
				afficher_carac_droit(numero_lettre('r'));

				for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
					//_delay_ms(1);
					ecran_disponible_droit();
					// on remet le bus DATA en sortie
					DATA_DIR = 0xFF;
					// on met sur DATA le N° de colonne
					DATAW = 0b00000000;
					CTRL = (WRITE_RAM 	| RIGHT);
					CTRL = (WRITE_RAM_E	| RIGHT);
					CTRL = (WRITE_RAM 	| RIGHT);
					CTRL = IDLE;

				}









				ecran_disponible_gauche();

				// on remet le bus DATA en sortie
				DATA_DIR = 0xFF;
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 1;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| LEFT);
				CTRL = (WRITE_CDE_E	| LEFT);
				CTRL = (WRITE_CDE 	| LEFT);
				CTRL = IDLE;

				ecran_disponible_droit();

				// on remet le bus DATA en sortie
				DATA_DIR = 0xFF;
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 1;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| RIGHT);
				CTRL = (WRITE_CDE_E	| RIGHT);
				CTRL = (WRITE_CDE 	| RIGHT);
				CTRL = IDLE;


	//			for(ligne = 0; ligne <8; ligne ++){
	//				decalage=0b11111111;
	//				decalage=decalage <<1;

					//tension V batterie
					afficher_carac_gauche(numero_lettre('V'));
					afficher_carac_gauche(numero_lettre(' '));
					afficher_carac_gauche(numero_lettre(':'));
					afficher_carac_gauche(numero_lettre(tension_string1[0]));
					afficher_carac_gauche(numero_lettre(tension_string1[1]));
					afficher_carac_gauche(numero_lettre(','));
					afficher_carac_gauche(numero_lettre(tension_string1[2]));
					afficher_carac_gauche(numero_lettre(tension_string1[3]));
					afficher_carac_gauche(numero_lettre(tension_string1[4]));
					afficher_carac_gauche(numero_lettre('V'));



				for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
					//_delay_ms(1);
					ecran_disponible_gauche();
					// on remet le bus DATA en sortie
					DATA_DIR = 0xFF;
					// on met sur DATA le N° de colonne
					DATAW = 0b00000000;
					CTRL = (WRITE_RAM 	| LEFT);
					CTRL = (WRITE_RAM_E	| LEFT);
					CTRL = (WRITE_RAM 	| LEFT);
					CTRL = IDLE;

				}

				// tension élévateur
				afficher_carac_droit(numero_lettre(' '));
				afficher_carac_droit(numero_lettre('v'));
				afficher_carac_droit(numero_lettre(':'));
				afficher_carac_droit(numero_lettre(tension_string3[0]));
				afficher_carac_droit(numero_lettre(tension_string3[1]));
				afficher_carac_droit(numero_lettre(','));
				afficher_carac_droit(numero_lettre(tension_string3[2]));
				afficher_carac_droit(numero_lettre(tension_string3[3]));
				afficher_carac_droit(numero_lettre(tension_string3[4]));
				afficher_carac_droit(numero_lettre('V'));


				for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
					//_delay_ms(1);
					ecran_disponible_droit();
					// on remet le bus DATA en sortie
					DATA_DIR = 0xFF;
					// on met sur DATA le N° de colonne
					DATAW = 0b00000000;
					CTRL = (WRITE_RAM 	| RIGHT);
					CTRL = (WRITE_RAM_E	| RIGHT);
					CTRL = (WRITE_RAM 	| RIGHT);
					CTRL = IDLE;

				}




				ecran_disponible_gauche();

				// on remet le bus DATA en sortie
				DATA_DIR = 0xFF;
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 2;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| LEFT);
				CTRL = (WRITE_CDE_E	| LEFT);
				CTRL = (WRITE_CDE 	| LEFT);
				CTRL = IDLE;

				ecran_disponible_droit();

				// on remet le bus DATA en sortie
				DATA_DIR = 0xFF;
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 2;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| RIGHT);
				CTRL = (WRITE_CDE_E	| RIGHT);
				CTRL = (WRITE_CDE 	| RIGHT);
				CTRL = IDLE;


	//			for(ligne = 0; ligne <8; ligne ++){
	//				decalage=0b11111111;
	//				decalage=decalage <<1;

					//IE courant entrée
					afficher_carac_gauche(numero_lettre('I'));
					afficher_carac_gauche(numero_lettre('E'));
					afficher_carac_gauche(numero_lettre(':'));
					afficher_carac_gauche(numero_lettre(tension_string2[0]));
					afficher_carac_gauche(numero_lettre(tension_string2[1]));
					afficher_carac_gauche(numero_lettre(','));
					afficher_carac_gauche(numero_lettre(tension_string2[2]));
					afficher_carac_gauche(numero_lettre(tension_string2[3]));
					afficher_carac_gauche(numero_lettre(tension_string2[4]));
					afficher_carac_gauche(numero_lettre('A'));



				for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
					//_delay_ms(1);
					ecran_disponible_gauche();
					// on remet le bus DATA en sortie
					DATA_DIR = 0xFF;
					// on met sur DATA le N° de colonne
					DATAW = 0b00000000;
					CTRL = (WRITE_RAM 	| LEFT);
					CTRL = (WRITE_RAM_E	| LEFT);
					CTRL = (WRITE_RAM 	| LEFT);
					CTRL = IDLE;

				}

				// courant elevateur
				afficher_carac_droit(numero_lettre(' '));
				afficher_carac_droit(numero_lettre('i'));
				afficher_carac_droit(numero_lettre(':'));
				afficher_carac_droit(numero_lettre(tension_string4[0]));
				afficher_carac_droit(numero_lettre(tension_string4[1]));
				afficher_carac_droit(numero_lettre(','));
				afficher_carac_droit(numero_lettre(tension_string4[2]));
				afficher_carac_droit(numero_lettre(tension_string4[3]));
				afficher_carac_droit(numero_lettre(tension_string4[4]));
				afficher_carac_droit(numero_lettre('A'));

				for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
					//_delay_ms(1);
					ecran_disponible_droit();
					// on remet le bus DATA en sortie
					DATA_DIR = 0xFF;
					// on met sur DATA le N° de colonne
					DATAW = 0b00000000;
					CTRL = (WRITE_RAM 	| RIGHT);
					CTRL = (WRITE_RAM_E	| RIGHT);
					CTRL = (WRITE_RAM 	| RIGHT);
					CTRL = IDLE;

				}





				ecran_disponible_gauche();

				// on remet le bus DATA en sortie
				DATA_DIR = 0xFF;
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 3;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| LEFT);
				CTRL = (WRITE_CDE_E	| LEFT);
				CTRL = (WRITE_CDE 	| LEFT);
				CTRL = IDLE;

				ecran_disponible_droit();

				// on remet le bus DATA en sortie
				DATA_DIR = 0xFF;
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 3;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| RIGHT);
				CTRL = (WRITE_CDE_E	| RIGHT);
				CTRL = (WRITE_CDE 	| RIGHT);
				CTRL = IDLE;


	//			for(ligne = 0; ligne <8; ligne ++){
	//				decalage=0b11111111;
	//				decalage=decalage <<1;

					//puissance entrée élévateur
					afficher_carac_gauche(numero_lettre('P'));
					afficher_carac_gauche(numero_lettre('E'));
					afficher_carac_gauche(numero_lettre(':'));
					afficher_carac_gauche(numero_lettre(puissance_entree_F_string[0]));
					afficher_carac_gauche(numero_lettre(puissance_entree_F_string[1]));
					afficher_carac_gauche(numero_lettre(','));
					afficher_carac_gauche(numero_lettre(puissance_entree_F_string[2]));
					afficher_carac_gauche(numero_lettre(puissance_entree_F_string[3]));
					afficher_carac_gauche(numero_lettre(puissance_entree_F_string[4]));
					afficher_carac_gauche(numero_lettre('W'));



				for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
					//_delay_ms(1);
					ecran_disponible_gauche();
					// on remet le bus DATA en sortie
					DATA_DIR = 0xFF;
					// on met sur DATA le N° de colonne
					DATAW = 0b00000000;
					CTRL = (WRITE_RAM 	| LEFT);
					CTRL = (WRITE_RAM_E	| LEFT);
					CTRL = (WRITE_RAM 	| LEFT);
					CTRL = IDLE;

				}

				//puissance élévateur
				afficher_carac_droit(numero_lettre(' '));
				afficher_carac_droit(numero_lettre('P'));
				afficher_carac_droit(numero_lettre(':'));
				afficher_carac_droit(numero_lettre(puissance_E_string[0]));
				afficher_carac_droit(numero_lettre(puissance_E_string[1]));
				afficher_carac_droit(numero_lettre(','));
				afficher_carac_droit(numero_lettre(puissance_E_string[2]));
				afficher_carac_droit(numero_lettre(puissance_E_string[3]));
				afficher_carac_droit(numero_lettre(puissance_E_string[4]));
				afficher_carac_droit(numero_lettre('W'));

				for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
					//_delay_ms(1);
					ecran_disponible_droit();
					// on remet le bus DATA en sortie
					DATA_DIR = 0xFF;
					// on met sur DATA le N° de colonne
					DATAW = 0b00000000;
					CTRL = (WRITE_RAM 	| RIGHT);
					CTRL = (WRITE_RAM_E	| RIGHT);
					CTRL = (WRITE_RAM 	| RIGHT);
					CTRL = IDLE;

				}




				ecran_disponible_gauche();

				// on remet le bus DATA en sortie
				DATA_DIR = 0xFF;
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 4;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| LEFT);
				CTRL = (WRITE_CDE_E	| LEFT);
				CTRL = (WRITE_CDE 	| LEFT);
				CTRL = IDLE;

				ecran_disponible_droit();

				// on remet le bus DATA en sortie
				DATA_DIR = 0xFF;
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 4;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| RIGHT);
				CTRL = (WRITE_CDE_E	| RIGHT);
				CTRL = (WRITE_CDE 	| RIGHT);
				CTRL = IDLE;


	//			for(ligne = 0; ligne <8; ligne ++){
	//				decalage=0b11111111;
	//				decalage=decalage <<1;

					//rendement élévateur
					afficher_carac_gauche(numero_lettre('N'));
					afficher_carac_gauche(numero_lettre('E'));
					afficher_carac_gauche(numero_lettre(':'));
					afficher_carac_gauche(numero_lettre(rendement_F_string[0]));
					afficher_carac_gauche(numero_lettre(rendement_F_string[1]));
					afficher_carac_gauche(numero_lettre(','));
					afficher_carac_gauche(numero_lettre(rendement_F_string[2]));
					afficher_carac_gauche(numero_lettre(rendement_F_string[3]));
					afficher_carac_gauche(numero_lettre(rendement_F_string[4]));
					afficher_carac_gauche(numero_lettre('%'));



				for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
					//_delay_ms(1);
					ecran_disponible_gauche();
					// on remet le bus DATA en sortie
					DATA_DIR = 0xFF;
					// on met sur DATA le N° de colonne
					DATAW = 0b00000000;
					CTRL = (WRITE_RAM 	| LEFT);
					CTRL = (WRITE_RAM_E	| LEFT);
					CTRL = (WRITE_RAM 	| LEFT);
					CTRL = IDLE;

				}

				afficher_carac_droit(numero_lettre(' '));
				afficher_carac_droit(numero_lettre('A'));
				afficher_carac_droit(numero_lettre('b'));
				afficher_carac_droit(numero_lettre('a'));
				afficher_carac_droit(numero_lettre('i'));
				afficher_carac_droit(numero_lettre('s'));
				afficher_carac_droit(numero_lettre('s'));
				afficher_carac_droit(numero_lettre('e'));
				afficher_carac_droit(numero_lettre('u'));
				afficher_carac_droit(numero_lettre('r'));

				for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
					//_delay_ms(1);
					ecran_disponible_droit();
					// on remet le bus DATA en sortie
					DATA_DIR = 0xFF;
					// on met sur DATA le N° de colonne
					DATAW = 0b00000000;
					CTRL = (WRITE_RAM 	| RIGHT);
					CTRL = (WRITE_RAM_E	| RIGHT);
					CTRL = (WRITE_RAM 	| RIGHT);
					CTRL = IDLE;

				}







				ecran_disponible_gauche();

				// on remet le bus DATA en sortie
				DATA_DIR = 0xFF;
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 5;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| LEFT);
				CTRL = (WRITE_CDE_E	| LEFT);
				CTRL = (WRITE_CDE 	| LEFT);
				CTRL = IDLE;

				ecran_disponible_droit();

				// on remet le bus DATA en sortie
				DATA_DIR = 0xFF;
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 5;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| RIGHT);
				CTRL = (WRITE_CDE_E	| RIGHT);
				CTRL = (WRITE_CDE 	| RIGHT);
				CTRL = IDLE;


	//			for(ligne = 0; ligne <8; ligne ++){
	//				decalage=0b11111111;
	//				decalage=decalage <<1;

					//rendement abaisseur
					afficher_carac_gauche(numero_lettre('N'));
					afficher_carac_gauche(numero_lettre('A'));
					afficher_carac_gauche(numero_lettre(':'));
					afficher_carac_gauche(numero_lettre(rendement_A_string[0]));
					afficher_carac_gauche(numero_lettre(rendement_A_string[1]));
					afficher_carac_gauche(numero_lettre(','));
					afficher_carac_gauche(numero_lettre(rendement_A_string[2]));
					afficher_carac_gauche(numero_lettre(rendement_A_string[3]));
					afficher_carac_gauche(numero_lettre(rendement_A_string[4]));
					afficher_carac_gauche(numero_lettre('%'));



				for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
					//_delay_ms(1);
					ecran_disponible_gauche();
					// on remet le bus DATA en sortie
					DATA_DIR = 0xFF;
					// on met sur DATA le N° de colonne
					DATAW = 0b00000000;
					CTRL = (WRITE_RAM 	| LEFT);
					CTRL = (WRITE_RAM_E	| LEFT);
					CTRL = (WRITE_RAM 	| LEFT);
					CTRL = IDLE;

				}

				//tension abaisseur
				afficher_carac_droit(numero_lettre(' '));
				afficher_carac_droit(numero_lettre('V'));
				afficher_carac_droit(numero_lettre(':'));
				afficher_carac_droit(numero_lettre(tension_string5[0]));
				afficher_carac_droit(numero_lettre(tension_string5[1]));
				afficher_carac_droit(numero_lettre(','));
				afficher_carac_droit(numero_lettre(tension_string5[2]));
				afficher_carac_droit(numero_lettre(tension_string5[3]));
				afficher_carac_droit(numero_lettre(tension_string5[4]));
				afficher_carac_droit(numero_lettre('V'));

				for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
					//_delay_ms(1);
					ecran_disponible_droit();
					// on remet le bus DATA en sortie
					DATA_DIR = 0xFF;
					// on met sur DATA le N° de colonne
					DATAW = 0b00000000;
					CTRL = (WRITE_RAM 	| RIGHT);
					CTRL = (WRITE_RAM_E	| RIGHT);
					CTRL = (WRITE_RAM 	| RIGHT);
					CTRL = IDLE;

				}








				ecran_disponible_gauche();

				// on remet le bus DATA en sortie
				DATA_DIR = 0xFF;
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 6;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| LEFT);
				CTRL = (WRITE_CDE_E	| LEFT);
				CTRL = (WRITE_CDE 	| LEFT);
				CTRL = IDLE;

				ecran_disponible_droit();

				// on remet le bus DATA en sortie
				DATA_DIR = 0xFF;
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 6;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| RIGHT);
				CTRL = (WRITE_CDE_E	| RIGHT);
				CTRL = (WRITE_CDE 	| RIGHT);
				CTRL = IDLE;


	//			for(ligne = 0; ligne <8; ligne ++){
	//				decalage=0b11111111;
	//				decalage=decalage <<1;

					//courant entrée abaisseur
					afficher_carac_gauche(numero_lettre('I'));
					afficher_carac_gauche(numero_lettre('A'));
					afficher_carac_gauche(numero_lettre(':'));
					afficher_carac_gauche(numero_lettre(tension_string7[0]));
					afficher_carac_gauche(numero_lettre(tension_string7[1]));
					afficher_carac_gauche(numero_lettre(','));
					afficher_carac_gauche(numero_lettre(tension_string7[2]));
					afficher_carac_gauche(numero_lettre(tension_string7[3]));
					afficher_carac_gauche(numero_lettre(tension_string7[4]));
					afficher_carac_gauche(numero_lettre('A'));



				for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
					//_delay_ms(1);
					ecran_disponible_gauche();
					// on remet le bus DATA en sortie
					DATA_DIR = 0xFF;
					// on met sur DATA le N° de colonne
					DATAW = 0b00000000;
					CTRL = (WRITE_RAM 	| LEFT);
					CTRL = (WRITE_RAM_E	| LEFT);
					CTRL = (WRITE_RAM 	| LEFT);
					CTRL = IDLE;

				}

				//courant abaisseur
				afficher_carac_droit(numero_lettre(' '));
				afficher_carac_droit(numero_lettre('i'));
				afficher_carac_droit(numero_lettre(':'));
				afficher_carac_droit(numero_lettre(tension_string6[0]));
				afficher_carac_droit(numero_lettre(tension_string6[1]));
				afficher_carac_droit(numero_lettre(','));
				afficher_carac_droit(numero_lettre(tension_string6[2]));
				afficher_carac_droit(numero_lettre(tension_string6[3]));
				afficher_carac_droit(numero_lettre(tension_string6[4]));
				afficher_carac_droit(numero_lettre('A'));

				for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
					//_delay_ms(1);
					ecran_disponible_droit();
					// on remet le bus DATA en sortie
					DATA_DIR = 0xFF;
					// on met sur DATA le N° de colonne
					DATAW = 0b00000000;
					CTRL = (WRITE_RAM 	| RIGHT);
					CTRL = (WRITE_RAM_E	| RIGHT);
					CTRL = (WRITE_RAM 	| RIGHT);
					CTRL = IDLE;

				}





				ecran_disponible_gauche();

				// on remet le bus DATA en sortie
				DATA_DIR = 0xFF;
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 7;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| LEFT);
				CTRL = (WRITE_CDE_E	| LEFT);
				CTRL = (WRITE_CDE 	| LEFT);
				CTRL = IDLE;

				ecran_disponible_droit();

				// on remet le bus DATA en sortie
				DATA_DIR = 0xFF;
				// on met sur DATA la commande PAGE + le N° de colonne
				DATAW = PAGE | 7;
				// et on ecrit la commande
				CTRL = (WRITE_CDE 	| RIGHT);
				CTRL = (WRITE_CDE_E	| RIGHT);
				CTRL = (WRITE_CDE 	| RIGHT);
				CTRL = IDLE;


	//			for(ligne = 0; ligne <8; ligne ++){
	//				decalage=0b11111111;
	//				decalage=decalage <<1;

					//puissance entrée abaisseur
					afficher_carac_gauche(numero_lettre('P'));
					afficher_carac_gauche(numero_lettre('A'));
					afficher_carac_gauche(numero_lettre(':'));
					afficher_carac_gauche(numero_lettre(puissance_entree_A_string[0]));
					afficher_carac_gauche(numero_lettre(puissance_entree_A_string[1]));
					afficher_carac_gauche(numero_lettre(','));
					afficher_carac_gauche(numero_lettre(puissance_entree_A_string[2]));
					afficher_carac_gauche(numero_lettre(puissance_entree_A_string[3]));
					afficher_carac_gauche(numero_lettre(puissance_entree_A_string[4]));
					afficher_carac_gauche(numero_lettre('W'));



				for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
					//_delay_ms(1);
					ecran_disponible_gauche();
					// on remet le bus DATA en sortie
					DATA_DIR = 0xFF;
					// on met sur DATA le N° de colonne
					DATAW = 0b00000000;
					CTRL = (WRITE_RAM 	| LEFT);
					CTRL = (WRITE_RAM_E	| LEFT);
					CTRL = (WRITE_RAM 	| LEFT);
					CTRL = IDLE;

				}

				//puissance abaisseur
				afficher_carac_droit(numero_lettre(' '));
				afficher_carac_droit(numero_lettre('P'));
				afficher_carac_droit(numero_lettre(':'));
				afficher_carac_droit(numero_lettre(puissance_A_string[0]));
				afficher_carac_droit(numero_lettre(puissance_A_string[1]));
				afficher_carac_droit(numero_lettre(','));
				afficher_carac_droit(numero_lettre(puissance_A_string[2]));
				afficher_carac_droit(numero_lettre(puissance_A_string[3]));
				afficher_carac_droit(numero_lettre(puissance_A_string[4]));
				afficher_carac_droit(numero_lettre('W'));

				for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
					//_delay_ms(1);
					ecran_disponible_droit();
					// on remet le bus DATA en sortie
					DATA_DIR = 0xFF;
					// on met sur DATA le N° de colonne
					DATAW = 0b00000000;
					CTRL = (WRITE_RAM 	| RIGHT);
					CTRL = (WRITE_RAM_E	| RIGHT);
					CTRL = (WRITE_RAM 	| RIGHT);
					CTRL = IDLE;

				}





				//_delay_ms(1000);
			//}
		}while(1);
}
