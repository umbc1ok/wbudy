/*
    Plik, zawierający kod źródłowy funkcji
    pomocniczych.
*/

#include "additional.h"

/******************************************************************************
** Function name:		mdelay
**
** Descriptions:
**
** parameters:			delay length
** Returned value:		None
**
******************************************************************************/

void udelay(tU32 delayInUs ) //bigdemo
{
  /*
   * setup timer #1 for delay
   */
  T1TCR = 0x02;          //stop and reset timer
  T1PR  = 0x00;          //set prescaler to zero
  T1MR0 = (((long)delayInUs-1) * (long)CORE_FREQ/1000) / 1000;
  T1IR  = 0xff;          //reset all interrrupt flags
  T1MCR = 0x04;          //stop timer on match
  T1TCR = 0x01;          //start timer
  
  //wait until delay time has elapsed
  while (T1TCR & 0x01)
    ;
}
/*
void udelay(tU32 delayInuS) //not sure if that works
{

  T1TCR = 0x02;                            
  T1PR  = (PERIPHERAL_CLOCK / 1000000000) - 1;   // 1 * 10^9 to jest wartosc wieksza niz wartosc w PERIPHERAL_CLOCK
                                                // Nie wiem jakim cudem mam tu inaczej wykombinowac
                                                // nano sekundy, ale okej

  T1MR0 = delayInuS;

  T1IR  = 0xFF;          
  T1MCR = 0x04;          
  T1TCR = 0x01;          
  while ((T1TCR & 0x01) == 1)
  {
    ;
  }
  T1TCR = 0x00;         
}*/
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

void sdelay(unsigned int delayInS)
{
  /*
   * setup timer #1 for delay
   */
  T1TCR = 0x02; // stop and reset timer
  T1PR = 0x00;  // set prescaler to zero
  T1MR0 = (((long)delayInS) * (long)CORE_FREQ);
  T1IR = 0xff;  // reset all interrrupt flags
  T1MCR = 0x04; // stop timer on match
  T1TCR = 0x01; // start timer

  // wait until delay time has elapsed
  while ((T1TCR & 0x01) != 0)
  {
    ;
  }
}
