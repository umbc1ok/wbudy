/*
    Plik, zawierający kod źródłowy funkcji
    pomocniczych.
*/

#include "additional.h"


void udelay(tU32 delayInUs ) //bigdemo
{
  /*
   * setup timer #1 for delay
   */
  T1TCR = 0x02;          //stop and reset timer
  T1PR  = (CORE_FREQ / PBSD / 1000000) -1;          //set prescaler to zero
  T1MR0 = delayInUs;
  T1IR  = 0xff;          //reset all interrrupt flags
  T1MCR = 0x04;          //stop timer on match
  T1TCR = 0x01;          //start timer
  
  //wait until delay time has elapsed
  while ((T1TCR & 0x01) == 1)
  {
    ;
  }
  T1TCR = 0x00;
}
/******************************************************************************
** Function name:		mdelay
**
** Descriptions:
**
** parameters:			delay length
** Returned value:		None
**
******************************************************************************/

void mdelay(tU32 delayInMs)
{

  T1TCR = 0x02;                            
  T1PR  = (PERIPHERAL_CLOCK / 1000) - 1;   
  T1MR0 = delayInMs;
  T1IR  = 0xFF;          
  T1MCR = 0x04;          
  T1TCR = 0x01;          
  while ((T1TCR & 0x01) == 1)
  {
    ;
  }
  T1TCR = 0x00;         
}




void sdelay(tU32 delayInS)
{
  /*
   * setup timer #1 for delay
   */
  T1TCR = 0x02; // stop and reset timer
  T1PR = PERIPHERAL_CLOCK -1;  // set prescaler to zero
  T1MR0 = delayInS;
  T1IR = 0xff;  // reset all interrrupt flags
  T1MCR = 0x04; // stop timer on match
  T1TCR = 0x01; // start timer

  // wait until delay time has elapsed
  while ((T1TCR & 0x01) == 1)
  {
    ;
  }
  T1TCR = 0x00;
}
