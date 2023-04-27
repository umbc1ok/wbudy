/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 * Description:
 *    Christmas tree
 *
 *****************************************************************************/

#ifndef _ADC_H_
#define _ADC_H_

#include "../pre_emptive_os/api/general.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

#define AIN0 0
#define AIN1 1
#define AIN2 2
#define AIN3 3
#define AIN4 4
#define AIN5 5
#define AIN6 6

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
tU16 getAnalogueInput(tU8 channel);

/*****************************************************************************
 *
 * Description:
 *
 ****************************************************************************/
void initAdc(void);

#endif
