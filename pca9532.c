/******************************************************************************
 *
 * Copyright:
 *    (C) 2006 Embedded Artists AB
 *
 * File:
 *    pca9532.c
 *
 * Description:
 *    Implements hardware specific routines
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "general.h"
#include <lpc2xxx.h>
#include <printf_P.h>
#include "i2c.h"
#include "pca9532.h"
#include "eeprom.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/


/*****************************************************************************
 * Global variables
 ****************************************************************************/

/*****************************************************************************
 * Local variables
 ****************************************************************************/

/*****************************************************************************
 * Local prototypes
 ****************************************************************************/


/*****************************************************************************
 *
 * Description:
 *    Initialize the io-pins and find out if HW is ver 1.0 or ver 1.1
 *
 ****************************************************************************/
tBool
pca9532Init(void)
{
  tU8 initCommand[] = {0x12, 0x97, 0x80, 0x4B, 0xC0, 0x00, 0x00, 0x00, 0x00};
  //                                                         04 = LCD_RST# low
  //                                                         10 = BT_RST# low

  //initialize PCA9532
  tU8 returnedValue = FALSE;
  if (I2C_CODE_OK == pca9532(initCommand, sizeof(initCommand), NULL, 0))
  {
    returnedValue = TRUE;
  }
  return returnedValue;
}


/*****************************************************************************
 *
 * Description:
 *
 ****************************************************************************/
void
setPca9532Pin(tU8 pinNum, tU8 value)
{
  tU8 command[] = {0x00, 0x00};
  tU8 regValue;
  tU8 mask;
  
  if (pinNum < (tU8)4) 
  {
    command[0] = 0x06;
  }
  else if (pinNum < (tU8)8) 
  {
    command[0] = 0x07;
  }
  else if (pinNum < (tU8)12) 
  {
    command[0] = 0x08;
  }
  else
  {
    command[0] = 0x09;
  }
    
  pca9532(command, 1, &regValue, 1);
  
  mask = ((tU8)3 << ((tU8)2 * (tU8)(pinNum % (tU8)4)));
  
  regValue &= ~mask;
  
  if (value == (tU8)0)
  {
    // gdy wartość = 0 - wyłączamy diodę
    command[1] = 0x00;
  }
  else if (value == (tU8)1)
  {
    // gdy wartość = 1 - włączamy diodę, która używa szybkość mrugania PWM0 
    command[1] = 0x02;
  }
    else if (value == (tU8)2)
  {
    //gdy wartość = 2 - włączamy diodę, która używa szybkość mrugania PWM1 
    command[1] = 0x03;
  }
  else
  {
    // gdy inna wartość =  włączamy diodę, która po prostu się świeci
    command[1] = 0x01; 
  }
    
  command[1] = (command[1] << ((tU8)2 * (pinNum % (tU8)4)));
  
  command[1] |= regValue;

  pca9532(command, sizeof(command), NULL, 0);
}


/*****************************************************************************
 *
 * Description:
 *
 ****************************************************************************/
tU16
getPca9532Pin(void)
{
  tU8 command[] = {0x19};
  tU8 regValue[3];
  
  pca9532(command, 1, regValue, 3);
  
  return (tU16)regValue[1] | ((tU16)regValue[2] << 8);
}


/*
 * @brief   Funckja blinkLeds wykorzystywana jest do kontrolwoania 16 LED-ów 0 - 15 (o numerach od 1 do 16). Działanie tej funckji
 *			przy wartości parametru value == 1 polega na ustawieniu zewnętrznych LED-ów, tj. LED numer 1, 8, 9 oraz 16
 *			na mruganie z szybkością PWM0 - diody te mrugają co 1 sekunde z mniejszą jasnością niż pozostałe LEDy,
 *			które mrugają z szybkością PWM1 co 0.5 sekundy. Gdy parametr value == 0, wyłączamy wszystkie LEDy.
 *
 * @param   value 
 *          Zmienna mówiąca czy włączyć czy wyłączyc diody
 * 
 * @param   pca9532Present 
 *          Zmienna sprawdzająca czy PCA9532 zostało zainicjalizowane.
 * @returns
 *          void
 * 
 * @side effects: 
 *          !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */
void 
blinkLeds(tU8 value, tU8 pca9532Present)
{
  if ((tU8)TRUE == pca9532Present)
  {
	tU8 i = 0;
    tU8 j = 0;
    if (value == (tU8)1)
    {
    	for (i = (tU8)0; i < (tU8)16; i = i + (tU8)1)
    	{
			if(i == (tU8)0 || i == (tU8)7 || i == (tU8)8 || i == (tU8)15)
			{
			setPca9532Pin(i, 1);
			}
			else 
			{
			setPca9532Pin(i, 2);
			}
    	}
    }
    else 
    {
		for (i = (tU8)0; i < (tU8)16; i = i + (tU8)1)
		{
			setPca9532Pin(i, 0);
		}
    }
  }
}



