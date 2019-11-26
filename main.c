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
#include "commun.h"



int main()
{
	GLCD_Code();
	return 0;
}
