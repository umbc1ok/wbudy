/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 * Description:
 *    Christmas tree
 *
 *****************************************************************************/

#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <lpc2xxx.h>
#include <config.h>
#include "adc.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/
#define CRYSTAL_FREQUENCY FOSC
#define PLL_FACTOR        PLL_MUL
#define VPBDIV_FACTOR     PBSD


/*****************************************************************************
 *
 * Description:
 *    Start a conversion of one selected analogue input and return
 *    10-bit result.
 *
 * Params:
 *    [in] channel - analogue input channel to convert.
 *
 * Return:
 *    10-bit conversion result
 *
 ****************************************************************************/
tU16
getAnalogueInput(tU8 channel)
{
//  volatile tU32 cpsrReg;
  tU16 returnResult;

  //disable IRQ
//  cpsrReg = disIrq();

	//start conversion now (for selected channel)
	ADCR = (ADCR & 0xFFFFFF00) | (1 << channel) | (1 << 24);
	
	//wait til done
	while((ADDR & 0x80000000) == 0)
	  ;

	//get result and adjust to 10-bit integer
	returnResult = (ADDR>>6) & 0x3FF;

  //enable IRQ
//  restoreIrq(cpsrReg);
  
  return returnResult;
}

/*****************************************************************************
 *
 * Description:
 *
 ****************************************************************************/
void
initAdc(void)
{
	volatile tU32 integerResult;

	//
  //Initialize ADC: AIN0 = P0.22
  //
//  PINSEL1 &= ~0x00003000;
//  PINSEL1 |=  0x00003000;
  
	//
  //Initialize ADC: AIN1 = P0.28
  //
  PINSEL1 &= ~0x03000000;
  PINSEL1 |=  0x01000000;

	//
  //Initialize ADC: AIN2 = P0.29
  //
  PINSEL1 &= ~0x0C000000;
  PINSEL1 |=  0x04000000;

	//
  //Initialize ADC: AIN3 = P0.30
  //
//  PINSEL1 &= ~0x30000000;
//  PINSEL1 |=  0x10000000;
  
	//
  //Initialize ADC: AIN14 = P0.21
  //
//  PINSEL1 &= ~0x00030000;
//  PINSEL1 |=  0x00030000;

	//
  //Initialize ADC: AIN15 = P0.22
  //
//  PINSEL1 &= ~0x000C0000;
//  PINSEL1 |=  0x000C0000;

  //initialize ADC
  ADCR = (1 << 0)                             |  //SEL = 1, dummy channel #0
         ((CRYSTAL_FREQUENCY *
           PLL_FACTOR /
           VPBDIV_FACTOR) / 4500000 - 1) << 8 |  //set clock division factor, so ADC clock is 4.5MHz
         (0 << 16)                            |  //BURST = 0, conversions are SW controlled
         (0 << 17)                            |  //CLKS  = 0, 11 clocks = 10-bit result
         (1 << 21)                            |  //PDN   = 1, ADC is active
         (1 << 24)                            |  //START = 1, start a conversion now
         (0 << 27);							                 //EDGE  = 0, not relevant when start=1

  //short delay and dummy read
  osSleep(1);
  integerResult = ADDR;
}

