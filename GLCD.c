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

//initialiser le LCD
//mettre CTRL_DIR en sortie
//mettre CTRL a RESET
//mettre DATA_DIR en sortie
//mettre DATA à 0xFF
//quitter le mode RESET  en mettant CTRL = IDLE
//
//tester que GLCD gauche est présent
//mettre DATA_DIR en entrée (0x00)
//mettre CTRL = READ_STATUS | GAUCHE
//mettre CTRL = READ-STATUS_E | GAUCHE
//mettre CTRL = READ_STATUS | GAUCHE
//lire le port de donnée du GLCD et mettre le résultat dans status_GAUCHE
//status_GAUCHE = DATAR ce qui signifie: lire le bus de donnée et le sauvegarder en status_GAUCHE
//mettre CTRL_CTRL en IDLE car on a fini le cycle et on va traiter status_GAUCHE
//vérifier que les 4 LSB des status_GAUCHE sont a 0. Si ce n'est pas le cas, pas de glcd qui répond.
//debug: on écrit status_GAUCHE sur le portb et on boucle sur un NOP while(1);
//
//si les 4 LSB sont à 1, tester le reset en vérifiant que le bit 4 de status_GAUCHE est à 0
//si ce n'est pas le cas, faire une boucle do while pour faire du pooling sur ce bit
//
//quand la phase de reset est terminée, faire de meme pour le coté droit
//
//quand les 2 NT7108C ont terminé la phase de reset, on les met on
//mettre DATA_DIR en sortie
//CTRL=WRITE_CDE | GAUCHE
//DATA=DISPLAY_ON
//CTRL=WRITE_CDE_E |GAUCHE
//CTRL=WRITE_CDE |GAUCHE
//CTRL=IDLE
//tester le bit display_on_off (bit 5) a 0 en réalisant a nouveau une lecture du status
//poller sur la lecture jusqu'a obtention du display ON et du bit busy à 0
//a ce stade on attend un status a 0x00. Si c'est le cas, on peu travailler
//écrire 0x55, puis 0xAA, puis 0x00 puis 0xFF dans la ram de gauche pouis dans la ram de droite

#endif /* GLCD_H_ */

// Ces deux variables recevront le registre de status des deux 1/2 ecrans.
unsigned char status_GAUCHE , status_DROIT;

//ecriture du N° de colonne dans la colonne, a gauche, puis l'inverse a droite
unsigned char no_colonne, no_page;

unsigned char ligne,decalage,ligne_carac;

//variable permettant de faire des mesures de tension
unsigned long tension_pinF0,tmp_tension_pinf;
unsigned long puissance_E,puissance_A,puissance_entree_E,puissance_entree_A,rendement_E,rendement_A;

static unsigned long tension_pinf[8][12];
//static unsigned long tension_pinf2[12];
//static unsigned long tension_pinf3[12];
//static unsigned long tension_pinf4[12];
//static unsigned long tension_pinf5[12];
//static unsigned long tension_pinf6[12];
//static unsigned long tension_pinf7[12];
//static unsigned long tension_pinf8[12];//tension de référence 4096mv
static unsigned long tension_pinf_moyenne[8];

//attend que l'écran sélectionné soit disponible
void ecran_disponible(GAUCHE_DROIT cote){
	// on commence par verifier que le bit busy est a 0
	// mise en entree du bus de DATA
	DATA_DIR = 0x00;
		do {
			CTRL = (READ_STATUS 	| cote);
			CTRL = (READ_STATUS_E	| cote);
			NOP;
			CTRL = (READ_STATUS 	| cote);
			CTRL = (READ_STATUS_E	| cote);
			NOP;
			status_GAUCHE = DATAR;
			CTRL = IDLE;
		} while (status_GAUCHE);
	DATA_DIR = 0xFF;
}

//met en noir l'écran sélectionné
void ecran_noir(GAUCHE_DROIT cote){
	//parcourt les 8 ligne a mettre en noir
	for (no_page = 0; no_page < 8; no_page ++) {
		// Quand on a rempli une page (64 ecritures), on change de page
		ecran_disponible(cote);
		// on met sur DATA la commande PAGE + le N° de colonne
		DATAW = PAGE | no_page;
		// et on ecrit la commande
		CTRL = (WRITE_CDE 	| cote);
		CTRL = (WRITE_CDE_E	| cote);
		CTRL = (WRITE_CDE 	| cote);
		CTRL = IDLE;

		//met en noir toute une ligne
		for (no_colonne = 0; no_colonne < 64; no_colonne ++) {
			ecran_disponible(cote);
			// on met sur DATA le N° de colonne
			DATAW = 0x00;
			CTRL = (WRITE_RAM 	| cote);
			CTRL = (WRITE_RAM_E	| cote);
			CTRL = (WRITE_RAM 	| cote);
			CTRL = IDLE;
			}
		}
}

//met en blanc l'écran sélectionné
void ecran_blanc(GAUCHE_DROIT cote){
	for (no_page = 0; no_page < 8; no_page ++) {
		// Quand on a rempli une page (64 ecritures), on change de page
		ecran_disponible(cote);
		// on met sur DATA la commande PAGE + le N° de colonne
		DATAW = PAGE | no_page;
		// et on ecrit la commande
		CTRL = (WRITE_CDE 	| cote);
		CTRL = (WRITE_CDE_E	| cote);
		CTRL = (WRITE_CDE 	| cote);
		CTRL = IDLE;
		for (no_colonne = 0; no_colonne < 64; no_colonne ++) {
			ecran_disponible(cote);
			// on met sur DATA le N° de colonne
			DATAW = 0xFF;
			CTRL = (WRITE_RAM 	| cote);
			CTRL = (WRITE_RAM_E	| cote);
			CTRL = (WRITE_RAM 	| cote);
			CTRL = IDLE;
			}
		}
}

//initialise l'écran avant de commencer
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
			CTRL = (READ_STATUS 	| GAUCHE);
			CTRL = (READ_STATUS_E	| GAUCHE);
			NOP;
			CTRL = (READ_STATUS 	| GAUCHE);
			CTRL = (READ_STATUS_E	| GAUCHE);
			NOP;
			status_GAUCHE = DATAR;
			CTRL = IDLE;
		} while (status_GAUCHE & 0xDF);

		// lecture du registre de status DROITE et verification que les 4 bits de poids faible sont a 0
		do {
			CTRL = (READ_STATUS 	| DROIT);
			CTRL = (READ_STATUS_E	| DROIT);
			NOP;
			CTRL = (READ_STATUS 	| DROIT);
			CTRL = (READ_STATUS_E	| DROIT);
			NOP;
			status_DROIT = DATAR;
			CTRL = IDLE;
		} while (status_DROIT & 0xDF);


		// Quand les 2 NT7108C ont termine la phase de reset, on les met ON
		// remettre DATA_DIR en sortie
		DATA_DIR = 0xFF;
		// mettre la valeur DIPLAY_ON sur le bus de DATA
		DATAW = DISPLAY_ON;
		// ecrire la commande Display_ON sur l'ecran gauche
		CTRL = (WRITE_CDE 	| GAUCHE);
		CTRL = (WRITE_CDE_E	| GAUCHE);
		CTRL = (WRITE_CDE 	| GAUCHE);
		CTRL = IDLE;

		// ecrire la commande Display_ON sur l'ecran droite
		CTRL = (WRITE_CDE 	| DROIT);
		CTRL = (WRITE_CDE_E	| DROIT);
		CTRL = (WRITE_CDE 	| DROIT);
		CTRL = IDLE;

		//verifier que les deux côtés sont ON (Tous les bits du registre de status à 0)

		// lecture du registre de status GAUCHE et verification que les 8 bits sont a 0
		// La lecture se fait en deux temps: Un premier front descendant sur E permet
		// de raffraichir le registre de données en sortie, puis on remet E a 1 pour
		// ouvrir le buffer tri-state et lire la donnée sur le bus.
}

//affiche sur l'écran le caractère donnée
void afficher_carac(GAUCHE_DROIT cote ,unsigned int ligne_font){

		for(ligne_carac=0;ligne_carac<5;ligne_carac++){
			ecran_disponible(cote);
			// on remet le bus DATA en sortie
			DATA_DIR = 0xFF;
			// on met sur DATA le N° de colonne
			DATAW = Font5x7[ligne_font + ligne_carac];
			CTRL = (WRITE_RAM 	| cote);
			CTRL = (WRITE_RAM_E	| cote);
			CTRL = (WRITE_RAM 	| cote);
			CTRL = IDLE;
		}
		ecran_disponible(cote);
		// on remet le bus DATA en sortie
		DATA_DIR = 0xFF;
		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| cote);
		CTRL = (WRITE_RAM_E	| cote);
		CTRL = (WRITE_RAM 	| cote);
		CTRL = IDLE;

}

//mesure toutes les tensions dans le tableau tension_pinf
void mesure_tension(unsigned int numero_mesure){
	//mesure la tension en millivolt

	//mesure la ref de tension 4096mv
	ADMUX = 0b010000000;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC)| 0x06;
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	//ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC & 0x000003FF;
	tension_pinf[0][numero_mesure] = 405500/950;

	//premiere mesure sur 7
	ADMUX = 0b010000001;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC)| 0x06;
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	//ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC & 0x000003FF;
	tension_pinf[1][numero_mesure] = (tension_pinF0*tension_pinf[0][numero_mesure]*3)/100;

	//fonction permettant de faire une conversion binaire BCD (que je n'utilise pas)
	//dtostrf(tension_pinf1,5,0,tension_string1);

	ADMUX = 0b010000010;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC)| 0x06;
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	//ADCSRA =0;	// on éteint l'ADC
	tension_pinF0=0;
	tension_pinF0 = ADC & 0x000003FF;
	tension_pinf[2][numero_mesure] = tension_pinF0*tension_pinf[0][numero_mesure]/100*2;

	ADMUX = 0b010000011;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC)| 0x06;
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	//ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC & 0x000003FF;
	tension_pinf[3][numero_mesure] = tension_pinF0*tension_pinf[0][numero_mesure]/100*5;

	ADMUX = 0b010000100;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC)| 0x06;
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	//ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC & 0x000003FF;
	tension_pinf[4][numero_mesure] = tension_pinF0*tension_pinf[0][numero_mesure]/100;

	ADMUX = 0b010000101;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC)| 0x06;
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	//ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC & 0x000003FF;
	tension_pinf[5][numero_mesure] = tension_pinF0*tension_pinf[0][numero_mesure]/100;

	ADMUX = 0b010000110;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC)| 0x06;
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	//ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC & 0x000003FF;
	tension_pinf[6][numero_mesure] = tension_pinF0*tension_pinf[0][numero_mesure]/100;

	ADMUX = 0b010000111;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC)| 0x06;
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	//ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC & 0x000003FF;
	tension_pinf[7][numero_mesure] = tension_pinF0*tension_pinf[0][numero_mesure]/100;
}

//fait la moyenne sur 12 valeurs
//écarte les 2 extremes
void mesure_tension_moyennee(){
	//mesure 12 valeurs des 8 tensions
	for(int i=0;i<12;i++){
		mesure_tension(i);
	}

	//tri les 12 valeurs
	for(int i=0;i<8;i++){
			//trie la plus grande et la plus grande et petite valeur
			for(int j=0;j<11;j++){
				//met en premier la plus petite valeur
				if(tension_pinf[i][0]>tension_pinf[i][j+1]){
					tmp_tension_pinf=tension_pinf[i][0];
					tension_pinf[i][0]=tension_pinf[i][j+1];
					tension_pinf[i][j+1]=tmp_tension_pinf;
				}
				//met en dernier la plus grande valeur
				if(tension_pinf[i][j+1]<tension_pinf[i][j]){
					tmp_tension_pinf=tension_pinf[i][j+1];
					tension_pinf[i][j+1]=tension_pinf[i][j];
					tension_pinf[i][j]=tmp_tension_pinf;
				}
			}
			//fait la moyenne des 10 valeurs sans les 2 extrèmes
			tension_pinf_moyenne[i]= ( tension_pinf[i][1] + tension_pinf[i][2]
									 + tension_pinf[i][3] + tension_pinf[i][4]
									 + tension_pinf[i][5] + tension_pinf[i][6]
									 + tension_pinf[i][7] + tension_pinf[i][8]
									 + tension_pinf[i][9] + tension_pinf[i][10] )/10;

	}

	//calcule toutes les valeurs et construit les nombres en décimal
	puissance_entree_E = (tension_pinf_moyenne[1]/10) * (tension_pinf_moyenne[2]/100);
	puissance_entree_A = (tension_pinf_moyenne[1]/10) * (tension_pinf_moyenne[7]/100);
	puissance_E = (tension_pinf_moyenne[3]/10) * (tension_pinf_moyenne[4]/100);
	puissance_A = (tension_pinf_moyenne[5]/10) * (tension_pinf_moyenne[6]/100);
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
}

//donne la valeur de la lettre dans la font
unsigned int numero_lettre(unsigned char lettre){
	return((lettre*5)-(32*5));
	//0> 80
	//A>165
	//a>345
}

// le masque de l'écran qui affiche les information fixes de l'écran
void masque_ecran(){
	ecran_disponible(GAUCHE);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = COLUMN | 0;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = (WRITE_CDE_E	| GAUCHE);
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = IDLE;

	ecran_disponible(DROIT);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = COLUMN | 0;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = (WRITE_CDE_E	| DROIT);
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = IDLE;

	//for (no_page = 0; no_page < 8; no_page ++) {
	ecran_disponible(GAUCHE);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = PAGE | 0;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = (WRITE_CDE_E	| GAUCHE);
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = IDLE;

	ecran_disponible(DROIT);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = PAGE | 0;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = (WRITE_CDE_E	| DROIT);
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = IDLE;

	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre('B'));
	afficher_carac(GAUCHE,numero_lettre('a'));
	afficher_carac(GAUCHE,numero_lettre('t'));
	afficher_carac(GAUCHE,numero_lettre('t'));
	afficher_carac(GAUCHE,numero_lettre('e'));
	afficher_carac(GAUCHE,numero_lettre('r'));
	afficher_carac(GAUCHE,numero_lettre('i'));
	afficher_carac(GAUCHE,numero_lettre('e'));
	afficher_carac(GAUCHE,numero_lettre(' '));

	for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
		//_delay_ms(1);
		ecran_disponible(GAUCHE);
		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| GAUCHE);
		CTRL = (WRITE_RAM_E	| GAUCHE);
		CTRL = (WRITE_RAM 	| GAUCHE);
		CTRL = IDLE;
	}

	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre('E'));
	afficher_carac(DROIT,numero_lettre('l'));
	afficher_carac(DROIT,numero_lettre('e'));
	afficher_carac(DROIT,numero_lettre('v'));
	afficher_carac(DROIT,numero_lettre('a'));
	afficher_carac(DROIT,numero_lettre('t'));
	afficher_carac(DROIT,numero_lettre('e'));
	afficher_carac(DROIT,numero_lettre('u'));
	afficher_carac(DROIT,numero_lettre('r'));

	for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
		//_delay_ms(1);
		ecran_disponible(DROIT);
		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| DROIT);
		CTRL = (WRITE_RAM_E	| DROIT);
		CTRL = (WRITE_RAM 	| DROIT);
		CTRL = IDLE;
	}

	ecran_disponible(GAUCHE);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = PAGE | 1;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = (WRITE_CDE_E	| GAUCHE);
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = IDLE;

	ecran_disponible(DROIT);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = PAGE | 1;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = (WRITE_CDE_E	| DROIT);
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = IDLE;

	//tension V batterie
	afficher_carac(GAUCHE,numero_lettre('V'));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(':'));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre('V'));

	for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
		//_delay_ms(1);
		ecran_disponible(GAUCHE);

		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| GAUCHE);
		CTRL = (WRITE_RAM_E	| GAUCHE);
		CTRL = (WRITE_RAM 	| GAUCHE);
		CTRL = IDLE;
	}

	// tension élévateur
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre('v'));
	afficher_carac(DROIT,numero_lettre(':'));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre('V'));


	for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
		//_delay_ms(1);
		ecran_disponible(DROIT);

		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| DROIT);
		CTRL = (WRITE_RAM_E	| DROIT);
		CTRL = (WRITE_RAM 	| DROIT);
		CTRL = IDLE;
	}

	ecran_disponible(GAUCHE);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = PAGE | 2;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = (WRITE_CDE_E	| GAUCHE);
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = IDLE;

	ecran_disponible(DROIT);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = PAGE | 2;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = (WRITE_CDE_E	| DROIT);
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = IDLE;

	//IE courant entrée
	afficher_carac(GAUCHE,numero_lettre('I'));
	afficher_carac(GAUCHE,numero_lettre('E'));
	afficher_carac(GAUCHE,numero_lettre(':'));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre('A'));



	for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
		//_delay_ms(1);
		ecran_disponible(GAUCHE);

		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| GAUCHE);
		CTRL = (WRITE_RAM_E	| GAUCHE);
		CTRL = (WRITE_RAM 	| GAUCHE);
		CTRL = IDLE;
	}

	// courant elevateur
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre('i'));
	afficher_carac(DROIT,numero_lettre(':'));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre('A'));

	for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
		//_delay_ms(1);
		ecran_disponible(DROIT);

		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| DROIT);
		CTRL = (WRITE_RAM_E	| DROIT);
		CTRL = (WRITE_RAM 	| DROIT);
		CTRL = IDLE;
	}

	ecran_disponible(GAUCHE);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = PAGE | 3;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = (WRITE_CDE_E	| GAUCHE);
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = IDLE;

	ecran_disponible(DROIT);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = PAGE | 3;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = (WRITE_CDE_E	| DROIT);
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = IDLE;

	//puissance entrée élévateur
	afficher_carac(GAUCHE,numero_lettre('P'));
	afficher_carac(GAUCHE,numero_lettre('E'));
	afficher_carac(GAUCHE,numero_lettre(':'));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre('W'));

	for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
		//_delay_ms(1);
		ecran_disponible(GAUCHE);

		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| GAUCHE);
		CTRL = (WRITE_RAM_E	| GAUCHE);
		CTRL = (WRITE_RAM 	| GAUCHE);
		CTRL = IDLE;
	}

	//puissance élévateur
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre('P'));
	afficher_carac(DROIT,numero_lettre(':'));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre('W'));

	for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
		//_delay_ms(1);
		ecran_disponible(DROIT);

		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| DROIT);
		CTRL = (WRITE_RAM_E	| DROIT);
		CTRL = (WRITE_RAM 	| DROIT);
		CTRL = IDLE;
	}

	ecran_disponible(GAUCHE);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = PAGE | 4;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = (WRITE_CDE_E	| GAUCHE);
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = IDLE;

	ecran_disponible(DROIT);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = PAGE | 4;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = (WRITE_CDE_E	| DROIT);
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = IDLE;

	//rendement élévateur
	afficher_carac(GAUCHE,numero_lettre('N'));
	afficher_carac(GAUCHE,numero_lettre('E'));
	afficher_carac(GAUCHE,numero_lettre(':'));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre('%'));

	for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
		//_delay_ms(1);
		ecran_disponible(GAUCHE);

		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| GAUCHE);
		CTRL = (WRITE_RAM_E	| GAUCHE);
		CTRL = (WRITE_RAM 	| GAUCHE);
		CTRL = IDLE;
	}

	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre('A'));
	afficher_carac(DROIT,numero_lettre('b'));
	afficher_carac(DROIT,numero_lettre('a'));
	afficher_carac(DROIT,numero_lettre('i'));
	afficher_carac(DROIT,numero_lettre('s'));
	afficher_carac(DROIT,numero_lettre('s'));
	afficher_carac(DROIT,numero_lettre('e'));
	afficher_carac(DROIT,numero_lettre('u'));
	afficher_carac(DROIT,numero_lettre('r'));

	for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
		//_delay_ms(1);
		ecran_disponible(DROIT);

		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| DROIT);
		CTRL = (WRITE_RAM_E	| DROIT);
		CTRL = (WRITE_RAM 	| DROIT);
		CTRL = IDLE;

	}

	ecran_disponible(GAUCHE);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = PAGE | 5;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = (WRITE_CDE_E	| GAUCHE);
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = IDLE;

	ecran_disponible(DROIT);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = PAGE | 5;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = (WRITE_CDE_E	| DROIT);
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = IDLE;

	//rendement abaisseur
	afficher_carac(GAUCHE,numero_lettre('N'));
	afficher_carac(GAUCHE,numero_lettre('A'));
	afficher_carac(GAUCHE,numero_lettre(':'));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre('%'));

	for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
		//_delay_ms(1);
		ecran_disponible(GAUCHE);

		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| GAUCHE);
		CTRL = (WRITE_RAM_E	| GAUCHE);
		CTRL = (WRITE_RAM 	| GAUCHE);
		CTRL = IDLE;
	}

	//tension abaisseur
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre('V'));
	afficher_carac(DROIT,numero_lettre(':'));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre('V'));

	for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
		//_delay_ms(1);
		ecran_disponible(DROIT);

		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| DROIT);
		CTRL = (WRITE_RAM_E	| DROIT);
		CTRL = (WRITE_RAM 	| DROIT);
		CTRL = IDLE;

	}

	ecran_disponible(GAUCHE);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = PAGE | 6;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = (WRITE_CDE_E	| GAUCHE);
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = IDLE;

	ecran_disponible(DROIT);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = PAGE | 6;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = (WRITE_CDE_E	| DROIT);
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = IDLE;

	//courant entrée abaisseur
	afficher_carac(GAUCHE,numero_lettre('I'));
	afficher_carac(GAUCHE,numero_lettre('A'));
	afficher_carac(GAUCHE,numero_lettre(':'));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre('A'));



	for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
		//_delay_ms(1);
		ecran_disponible(GAUCHE);

		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| GAUCHE);
		CTRL = (WRITE_RAM_E	| GAUCHE);
		CTRL = (WRITE_RAM 	| GAUCHE);
		CTRL = IDLE;

	}

	//courant abaisseur
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre('i'));
	afficher_carac(DROIT,numero_lettre(':'));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre('A'));

	for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
		//_delay_ms(1);
		ecran_disponible(DROIT);

		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| DROIT);
		CTRL = (WRITE_RAM_E	| DROIT);
		CTRL = (WRITE_RAM 	| DROIT);
		CTRL = IDLE;

	}

	ecran_disponible(GAUCHE);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = PAGE | 7;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = (WRITE_CDE_E	| GAUCHE);
	CTRL = (WRITE_CDE 	| GAUCHE);
	CTRL = IDLE;

	ecran_disponible(DROIT);

	// on met sur DATA la commande PAGE + le N° de colonne
	DATAW = PAGE | 7;
	// et on ecrit la commande
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = (WRITE_CDE_E	| DROIT);
	CTRL = (WRITE_CDE 	| DROIT);
	CTRL = IDLE;

	//puissance entrée abaisseur
	afficher_carac(GAUCHE,numero_lettre('P'));
	afficher_carac(GAUCHE,numero_lettre('A'));
	afficher_carac(GAUCHE,numero_lettre(':'));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre(' '));
	afficher_carac(GAUCHE,numero_lettre('W'));



	for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
		//_delay_ms(1);
		ecran_disponible(GAUCHE);

		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| GAUCHE);
		CTRL = (WRITE_RAM_E	| GAUCHE);
		CTRL = (WRITE_RAM 	| GAUCHE);
		CTRL = IDLE;

	}

	//puissance abaisseur
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre('P'));
	afficher_carac(DROIT,numero_lettre(':'));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre(' '));
	afficher_carac(DROIT,numero_lettre('W'));

	for (no_colonne = 0; no_colonne < 4; no_colonne ++) {
		//_delay_ms(1);
		ecran_disponible(DROIT);

		// on met sur DATA le N° de colonne
		DATAW = 0b00000000;
		CTRL = (WRITE_RAM 	| DROIT);
		CTRL = (WRITE_RAM_E	| DROIT);
		CTRL = (WRITE_RAM 	| DROIT);
		CTRL = IDLE;

	}
}

//positionne la ligne sur laquelle s'affichera les caractères
void glcd_y (GAUCHE_DROIT cote, unsigned char no_ligne){
		ecran_disponible(cote);
		// positionnement de la data column sur le bus de DATA
		// le N° de colonne reçu correspond à une position de caractere, sachant qu'un caractere
		// occupe 5 colonnes + une colonne de séparation
		DATAW = PAGE | no_ligne;
		// ecrire DATAW sur l'ecran specifie
		CTRL = (WRITE_CDE 	| cote);
		CTRL = (WRITE_CDE_E	| cote);
		CTRL = (WRITE_CDE 	| cote);
		CTRL = IDLE;
}

//positionne le "curseur" sur le bon caractere
void glcd_x (GAUCHE_DROIT cote, unsigned char no_caractere){
		ecran_disponible(cote);
		// positionnement de la data column sur le bus de DATA
		// le N° de colonne reçu correspond à une position de caractere, sachant qu'un caractere
		// occupe 5 colonnes + une colonne de séparation
		DATAW = COLUMN | (no_caractere*6);
		// ecrire DATAW sur l'ecran specifie
		CTRL = (WRITE_CDE 	| cote);
		CTRL = (WRITE_CDE_E	| cote);
		CTRL = (WRITE_CDE 	| cote);
		CTRL = IDLE;
}

//construit les chiffres pour l'afficher à l'écran
void glcd_write_nombre(GAUCHE_DROIT cote, u8 ligne, u32 nombre){
	if( nombre > 99994){
		u8 message_erreur[] = "99,++";
		glcd_y(cote,ligne);

		//placement sur le caractere n°1 ( le premier caractere est 0) a gauche
		glcd_x(cote,4);

		//message "batterie"
		u8 i = 0;
		do{
			afficher_carac(cote, (message_erreur[i]-32)*5);
			i++;
		}while (message_erreur[i]);

	}
	else{
		//variables des chaque chiffres à inscrire sur le lcd en BCD
		u8 dizaine, unite, dixieme, centieme, millieme;

		//variables de restes centaines = nombre de centaines, dizaines = nombre de dizaines
		//		u16 nbr_unites, nbr_dixiemes;
		//		u8 nbr_centiemes;

		//calcul du chiffre des dizaine
		dizaine = nombre / 10000 ;
		//reste les nombres d'unité
		nombre = nombre % 10000 ;
		//calcul du chiffre des unité
		unite = nombre / 1000 ;
		//reste les nombres de nbr_dixiemes
		nombre = nombre % 1000 ;
		//calcul du chiffre des dixieme
		dixieme = nombre / 100 ;
		// reste les nombres des centieme
		nombre = nombre % 100 ;
		// calcule du chiffre des  centieme
		centieme = nombre / 10 ;
		// reste le chiffre des milliemes
		millieme = nombre % 10 ;

		//si les millieme sont infierieur ou egal à 5 on aronddi au centieme superieur
		if(millieme > 4 ){
			//arrondi s
			if(centieme<9){
				centieme++;
			}else if (dixieme<9){
				centieme=0;
				dixieme++;
			}else if(unite<9){
				centieme=0;
				dixieme=0;
				unite++;
			}else if(dizaine<9){
				centieme=0;
				dixieme=0;
				unite=0;
				dizaine++;
			}
		}

		//le code ascii des nombres à 3 comme valeur sur le 1er quarter (0x30)
		//donc si notre chiffre vaut 9, le code ascii sera 0x39
		dizaine = dizaine | 0x30;
		unite = unite | 0x30;
		dixieme = dixieme | 0x30;
		centieme = centieme | 0x30;

		//parametrage d'ecriture en ligne 2
		glcd_y(cote, ligne);

		//parametrage d'ecriture en colonne de caractere 3
		glcd_x(cote, 4);

		afficher_carac(cote, (dizaine-32)*5 );
		afficher_carac(cote, (unite-32)*5 );


		afficher_carac(cote, (',' - 32)*5 );
		afficher_carac(cote, (dixieme-32)*5 );
		afficher_carac(cote, (centieme-32)*5 );
	}
}

//fonction principale affichant toutes les tensions
void GLCD_Code(){
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
		//affiche toutes les valeurs
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

		_delay_ms(250);
		}while(1);
}
