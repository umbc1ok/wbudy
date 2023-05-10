#include "dac.h"
#include "startup/lpc2xxx.h"


void initDac(void){
	// Aktywuje P0.25. Bit 19-ty jest odpowiedzialny za Aout(DAC)
	// czyli wyj�cie analogowe przetwornik cyrfowo analogowego.
	// w��czenie zatem polega tylko na w��czeniu pinu
	PINSEL1 = 1<<19;
}
