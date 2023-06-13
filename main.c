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
#include "sound.h"
#include "rtc.h"
#include "lm75.h"
#include "htu21df.h"
#include "lcd.h"
#include "pca9532.h"
#include "additional.h"

#include "Common_Def.h"




tU8 init(void);

/*!
 *  @brief    Logika programu budzika.
 *  @returns  W praktyce nic, zawiera petle nieskonczona zawierajaca algorytm. 
 */
int main(void)
{
	tU8 pca9532Present;
	pca9532Present = init();

	tU8 lm75address = (((tU8)0x48 << 1) | (tU8)1);

	tU8 isTimeBeingSet = FALSE;
	tU8 currentType = 0;
	tU8 humidityString[10] = {0};
	tU8 temperature[2] = {0};
	
	tU8 temp_SEC = RTC_SEC;
	tU8 temp_MIN = RTC_MIN;
	tU8 temp_HOUR = RTC_HOUR;

	// Ponizsze linijki sa po to, zeby spelnic wymaganie MISRA.
	const tU8 tempPrompt[] = "Temperatura: \0";
	const tU8 tempUnitPrompt[] = "st. C \0";
	const tU8 humidityPrompt[] = "Wilgotnosc:\0";
	const tU8 percentPrompt[] = "%\0";
	const tU8 setAlarmPrompt[] = "R - ustaw budzik \0";
	const tU8 wakeUpPrompt[] = "Wake up!\0";
	const tU8 UPrompt[] = "U - zwieksz\0";
	const tU8 DPrompt[] = "D - zmniejsz\0";
	const tU8 RPrompt[] = "R - zatwierdz\0";


	while (TRUE == TRUE) { //TRUE == TRUE bo MISRA nie pozwala na while(TRUE)
		showTime(RTC_SEC, RTC_MIN,RTC_HOUR,5);
        //check if right key is pressed
        if ((IOPIN & 0x00000800) == 0)
        {
        	lcdClrscr();
            isTimeBeingSet = TRUE;
			temp_SEC = RTC_SEC;
			temp_MIN = RTC_MIN;
			temp_HOUR = RTC_HOUR;

			showTime(temp_SEC,temp_MIN,temp_HOUR,5);
			sdelay(1);
			while(isTimeBeingSet == (tU8)TRUE){
				/* check if right key is pressed
				*  Jesli wybieramy godziny, przechodzimy od ustawiania minut
				*  Jesli minuty to przechodzimy do sekund
				*  Jesli sekundy - ustawiamy alarm i wychodzimy z petli.
				*/
				if ((IOPIN & 0x00000800) == 0)
				{
					currentType++;
					if((currentType< (tU8)0) || (currentType > (tU8)2)){
						isTimeBeingSet = FALSE;
						setAlarm(temp_SEC,temp_MIN,temp_HOUR);
						currentType = 0;
						sdelay(1);
					}
					else{
						sdelay(1);
					}
				}
				/* check if up-key is pressed
				*  Zwiekszanie wartosci godzin/minut/sekund
				*/
				else if((IOPIN & 0x0000400) == 0){
					switch(currentType){
						case 0:
							if(temp_HOUR<(tU8)23){
								temp_HOUR++;
							}
							break;
						case 1:
							if(temp_MIN < (tU8)59){
								temp_MIN++;
							}
							break;
						case 2:
							if(temp_SEC < (tU8)59){
								temp_SEC++;
							}
							break;
						default:
							break;
					}
				}
				/* check if P0.12 down-key is pressed
				*  Zmniejszanie wartosci godzin/minut/sekund
				*/
				else if((IOPIN & 0x00001000) == 0){
					switch(currentType){
						case 0:
							if(temp_HOUR > (tU8)0){
								temp_HOUR--;
							}
							break;
						case 1:
							if(temp_MIN > (tU8)0){
								temp_MIN--;
							}
							break;
						case 2:
							if(temp_SEC > (tU8)0){
								temp_SEC--;
							}
							break;
						default:
							break;
					}
				}
				else{
					// do nothing
				}
				showTime(temp_SEC,temp_MIN,temp_HOUR,currentType);

				lcdGotoxy(0,0);
				lcdPuts(UPrompt);

				lcdGotoxy(0,15);
				lcdPuts(DPrompt);

				lcdGotoxy(0,30);
				lcdPuts(RPrompt);

				mdelay(100);
			}
			lcdClrscr();
        }
        if((RTC_ILR & 0x2) == 0x2)
        {
			showTime(RTC_SEC,RTC_MIN,RTC_HOUR,5);
			lcdGotoxy(50, 90);
			lcdPuts(wakeUpPrompt);
			sdelay(1);
			blinkLeds(1, pca9532Present);
			beep(5);
			blinkLeds(0, pca9532Present);
			lcdClrscr();
			RTC_ILR = 0x2;
			RTC_AMR = 0xFF;
		}

		lcdGotoxy(0, 0);
		tS8 i2cCode;
		i2cCode = measureTemperature(lm75address, temperature); // MISRA wymaga aby funkcje zwracajace jakas wartosc nie zwracaly jej w eter
		lcdGotoxy(0, 0);
		lcdPuts(tempPrompt);
		lcdGotoxy(0, 15);
		calculateTemperatureValue(temperature);
		lcdGotoxy(40,15);
		lcdPuts(tempUnitPrompt);

		tU16 humidity = measureHumidity();
		tU8 sprintfHolder;
		sprintfHolder = sprintf(humidityString,"%d",humidity);
		lcdGotoxy(0,30);
		lcdPuts(humidityPrompt);
		lcdGotoxy(0,45);
		lcdPuts(humidityString);
		lcdGotoxy(20,45);
		lcdPuts(percentPrompt);
		lcdGotoxy(0,105);
		lcdPuts(setAlarmPrompt);
    }
}
/*!
 *  @brief    Inicjalizacja I2C, PCA9532, RTC oraz GPIO.
 *
 *  @returns  TRUE jesli pca9532 zostalo poprawnie zainicjalizowane, FALSE jesli nie zostalo.
 */
tU8 init(void){
	i2cInit();
	tU8 pca9532Present;
	pca9532Present = pca9532Init();

	lcdInit();
	// Setting colors for background and text.
	lcdColor(0xff, 0x00);
	lcdClrscr();
	
	RTC_SEC = 0;
	RTC_MIN = 0;
	RTC_HOUR = 0;

	RTC_CCR  = 0x00000011;
	PINSEL0&=~0x3FF0000; //init GPIO for joystick

	RTC_ILR = 0x2;
	RTC_AMR = 0xFF;

	return pca9532Present;
}

