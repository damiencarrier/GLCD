/*
 * GLCD.h
 *
 *  Created on: 13 nov. 2019
 *      Author: stetd
 */

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

#define WRITE_CDE 0x80
#define WRITE_CDE_E 0xC0
#define READ_STATUS 0xA0
#define READ_STATUS_E 0xE0
#define WRITE_RAM 0x90
#define WRITE_RAM_E 0xD0
#define READ_RAM 0xB0
#define READ_RAM_E 0xF0

#define LEFT 0x08
#define RIGHT 0x04

#define DISPLAY_ON 0x3F
#define DISPLAY_OFF 0x3E

#define COLUMN 0x40
#define PAGE 0xB8

#define RESET 0x3C
#define IDLE 0xBC



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
