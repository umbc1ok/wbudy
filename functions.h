#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include "inttypes.h"
#include "pff2/src/pff.h"

typedef enum { //okre�la stan odtwarzacza
	Player_Stoped = 0,
	Player_Playing,
	Player_Paused,
	Player_Prev,
	Player_Next,
	Player_Random
} PlayerStatus ;

typedef enum { //okre�la metode prezentacji czasu odtwarzania
	Timer_UP = 0, //odliczanie ile mine�o
	Timer_DOWN //odliczanie ile pozosta�o
} TimerStatus;

#define JOYCENTER 0x00000100 //P0.8
#define JOYLEFT 0x00000200 //P0.9
#define JOYUP 0x00000400 //P0.10
#define JOYRIGHT 0x00000800 //P0.11
#define JOYDOWN 0x00001000  //P0.12

//inicjuj akcelerometr
void initAccel();

//sprawdza czy okre�lony przycisk jest wci�ni�ty
tBool checkButton(uint8 button);
//sprawdza czy d�ojstik jest "wci�ni�ty"
tBool checkJoystic(uint32 key);
//sprawdza czy przy�pieszenie osi�gne�o warto�� przy kt�rej uznajemy potrz�sanie
tBool checkAccel();

//sprawdza wszystkie zmienne i we zmienia status
void updateStatus(PlayerStatus *status, TimerStatus *timer, tBool delay);

//op�nienie proste nie oparte na zegarze
void delayT (uint32 val);

//wypisuje na terminalu, pe�n� zawarto�� karty
FRESULT listDir(char *path, tBool first);

//tworzy list� plik�w dla otwarzacza
uint32 filesList(char* path, char* list);

#endif
