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
int main(void)
{
	tU8 pca9532Present = FALSE;
	// I2C initalization
	i2cInit();
	pca9532Present = pca9532Init();
	tU8 lm75address = (((tU8)0x48 << 1) | (tU8)1);
	printf_init();
	// LCD display initialization
	lcdInit();
	// Setting colors for background and text.
	lcdColor(0xff, 0x00);
	lcdClrscr();

	/*
		Main loop containg enitre program logic. With every iteration, joystick position is checked
		and based on that, appropriate measurement value.
	*/
	/*
	tU8 *charPtr;
	tS8 returnedValue = 0;
	tU8 charArray[10] = {0};
	// Variables for holding messages to display with lcdPuts
	tU8 *messagePointer;
	tU8 messageHolder[128];

	tS8 humidityValue;
	tU16 brightnessValue;
	tS64 pressureValue; */

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
	tU32 temp_SEC = RTC_SEC;
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
		tU16 keys;
			  if (TRUE == pca9532Present)
			  {
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
			    if (!(keys & KEY_RIGHT)) setPca9532Pin(15, 0);
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
		      keys = ~getPca9532Pin();

				// TUTAJ TEORETYCZNIE POWINNA GRAC MUZYKA ALE JA ZA CHINY LUDOWE NIE OGARNIAM TEGO REJESTRU
			  
				//Initialize DAC: AOUT = P0.25

				
				//set analogue output
				/*Bity 0:5 sa zarezerowwane
				* Bity 15:6 to VALUE
				* Bit 16 to BIAS
				* Bity 31-17 sa zarezerwowane
				* */
				/*
				tU16 buffer = 0;
				PINSEL1 &= ~0x000C0000;
				PINSEL1 |=  0x00080000;
			  DACR = (buffer << 6) |  //actual value to output
               (1 << 16);         //BIAS = 1, 2.5uS settling time
			   */
			   
			  }
			  lcdGotoxy(0, 0);
				//messagePointer = strcpy(messageHolder, "Pomiar \temperatury: \n");
				lcdGotoxy(0, 30);
				measureTemperature(lm75address, readTemperature);
				lcdGotoxy(0, 0);
				lcdPuts("Pomiar\ntemperatury: ");
				lcdGotoxy(0, 30);
				calculateTemperatureValue(readTemperature);
    }
}


/******************************************************************************
 *
 * Copyright:
 *    (C) 2008 Embedded Artists AB
 *
 * Description:
 *    Main program for I2C_LEDs_KEYs demo
 *
 *****************************************************************************/

/*
#include <printf_P.h>
#include <lpc2xxx.h>
#include <consol.h>
#include "i2c.h"
#include "pca9532.h"*/
