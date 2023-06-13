/*
    Plik, zawierający kod źródłowy funkcji
    pomocniczych.
*/

#include "additional.h"

/*
 * @brief   Funkcja udelay() służy do odmierzania podanego w mikrosekundach okresu czasu, po 
 *          upłynięciu którego funkcja zakończy działanie.           
 * @param   delayInUs  
 *          Długość okresu oczekiwania, podana w mikrosekundach. 
 * @returns void
 * 
 * @side effects: 
 *          Brak
 */
void udelay(tU32 delayInUs)
{

  T1TCR = 0x02; // zatrzymanie i reset zegara
  T1PR  = (CORE_FREQ / 1000000) -1; //57

  T1MR0 = delayInUs;
  T1IR  = 0xff; // zresetowanie wszystkich flag przerwań       
  T1MCR = 0x04; // zatrzyma inkrementowanie TC i PC oraz wyłączy timer1, gdy wartość rejestru MR0 == TC             
  T1TCR = 0x01; // start timera       
  
  // czekaj dopóki zadany czas nie upłynie
  while ((T1TCR & 0x01) == 1)
  {
    ;
  }
  T1TCR = 0x00; // zresetowanie wartości w rejestrze T1TCR, timer1 zostaje wyłączony
}


/*
 * @brief   Funkcja mdelay() służy do odmierzania podanego w milisekundach okresu czasu, po 
 *          upłynięciu którego funkcja zakończy działanie   
 * @param   delayInMs  
 *          Długość okresu oczekiwania, podana w milisekundach.
 * @returns void
 * 
 * @side effects: 
 *          Brak
 */

void mdelay(tU32 delayInMs)
{

  T1TCR = 0x02; // zatrzymanie i reset zegara                         
  T1PR  = (PERIPHERAL_CLOCK / 1000) - 1; //58 981
  T1MR0 = delayInMs;
  T1IR  = 0xFF; // zresetowanie wszystkich flag przerwań       
  T1MCR = 0x04; // zatrzyma inkrementowanie TC i PC oraz wyłączy timer1, gdy wartość rejestru MR0 == TC    
  T1TCR = 0x01; // start timera

  // czekaj dopóki zadany czas nie upłynie         
  while ((T1TCR & 0x01) == 1)
  {
    ;
  }
  T1TCR = 0x00; // zresetowanie wartości w rejestrze T1TCR, timer1 zostaje wyłączony        
}


/*
 * @brief   Funkcja sdelay() służy do odmierzania podanego w sekundach okresu czasu, po 
 *          upłynięciu którego funkcja zakończy działanie.     
 * @param   delayInS 
 *          Długość okresu oczekiwania, podana w sekundach.
 * @returns void    
 * 
 * @side effects: 
 *          Brak
 */

void sdelay(tU32 delayInS)
{

  T1TCR = 0x02; // zatrzymanie i reset zegara
  T1PR  = PERIPHERAL_CLOCK - 1; //58 982 399    
  T1MR0 = delayInS;
  T1IR = 0xff;  // zresetowanie wszystkich flag przerwań
  T1MCR = 0x04; // zatrzyma inkrementowanie TC i PC oraz wyłączy timer1, gdy wartość rejestru MR0 == TC
  T1TCR = 0x01; // start timera

  // czekaj dopóki zadany czas nie upłynie
  while ((T1TCR & 0x01) == 1)
  {
    ;
  }
  T1TCR = 0x00; // zresetowanie wartości w rejestrze T1TCR, timer1 zostaje wyłączony
}
