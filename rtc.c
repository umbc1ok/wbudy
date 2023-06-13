#include "rtc.h"

void showTime(tU8 sec,tU8 min,tU8 hour, tU8 highlight){

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

    if(highlight == 0){
    	lcdColor(0x00, 0xff);
    }
    lcdGotoxy(10, 70);
    lcdPuts(tabHr);
    lcdColor(0xff, 0x00);

    lcdGotoxy(30, 70);
    lcdPuts(":");

    if(highlight == 1){
        lcdColor(0x00, 0xff);
    }
    lcdGotoxy(50, 70);
    lcdPuts(tabMin);
    lcdColor(0xff, 0x00);


    lcdGotoxy(70, 70);
    lcdPuts(":");
    if(highlight == 2){
    	lcdColor(0x00, 0xff);
    }
    lcdGotoxy(90, 70);
    lcdPuts(tabS);
    lcdColor(0xff, 0x00);
}


void setAlarm(tU8 sec, tU8 min, tU8 hour){
	RTC_ALSEC = sec;
	RTC_ALMIN = min;
	RTC_ALHOUR = hour;
	RTC_AMR = 0xF8;
}