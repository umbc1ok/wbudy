#include "general.h"
#include <lpc2xxx.h>
#include <printf_P.h>
#include <printf_init.h>
#include <consol.h>
#include <config.h>
#include "irq/irq_handler.h"
#include "timer.h"
#include "VIC.h"
#include "i2c.h"
#include <stdio.h>
#include <string.h>

// Include'y do urządzeń peryferyjnych

#include "lm75.h"
#include "htu21df.h"
#include "bmp180.h"
#include "tsl2561.h"

// End

#include "lcd.h"
#include "pca9532.h"
#include "additional.h"

#include "Common_Def.h"

#define STEP_DELAY 50
#define KEY_LEFT   0x0080
#define KEY_RIGHT  0x0100




/* VREF start-up time */
#define VREF_STARTUP_TIME       (50)
/* Mask needed to get the 2 LSb for DAC Data Register */
#define LSB_MASK                (0x03)
/* Number of samples for a sine wave period */
#define SINE_PERIOD_STEPS       (100)
/* Sine wave amplitude */
#define SINE_AMPLITUDE          (511)
/* Sine wave DC offset */
#define SINE_DC_OFFSET          (512)
/* Frequency of the sine wave */
#define SINE_FREQ               (100)
/* Step delay for the loop */
#define STEP_DELAY_TIME         ((1000000 / SINE_FREQ) / SINE_PERIOD_STEPS)


#define M_PI 3.14159265358979323846

/*!
 *  @brief    Uruchomienie obsługi przerwań
 *  @param period
 *             Okres generatora przerwań
 *  @param duty_cycle
 *             Wypełnienie w %
 *
 *  @returns  Nic
 *  @side effects:
 *            Przeprogramowany timer #1
 */

/*!
 *  @brief Main function of the entire program - responsible for managing all user
 * 		   interaction and showing them results of these actions.
 *  @returns Technically, this function is executed constantly, so in fact it does not return
 * 			 any value.
 *  @side effects: As for now, they are possible, but not yet known.
 */
void showTime(tU32 sec,tU32 min,tU32 hour){
	// zrobic ewentualnie zeby przekazywac s
    //mozliwe ze zamiana char na tU8 cos psuje
	/*
    tU8 tabS[3];
    sprintf(tabS, "%d", RTC_SEC);
    tU8 tabMin[3];
    sprintf(tabMin, "%d", RTC_MIN);
    tU8 tabHr[3];
    sprintf(tabHr, "%d", RTC_HOUR);*/

	tU8 tabS[3];
    sprintf(tabS, "%d", sec);
    tU8 tabMin[3];
    sprintf(tabMin, "%d", min);
    tU8 tabHr[3];
    sprintf(tabHr, "%d", hour);

    tabS[2] = '\0';
    tabMin[2] = '\0';
    tabHr[2] = '\0';

    if (sec < 10) {
        tU8 temp1 = tabS[0];
        tabS[1] = temp1;
        tabS[0] = '0';
    }
    if (min < 10) {
        tU8 temp2 = tabMin[0];
        tabMin[1] = temp2;
        tabMin[0] = '0';
    }
    if (hour < 10) {
        tU8 temp3 = tabHr[0];
        tabHr[1] = temp3;
        tabHr[0] = '0';
    }

    lcdGotoxy(10, 50);
    lcdPuts(tabHr);

    lcdGotoxy(30, 50);
    lcdPuts(":");

    lcdGotoxy(50, 50);
    lcdPuts(tabMin);

    lcdGotoxy(70, 50);
    lcdPuts(":");

    lcdGotoxy(90, 50);
    lcdPuts(tabS);
}

void blink(void){
	tU16 keys;
	tU8 pca9532Present = FALSE;
	pca9532Present = pca9532Init();
	  if (TRUE == pca9532Present)
	  {
		/*
		if (!(keys & KEY_LEFT))  setPca9532Pin(0, 0);
		if (!(keys & KEY_RIGHT)) setPca9532Pin(8, 0);
		if (!(keys & KEY_LEFT))  setPca9532Pin(1, 0);
		if (!(keys & KEY_RIGHT)) setPca9532Pin(9, 0);
		if (!(keys & KEY_LEFT))  setPca9532Pin(2, 0);
		if (!(keys & KEY_RIGHT)) setPca9532Pin(10, 0);
		if (!(keys & KEY_LEFT))  setPca9532Pin(3, 0);
		if (!(keys & KEY_RIGHT)) setPca9532Pin(11, 0);
		if (!(keys & KEY_LEFT))  setPca9532Pin(4, 0);
		if (!(keys & KEY_RIGHT)) setPca9532Pin(12, 0);
		if (!(keys & KEY_LEFT))  setPca9532Pin(5, 0);
		if (!(keys & KEY_RIGHT)) setPca9532Pin(13, 0);
		if (!(keys & KEY_LEFT))  setPca9532Pin(6, 0);
		if (!(keys & KEY_RIGHT)) setPca9532Pin(14, 0);
		if (!(keys & KEY_LEFT))  setPca9532Pin(7, 0);
		if (!(keys & KEY_RIGHT)) setPca9532Pin(15, 0);*/
		setPca9532Pin(0, 1);
		setPca9532Pin(1, 1);
		setPca9532Pin(2, 1);
		setPca9532Pin(3, 1);
		setPca9532Pin(4, 1);
		setPca9532Pin(5, 1);
		setPca9532Pin(6, 1);
		setPca9532Pin(7, 1);
		setPca9532Pin(8, 1);
		setPca9532Pin(9, 1);
		setPca9532Pin(10, 1);
		setPca9532Pin(11, 1);
		setPca9532Pin(12, 1);
		setPca9532Pin(13, 1);
		setPca9532Pin(14, 1);
		setPca9532Pin(15, 1);
	  	//keys = ~getPca9532Pin();
	  }
}

int main(void)
{

	// I2C initalization
	i2cInit();

	tU8 lm75address = (((tU8)0x48 << 1) | (tU8)1);
	printf_init();
	// LCD display initialization
	lcdInit();
	// Setting colors for background and text.
	lcdColor(0xff, 0x00);
	lcdClrscr();


	tU8 isTimeBeingSet = FALSE;
	tU16 currentType = 0;
	tU8 charArray[10] = {0};
	tU8 *messagePointer;
	tU8 messageHolder[128];
	tU8 readTemperature[2] = {0};


	RTC_CCR = 0x00000012;
	RTC_CCR = 0x00000010;
	RTC_ILR = 0x00000000;
	RTC_CIIR = 0x00000000;
	RTC_AMR = 0x00000000;
	//PREINT  = 449;
	//PREFRAC = 0;
	RTC_SEC = 50;
	RTC_MIN = 59;
	RTC_HOUR = 23;
	RTC_CCR = 0x00000011;
	tU32 temp_SEC = RTC_SEC; //to jednak nie sa dobre typy, adresy sa w tU32, powinno tu byc prawdopodobnie tU8, bo te wartosci sa 5-6 bitowe
    tU32 temp_MIN = RTC_MIN;
    tU32 temp_HOUR = RTC_HOUR;

	IODIR |= 0x00008000; //P0.15

	IODIR |= 0x00260000; //RGB
	IOSET = 0x00260000;

	IODIR1 |= 0x000F0000; //LEDs
	IOSET1 = 0x000F0000;
	IOCLR1 = 0x00030000;
	IOCLR1 = 0x00050000;
	IOCLR1 = 0x000c0000;
	IOCLR1 = 0x00090000;
	IOSET1 = 0x000F0000;
	IODIR1 &= ~0x00F00000; //Keys

	tU16 sineWave[SINE_PERIOD_STEPS];
	tU8 i;
    for(i = 0; i < SINE_PERIOD_STEPS; i++)
    {
        sineWave[i] = SINE_DC_OFFSET + SINE_AMPLITUDE * sin(2 * M_PI * i / SINE_PERIOD_STEPS);
    }

	while (TRUE) {
		showTime(RTC_SEC, RTC_MIN,RTC_HOUR);
        //check if P0.8 center-key is pressed
        if ((IOPIN & 0x00000100) == 0 && !isTimeBeingSet)
        {
            isTimeBeingSet = TRUE;
			temp_SEC = RTC_SEC;
			temp_MIN = RTC_MIN;
			temp_HOUR = RTC_HOUR;

			showTime(temp_SEC,temp_MIN,temp_HOUR);
			sdelay(1);
			while(isTimeBeingSet){
				//check if P0.8 center-key is pressed
				if ((IOPIN & 0x00000100) == 0)
				{
					currentType++;
					if(currentType<0 || currentType > 2){
						isTimeBeingSet = FALSE;
					}
					sdelay(1);
				}
				//check if P0.10 up-key is pressed
				else if((IOPIN & 0x0000400) == 0){
					switch(currentType){
						case 0:
							temp_HOUR++;
							break;
						case 1:
							temp_MIN++;
							break;
						case 2:
							temp_SEC++;
							break;
					}
					showTime(temp_SEC,temp_MIN,temp_HOUR);
					sdelay(1);
				}
				//check if P0.12 down-key is pressed
				else if((IOPIN & 0x00001000) == 0){
					switch(currentType){
						case 0:
							temp_HOUR--;
							break;
						case 1:
							temp_MIN--;
							break;
						case 2:
							temp_SEC--;
							break;
					}
					showTime(temp_SEC,temp_MIN,temp_HOUR);
					sdelay(1);
				}

			}
        }
		if(RTC_HOUR == temp_HOUR && RTC_MIN == temp_MIN && RTC_SEC == temp_SEC){
			showTime(RTC_SEC,RTC_MIN,RTC_HOUR);
			lcdGotoxy(50, 70);
			lcdPuts("Wake up!\0");
			sdelay(1);
			lcdClrscr();
		}


		// TUTAJ TEORETYCZNIE POWINNA GRAC MUZYKA ALE JA ZA CHINY LUDOWE NIE OGARNIAM TEGO REJESTRU

		//Initialize DAC: AOUT = P0.25


		//set analogue output
		/*Bity 0:5 sa zarezerowwane
		* Bity 15:6 to VALUE
		* Bit 16 to BIAS
		* Bity 31-17 sa zarezerwowane
		* */
		tU8 it = 0;
		// to powinno grac sinusoidalny dzwiek (czy cos w ten desen) w nieskonczonosc
		while(TRUE){
			if(it == SINE_PERIOD_STEPS) it = 0;
			tU16 buffer = sineWave[it];
			PINSEL1 &= ~0x000C0000;
			PINSEL1 |=  0x00080000;
			DACR = (buffer << 6) |  //actual value to output
			(0 << 16);         //BIAS = 0, 1uS settling time
			it++;
			udelay(100);			// this function might not work yet (idk)
		}

		lcdGotoxy(0, 0);
		//messagePointer = strcpy(messageHolder, "Pomiar \temperatury: \n");
		lcdGotoxy(0, 30);
		measureTemperature(lm75address, readTemperature);
		lcdGotoxy(0, 0);
		lcdPuts("Pomiar\ntemperatury: ");
		lcdGotoxy(0, 30);
		calculateTemperatureValue(readTemperature);

		tU8 charArrray[10] = {0};
		tU16 humidity = measureHumidity();
		sprintf(charArray,"%d",humidity);
		lcdGotoxy(0,100);
		tU8 percent[2] = "%\0";
		lcdPuts(charArray);
		lcdGotoxy(20,100);
		lcdPuts(percent);


		// bez sensu jest ustawianie tych diod co chwile na 1, bo one i tak beda wlaczone
		// potrzebujemy funkcji ktora wlacza miganie i funkcje ktora wylacza
		blink();
    }
}

