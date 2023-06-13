#include "rtc.h"

/*!
 *  @brief    Wyswietla zadany czas (godziny,minuty,sekundy) i podswietla wybrane pole czasu.
 *  @param sec
 *             Sekunda do wyswietlenia.
 *  @param min
 *             Sekunda do wyswietlenia.
 *  @param hour
 *             Sekunda do wyswietlenia.
 *  @param highlight
 *             Okresla wybrane pole czasu, ktore ma byc podswietlone,
 *             0 - godziny, 1 - minuty, 2 - sekundy, inna wartosc - brak podswietlenia
 *  @side effects:
 *              Zmiana pozycji wskaznika lcd w srodku funkcji. Po zakonczeniu dzialania jest on w pozycji x = 90, y = 70.
 */
void showTime(tU8 sec,tU8 min,tU8 hour, tU8 highlight){
	tU8 sprintfHolder;
	tU8 tabS[3];
	sprintfHolder = sprintf(tabS, "%d", sec);
    tU8 tabMin[3];
    sprintfHolder = sprintf(tabMin, "%d", min);
    tU8 tabHr[3];
    sprintfHolder = sprintf(tabHr, "%d", hour);

    tabS[2] = '\0';
    tabMin[2] = '\0';
    tabHr[2] = '\0';

    if (sec < (tU8)10) {
        tU8 temp1 = tabS[0];
        tabS[1] = temp1;
        tabS[0] = '0';
    }
    if (min < (tU8)10) {
        tU8 temp2 = tabMin[0];
        tabMin[1] = temp2;
        tabMin[0] = '0';
    }
    if (hour < (tU8)10) {
        tU8 temp3 = tabHr[0];
        tabHr[1] = temp3;
        tabHr[0] = '0';
    }

    if(highlight == (tU8)0){
    	lcdColor(0x00, 0xff);
    }
    lcdGotoxy(10, 70);
    lcdPuts(tabHr);
    lcdColor(0xff, 0x00);

    lcdGotoxy(30, 70);
    lcdPuts(":");

    if(highlight == (tU8)1){
        lcdColor(0x00, 0xff);
    }
    lcdGotoxy(50, 70);
    lcdPuts(tabMin);
    lcdColor(0xff, 0x00);


    lcdGotoxy(70, 70);
    lcdPuts(":");
    if(highlight == (tU8)2){
    	lcdColor(0x00, 0xff);
    }
    lcdGotoxy(90, 70);
    lcdPuts(tabS);
    lcdColor(0xff, 0x00);
}

/*!
 *  @brief    Wlacza alarm o okreslonym czasie (tzn. ustawia rejestry alarmu oraz AMR)
 *  @param sec
 *             Sekunda alarmu.
 *  @param min
 *             Minuta alarmu.
 *  @param hour
 *              Godzina alarmu.
 */
void setAlarm(tU8 sec, tU8 min, tU8 hour){
	RTC_ALSEC = sec;
	RTC_ALMIN = min;
	RTC_ALHOUR = hour;
	RTC_AMR = 0xF8;
}
