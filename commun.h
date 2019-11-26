/*
 * commun.h
 *
 *  Created on: 19 nov. 2019
 *      Author: formateur
 */

#ifndef COMMUN_H_
#define COMMUN_H_

// creation de 3 types de variables non signées, octet, mot, double mot
#define u8 unsigned char
#define u16 unsigned short int
#define u32 unsigned long int


// DEBUG demande au compilateur d'intégrer le code de debuggage
// définition à enlever pour les versions livrables.
#define DEBUG

typedef enum {	GAUCHE	= 0x08 , DROIT 	= 0x04} GAUCHE_DROIT;


#endif /* COMMUN_H_ */
