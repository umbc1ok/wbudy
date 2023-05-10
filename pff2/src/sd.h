#ifndef _SD_H
#define _SD_H

#include "integer.h"

#define	CMDREAD		17
#define	CMDWRITE	24
#define	CMDREADCSD	9

//inicjuj czytnik
CHAR sdInit(void);
//sprawd� system plik�w
CHAR sdState(void);
//pozwala na odczyt rejestru z karty i np pobranie wielko�ci karty
//z moj� to i tak nie dzia�a ;)
//CHAR sdgetDriveSize(void);
//wy�lij komende
void sdCommand(BYTE cmd, WORD paramx, WORD paramy);

//odpowied� 8bitowa
BYTE sdResp8b(void);
//b��d je�li nast�pi
void sdResp8bError(BYTE value);
//odpowied� 16 bitowa
WORD sdResp16b(void);


#endif //_SD_H
