#include <stdlib.h>

#include "inttypes.h"
#include "lcd.h"


#include "startup/general.h"
#include "startup/printf_P.h"
#include "startup/ea_init.h"
#include "startup/lpc2xxx.h" //definicje
#include "startup/consol.h"
#include "startup/config.h"

#include "pff2/src/pff.h"

#include "functions.h"

#include "wave.h"

#include "dac.h"

#include "adc.h"

#include "timer.h"

#include <string.h>

static void hang(void){
	initLed(4);
	while(1){
		turnOnLed(4);
		delayT(0x100000);
		turnOffLed(4);
		delayT(0x100000);
	}
}

void delayS (void)  {                   //definicja funkcji op�niaj�cej
  tU32 i;   //deklaracja zmiennej, kt�ra pos�u�y jako licznik petli
  for(i = 0; i <= 0x100000; i++){};                //op�znienie czasowe
}



FATFS fatfs;			/* File system object */
DIR dir;				/* Directory object */
FILINFO fno;			/* File information object */
DWORD rc;



int main (void) {
	eaInit();   //initialize printf

	srand(102); //inicjalizacja generatora liczb losowy, liczba startowa
				//uzyskana losowo od mojej siostry ;)

	printf("*******************************************************\n");
	printf("*                                                     *\n");
	printf("* EmPeCzy                                             *\n");
	printf("*                                                     *\n");
	printf("*******************************************************\n");


	//ustawimy nasz ekran
	lcdInit();
	lcdColor(0xff,0x00);//bia�e t�o, czarny tekst
	lcdClrscr();

	//init DAC
	initDac();

	//init ADC
	initAdc();

	//init LED
	initLeds();

	//init accel
	initAccel();

	printf("\nMontuje wolumin.\n");
	rc = pf_mount(&fatfs);
	if (rc){
		printf("blad po mount %x \n", rc);
		if( FR_DISK_ERR == rc || FR_NOT_READY == rc ){
			lcdGotoxy(0,0);
			lcdPuts("BLAD INTERFEJSU");
			printf("Blad interfejsu");
		}else if( FR_NO_FILESYSTEM == rc ){
			lcdGotoxy(0,0);;
			lcdPuts("BLAD SYSTEMU_PL");
			printf("Nieprawidlowy system plikow na karcie pamieci");
		}
		hang();
	}


	rc = listDir("/", TRUE); //poka� dok�adn� zawarto�� karty
	if (rc){
		printf("listDir, rc=%x\n", rc);
	}

	char pliki[256 * 12];

	//uint32 filesCount = filesList("/", pliki) ; // stw�rz liste plik�w
	tU32 filesCount = filesList("/", pliki) ; // stw�rz liste plik�w
	if( filesCount == 0){
		printf("fileList => brak plikow, lub nie mozna ich odczytac");
		hang();
	}

	printf("Liczba plikow WAV: %d\n", filesCount);

	//int32 i = 0;
	tS32 i = 0;

	PlayerStatus status;
	status = Player_Stoped;

	TimerStatus timerStatus;
	timerStatus = Timer_UP;

	char nazwa[12];

	strncpy(nazwa, &pliki[i*12],12);
	printf("Aktualny plik: %s\n", nazwa);
	lcdGotoxy(0,0);
	lcdPuts(nazwa);

	lcdGotoxy(15*8,0);
	lcdPuts("#");
	while(1){ //g��wna p�tla programu
		updateStatus( &status, &timerStatus, TRUE ); //sprawd� przyciski
		if( i < 0 )
			i = filesCount - 1;
		if( i >= filesCount )
			i = 0;

		if( status == Player_Next ){
			printf("nastepny\n");
			status = Player_Stoped;
			i++;
			strncpy(nazwa, &pliki[(i%filesCount)*12],12);
			printf("Aktualny plik: %s\n", nazwa);
			lcdGotoxy(0,0);
			lcdPuts(nazwa);
			lcdGotoxy(12*8,0);
			lcdPuts(" ");
			continue;
		}
        
		if( status == Player_Prev ){
			printf("poprzedni\n");
			status = Player_Stoped;
			i--;
			strncpy(nazwa, &pliki[(i%filesCount)*12],12);
			printf("Aktualny plik: %s\n", nazwa);
			lcdGotoxy(0,0);
			lcdPuts(nazwa);
			lcdGotoxy(12*8,0);
			lcdPuts(" ");
			continue;
		}

		if( status == Player_Random ){ //potrz��ni�to akcelerometrem
			printf("losowy");
			status = Player_Playing;
			i = rand() % filesCount;
			strncpy(nazwa, &pliki[(i%filesCount)*12],12);
			printf("Aktualny plik: %s\n", nazwa);
			lcdGotoxy(0,0);
			lcdPuts(nazwa);
			lcdGotoxy(12*8,0);
			lcdPuts(" ");
			//continue;
		}

		if( status == Player_Playing ){ //wci�ni�to przycisk play

			lcdRect(0,32,9*8,16, 0xff); //wyczy�� miejsce gdze pokazywany czas
			lcdGotoxy(15*8,0);
			lcdPuts(">"); //umie�� symbol w prawym g�rnym rogu

			strncpy(nazwa, &pliki[i*12],12);

			lcdGotoxy(0,0);
			lcdPuts(nazwa); //poka� nazwe odtwarzanego pliku

			printf("\nOtwieram plik: %s.\n", nazwa);
			rc = pf_open(nazwa);
			if (rc){
				lcdPuts("NIE MOGE OTWORZYC");
				printf("nie moge otworzyc pliku (%s),", nazwa);
				printf(" rc=%x\n", rc);
				if( FR_NO_FILE == rc )
					printf("nie ma takiego pliku\n");
				if( FR_NOT_ENABLED == rc )
					printf("problem z systemem plikow\n");
				hang();
			}


			if( playerInit() ){ //inicjacja nag��wk�w wave

				if( playWave( &status, &timerStatus ) ) //odtwarzamy
					i++; //gdy zako�czone naturalnie, przeskocz do nast�pnego

				lcdRect(0,32,9*8,16, 0xff);
				lcdGotoxy(15*8,0);
				lcdPuts("#");
			}else{
				printf("Nie udalo sie zainicjowac pliku\n");
				status = Player_Stoped;
				lcdGotoxy(15*8,0);
				lcdPuts("X");
			}
		}

	}

	//tutaj nie ma prawa nigdy doj��, ale w razie czego trzeba zadba� o par�
	//rzeczy, dlak dla spokojnego ducha.

	//odmontuj system plik�w
	printf("\nodmontuj system plikow\n");
	rc = pf_mount(0);
	if( rc )
		printf("jakis blad przy odmontowywaniu, rc=%x\n");

	initLed(1);

  while(1) { //p�tla niesko�czona, mrugaj dioda P1.17
	  turnOnLed(1);
	  zegarek(2000000);
	  //delayS();

	  turnOffLed(1);
	  zegarek(2000000);
	  //delayS();
  }
}
