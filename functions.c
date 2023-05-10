#include "functions.h"
#include "startup/printf_P.h"
#include "startup/lpc2xxx.h"
#include <string.h>
#include "timer.h"
#include "led.h"
#include "lcd.h"
#include "adc.h"

int16 Xvalue, refXvalue;
int16 Yvalue, refYvalue;
int16 Zvalue, refZvalue;

void initAccel(){
	//zczytujemy wartoœci pocz¹tkowe (punkt odniesienia)
	refXvalue = getAnalogueInput1(AIN6);
	refYvalue = getAnalogueInput1(AIN7);
	refZvalue = getAnalogueInput0(AIN3);
	printf("\nx=%d, y=%d, z=%d", refXvalue, refYvalue, refZvalue);
}

tBool checkButton(uint8 button){
	//przycisku numerujemy od 0 do 4
	if( button != 4 ){
		if( (IO1PIN & (1 << (20 + button) )) == 0 )
			return TRUE;
	}else{
		if( (IO0PIN & (1 << (10 + button) )) == 0 )
			return TRUE;
	}

	return FALSE;
}

tBool checkJoystic( uint32 key ){
	if( (IOPIN & key) == 0 )
		return TRUE;

	return FALSE;
}


tBool checkAccel(){
	//pobierz wychylenie na osi Y
	Yvalue = getAnalogueInput1(AIN7);
	if( Yvalue-refYvalue >= 100 || Yvalue-refYvalue <= -100 ){
		//jeœli jest odpowiednie du¿e, odczekaj chwile i sprawdŸ ponownie
		zegarek(10000);
		Yvalue = getAnalogueInput1(AIN7);
		if( Yvalue-refYvalue >= 100 || Yvalue-refYvalue <= -100 ){
			//jeœli jest ponownie du¿e, ozacza to ¿e raczej potrz¹sneliœmy
			return TRUE;
		}
	}

	//pobierz wychylenie na osi X
	Xvalue = getAnalogueInput1(AIN6);
	if( Xvalue-refXvalue >= 100 || Xvalue-refXvalue <= -100 ){
		zegarek(10000);
		Yvalue = getAnalogueInput1(AIN7);
		if( Xvalue-refXvalue >= 100 || Xvalue-refXvalue <= -100 ){
			return TRUE;
		}
	}

	return FALSE;
}

void updateStatus(PlayerStatus *status, TimerStatus *timer, tBool delay){
	if( checkAccel() ){
		*status = Player_Random;
		goto END;
	}

	if( checkButton(0) ){
		turnOnLed(0);
		*status = Player_Prev;
		goto END;
	}

	if( checkButton(1) ){
		turnOnLed(1);
		*status = Player_Playing;
		lcdGotoxy(15*8,0);
		lcdPuts(">");
		goto END;
	}

	if( checkButton(2) ){
		turnOnLed(2);
		*status = Player_Paused;
		lcdGotoxy(15*8,0);
		lcdPuts("|");
		goto END;
	}

	if( checkButton(3) ){
		turnOnLed(3);
		*status = Player_Stoped;
		goto END;
	}

	if( checkButton(4) ){
		//turnOnLed(4); //zbêdne ona sama siê zapala
		*status = Player_Next;
		goto END;
	}

	if( checkJoystic(JOYUP) ){
		*timer = Timer_UP;
		goto END;
	}

	if( checkJoystic(JOYDOWN) ){
		*timer = Timer_DOWN;
		lcdRect(0,32,9*8,16, 0xff);
		goto END;
	}

	if( checkJoystic(JOYLEFT) ){
		turnOnLed(0);
		*status = Player_Prev;
		goto END;
	}

	if( checkJoystic(JOYRIGHT) ){
		turnOnLed(4);
		*status = Player_Next;
		goto END;
	}

	if( checkJoystic(JOYCENTER) ){
		if( *status == Player_Playing ){
			turnOnLed(3);
			*status = Player_Stoped;
		}else{
			turnOnLed(1);
			*status = Player_Playing;
		}
		goto END;
	}

	END: //czekaj przez zakoñczeniem, zabezpiecza nas to przed powtarzaniem
	//wiele razy tej procedury, gdy¿ ona siê w miarê szybko wykonuje
	if( delay ||
		(delay == FALSE && (*status == Player_Next || *status == Player_Prev
							|| *status == Player_Stoped ||
							*status == Player_Random  ) )
	  )
	{
		zegarek(125000);
	}

	//zgaœ zapalone diody
	if( *status == Player_Prev )
		turnOffLed(0);
	else if( *status == Player_Playing )
		turnOffLed(1);
	else if( *status == Player_Paused )
		turnOffLed(2);
	else if( *status == Player_Stoped )
		turnOffLed(3);
	else if( *status == Player_Next )
		turnOffLed(4);
}

void delayT (uint32 val)  {                   //definicja funkcji opóŸniaj¹cej
  uint32 i;   //deklaracja zmiennej, która posluzy jako licznik petli
  for(i = 0; i <= val; i++){};                //opóŸnienie czasowe
}

FRESULT listDir(char* path, tBool first){
	FRESULT res;
	FILINFO fno;
	DIR dir;
	int i;

	if(first == TRUE){
		printf("D-Directory R-Read_Only H-Hidden S-System A-Archive\n");
		printf("DRHSA path/fileName\n");
		printf("-------------------------------------------------------\n");
	}

	res = pf_opendir(&dir, path);
	if (res == FR_OK) {
		i = strlen(path);
		for (;;) {
			res = pf_readdir(&dir, &fno);
			if (res != FR_OK || fno.fname[0] == 0) break;
			if (fno.fattrib & AM_DIR) {
				char tmp[strlen(path) + 1 + strlen(fno.fname)];
				strcpy(tmp, path);
				if( path[1]!= "/" && path[1]!= 0 )
					strncat(tmp, "/", 1);
				strncat(tmp, fno.fname, strlen(fno.fname));

				printf("D---- %s\n",tmp);
				res = listDir(tmp, FALSE); //rekursywne przeszukiwanie katalogów
				if (res != FR_OK) break;
			} else { //wypisz atrybuty
				printf("-");
				if(fno.fattrib & AM_RDO)
					printf("R");
				else
					printf("-");

				if(fno.fattrib & AM_HID)
					printf("H");
				else
					printf("-");

				if(fno.fattrib & AM_SYS)
					printf("S");
				else
					printf("-");

				if(fno.fattrib & AM_ARC)
					printf("A");
				else
					printf("-");

				if( path[0]!= "/" && i != 1)
					printf(" %s/%s\n", path, fno.fname);
				else
					printf(" %s%s\n", path, fno.fname);
			}
		}
	}

	return res;
}

uint32 filesList(char* path, char* list){
	FRESULT res;
	FILINFO fno;
	DIR dir;
	uint32 i=0;

	res = pf_opendir(&dir, path);
	if (res == FR_OK) {
		for (;;) {
			res = pf_readdir(&dir, &fno);
			if (res != FR_OK || fno.fname[0] == 0) break;
			if (fno.fattrib & AM_DIR) {
				continue;
			} else {
				if( fno.fname[9] != 'W' || fno.fname[10] != 'A' ||
					fno.fname[11] != 'V' )
					continue;
				if( i >= 256)
					break;
				char *p = &list[i * 12];
				strncpy(p, fno.fname,12);
				i++;
			}
		}
	}
	return i;
}
