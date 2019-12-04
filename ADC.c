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
//#include "lcd_font_5x7.h"
#include "commun.h"
#include "ADC.h"

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

//variable permettant de faire des mesures de tension
unsigned long tension_pinF0,tmp_tension_pinf;
unsigned long puissance_E,puissance_A,puissance_entree_E,puissance_entree_A,rendement_E,rendement_A;

//8 tableaux pour chaque pin de mesure
//qui contient 12 mesures chacunes
//pin0 est la mesure du 4096mv
//unsigned long tension_pinf[8][12];
//unsigned long tension_pinf_moyenne[8];
//typedef struct{
//	unsigned long tension_pinf[12];//tension en mv
//	unsigned long tension_pinf_moyenne;//tension moyenne en mv
//	unsigned char status; //0x01 quand la moyenne est faite
//}structure_tension;
//
//// Les 8 structures pour chaque voltmetre
//structure_tension base_donnees[8];

unsigned long reglage_quantum;

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
	// la référence de tension est à 4.091v mesurée
	base_donnees[0].tension_pinf[numero_mesure] = reglage_quantum/tension_pinF0;

	//premiere mesure sur 7
	ADMUX = 0b010000001;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC)| 0x06;
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	//ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC & 0x000003FF;
	base_donnees[1].tension_pinf[numero_mesure] = (tension_pinF0*base_donnees[0].tension_pinf[numero_mesure]*4)/100;

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
	base_donnees[2].tension_pinf[numero_mesure] = tension_pinF0*base_donnees[0].tension_pinf[numero_mesure]/100*2;

	ADMUX = 0b010000011;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC)| 0x06;
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	//ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC & 0x000003FF;
	base_donnees[3].tension_pinf[numero_mesure] = tension_pinF0*base_donnees[0].tension_pinf[numero_mesure]/100*5;

	ADMUX = 0b010000100;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC)| 0x06;
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	//ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC & 0x000003FF;
	base_donnees[4].tension_pinf[numero_mesure] = tension_pinF0*base_donnees[0].tension_pinf[numero_mesure]/100;

	ADMUX = 0b010000101;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC)| 0x06;
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	//ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC & 0x000003FF;
	base_donnees[5].tension_pinf[numero_mesure] = tension_pinF0*base_donnees[0].tension_pinf[numero_mesure]/100*2;

	ADMUX = 0b010000110;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC)| 0x06;
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	//ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC & 0x000003FF;
	base_donnees[6].tension_pinf[numero_mesure] = tension_pinF0*base_donnees[0].tension_pinf[numero_mesure]/100;

	ADMUX = 0b010000111;		// Initialisation de l'ADC avec selection AVCC
	// On lance la conversion
	ADCSRA = (1<<ADEN) | (1<<ADSC)| 0x06;
	// On attend que le bit ADSC repasse à 0 pour signifier la fin de conversion
	while(ADCSRA & (1<<ADSC));
	//ADCSRA =0;	// on éteint l'ADC
	tension_pinF0 = ADC & 0x000003FF;
	base_donnees[7].tension_pinf[numero_mesure] = tension_pinF0*base_donnees[0].tension_pinf[numero_mesure]/100;
}

//fait la moyenne de 10 mesures sur 12
//écarte les 2 extremes
void mesure_tension_moyennee(){

	//tri le min et le max des 12 valeurs
	for(int i=0;i<8;i++){
		base_donnees[i].status=0;
		//trie la plus grande et la plus grande et petite valeur
		for(int j=0;j<11;j++){
			//met en premier la plus petite valeur
			if(base_donnees[i].tension_pinf[0]>base_donnees[i].tension_pinf[j+1]){
				tmp_tension_pinf=base_donnees[i].tension_pinf[0];
				base_donnees[i].tension_pinf[0]=base_donnees[i].tension_pinf[j+1];
				base_donnees[i].tension_pinf[j+1]=tmp_tension_pinf;
			}
			//met en dernier la plus grande valeur
			if(base_donnees[i].tension_pinf[j+1]<base_donnees[i].tension_pinf[j]){
				tmp_tension_pinf=base_donnees[i].tension_pinf[j+1];
				base_donnees[i].tension_pinf[j+1]=base_donnees[i].tension_pinf[j];
				base_donnees[i].tension_pinf[j]=tmp_tension_pinf;
			}
		}
		//fait la moyenne des 10 valeurs sans les 2 extrèmes
		base_donnees[i].tension_pinf_moyenne= ( base_donnees[i].tension_pinf[1] + base_donnees[i].tension_pinf[2]
											  + base_donnees[i].tension_pinf[3] + base_donnees[i].tension_pinf[4]
											  + base_donnees[i].tension_pinf[5] + base_donnees[i].tension_pinf[6]
											  + base_donnees[i].tension_pinf[7] + base_donnees[i].tension_pinf[8]
											  + base_donnees[i].tension_pinf[9] + base_donnees[i].tension_pinf[10] )/10;
		base_donnees[i].status=1;
	}

	//calcule toutes les valeurs et construit les nombres en décimal
	puissance_entree_E = (base_donnees[1].tension_pinf_moyenne*base_donnees[2].tension_pinf_moyenne)/1000;
	puissance_entree_A = (base_donnees[1].tension_pinf_moyenne*base_donnees[7].tension_pinf_moyenne)/1000;
	puissance_E = (base_donnees[3].tension_pinf_moyenne*base_donnees[4].tension_pinf_moyenne)/1000;
	puissance_A = (base_donnees[5].tension_pinf_moyenne*base_donnees[6].tension_pinf_moyenne)/1000;
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
