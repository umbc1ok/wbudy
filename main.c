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






int main(void)
{
	// I2C initalization
	i2cInit();
	tU8 pca9532Present;
	pca9532Present = pca9532Init();
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
	tU8 temperature[2] = {0};


	RTC_SEC = 0;
	RTC_MIN = 0;
	RTC_HOUR = 0;
	tU8 temp_SEC = RTC_SEC;
	tU8 temp_MIN = RTC_MIN;
	tU8 temp_HOUR = RTC_HOUR;

	//IODIR |= 0x00008000; //P0.15




	while (TRUE) {
		showTime(RTC_SEC, RTC_MIN,RTC_HOUR,5);
        //check if P0.8 center-key is pressed
        if ((IOPIN & 0x00000800) == 0 && !isTimeBeingSet)//bylo 100
        {
            isTimeBeingSet = TRUE;
			temp_SEC = RTC_SEC;
			temp_MIN = RTC_MIN;
			temp_HOUR = RTC_HOUR;

			showTime(temp_SEC,temp_MIN,temp_HOUR,5);
			sdelay(1);
			while(isTimeBeingSet){
				//check if P0.8 center-key is pressed
				if ((IOPIN & 0x00000800) == 0) //bylo 100
				{
					currentType++;
					if(currentType<0 || currentType > 2){
						isTimeBeingSet = FALSE;
						setAlarm(temp_SEC,temp_MIN,temp_HOUR);
						currentType = 0;
						sdelay(1);
					}
					else{
						sdelay(1);
					}
				}
				//check if P0.10 up-key is pressed
				else if((IOPIN & 0x0000400) == 0){
					switch(currentType){
						case 0:
							if(temp_HOUR<23){
								temp_HOUR++;
							}
							break;
						case 1:
							if(temp_MIN < 60){
								temp_MIN++;
							}
							break;
						case 2:
							if(temp_SEC < 60){
								temp_SEC++;
							}
							break;
					}
				}
				//check if P0.12 down-key is pressed
				else if((IOPIN & 0x00001000) == 0){
					switch(currentType){
						case 0:
							if(temp_HOUR >0){
								temp_HOUR--;
							}
							break;
						case 1:
							if(temp_MIN > 0){
								temp_MIN--;
							}
							break;
						case 2:
							if(temp_SEC > 0){
								temp_SEC--;
							}
							break;
					}
				}
				showTime(temp_SEC,temp_MIN,temp_HOUR,currentType);
				mdelay(100);
			}
        }
        if((RTC_ILR & 0x2) == 0x2)
        {
			showTime(RTC_SEC,RTC_MIN,RTC_HOUR,5);
			lcdGotoxy(50, 90);
			lcdPuts("Wake up!\0");
			sdelay(1);
			blinkLeds(1, pca9532Present);
			beep(5);
			blinkLeds(0, pca9532Present);
			lcdClrscr();
			RTC_ILR = 0x2;
			RTC_AMR = 0xFF;
		}

		lcdGotoxy(0, 0);
		measureTemperature(lm75address, temperature);
		lcdGotoxy(0, 0);
		lcdPuts("Temperatura: \0");
		lcdGotoxy(0, 15);
		calculateTemperatureValue(temperature);

		tU8 charArrray[10] = {0};
		tU16 humidity = measureHumidity();
		sprintf(charArray,"%d",humidity);
		lcdGotoxy(0,30);
		lcdPuts("Wilgotnosc:");
		lcdGotoxy(0,45);
		tU8 percent[2] = "%\0";
		lcdPuts(charArray);
		lcdGotoxy(20,45);
		lcdPuts(percent);


    }
}

