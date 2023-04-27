/******************************************************************************
 *
 * Copyright:
 *    (C) 2005 Embedded Artists AB
 *
 * File:
 *    eepromTest.c
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "pre_emptive_os/api/general.h"
#include <lpc2xxx.h>
#include "startup/config.h"
#include <printf_P.h>
#include "eeprom.h"
#include <string.h>


/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/
#define MAX_LENGTH 14

/*****************************************************************************
 * Public function prototypes
 ****************************************************************************/

/*****************************************************************************
 *
 * Description:
 *    Test EEPROM
 *
 ****************************************************************************/
tU8
testEEPROM(void)
{
  tU8 eepromTestResultOK;
  tU8 testString1[] = "String #1";
  tU8 testString2[] = "sTrInG #2";
  tU8 testBuf[MAX_LENGTH];
  tS8 errorCode;
  
  eepromTestResultOK = TRUE;
//  printf("\nTest #1 - write string '%s' to address 0x0000", testString1);
  errorCode = eepromWrite(0x0000, testString1, sizeof(testString1));
  if (errorCode == I2C_CODE_OK)
;//    printf("\n        - done (status code OK)");
  else
  {
    printf("\nTest #1 - write string '%s' to address 0x0000", testString1);
    printf("\n        - failed (error code = %d)!", errorCode);
    eepromTestResultOK = FALSE;
  }
  
  if (eepromPoll() == I2C_CODE_OK)
;//    printf("\n        - program cycle completed");
  else
  {
    printf("\nTest #1 - write string '%s' to address 0x0000", testString1);
    printf("\n        - program cycle failed!");
    eepromTestResultOK = FALSE;
  }

//  printf("\nTest #2 - write string '%s' to address 0x0040", testString2);
  errorCode = eepromWrite(0x0040, testString2, sizeof(testString2));
  if (errorCode == I2C_CODE_OK)
;//    printf("\n        - done (status code OK)");
  else
  {
  	printf("\nTest #2 - write string '%s' to address 0x0040", testString2);
    printf("\n        - failed (error code = %d)!", errorCode);
    eepromTestResultOK = FALSE;
  }
   
  if (eepromPoll() == I2C_CODE_OK)
;//    printf("\n        - program cycle completed");
  else
  {
  	printf("\nTest #2 - write string '%s' to address 0x0040", testString2);
    printf("\n        - program cycle failed!");
    eepromTestResultOK = FALSE;
  }

  /*
   * Read from eeprom
   */
//  printf("\nTest #3 - read string from address 0x0000");
  errorCode = eepromPageRead(0x0000, testBuf, MAX_LENGTH);
  if (errorCode == I2C_CODE_OK)
  {
    if (strlen(testBuf) == sizeof(testString1)-1)
;//      printf("\n        - string is '%s'", testBuf);
    else
    {
    	printf("\nTest #3 - read string from address 0x0000");
      printf("\n        - wrong length (read string is %d characters long)! %d", strlen(testBuf),   sizeof(testString1)-1);

      printf("\n%d,%d,%d,%d,%d %d,%d,%d,%d,%d %d",
             testBuf[0],testBuf[1],testBuf[2],testBuf[3],testBuf[4],testBuf[5],testBuf[6],testBuf[7],testBuf[8],testBuf[9],testBuf[10]);
      printf("\n%c%c%c%c%c%c%c%c%c%c%c",
             testBuf[0],testBuf[1],testBuf[2],testBuf[3],testBuf[4],testBuf[5],testBuf[6],testBuf[7],testBuf[8],testBuf[9],testBuf[10]);

      eepromTestResultOK = FALSE;
    }
  }
  else
  {
  	printf("\nTest #3 - read string from address 0x0000");
    printf("\n        - failed (error code = %d)!", errorCode);
    eepromTestResultOK = FALSE;
  }

//  printf("\nTest #4 - read string from address 0x0040");
  errorCode = eepromPageRead(0x0040, testBuf, MAX_LENGTH);
  if (errorCode == I2C_CODE_OK)
  {
    if (strlen(testBuf) == sizeof(testString2)-1)
;//      printf("\n        - string is '%s'", testBuf);
    else
    {
    	printf("\nTest #4 - read string from address 0x0040");
      printf("\n        - wrong length (read string is %d characters long)!", strlen(testBuf));
      eepromTestResultOK = FALSE;
    }
  }
  else
  {
  	printf("\nTest #4 - read string from address 0x00a0");
    printf("\n        - failed (error code = %d)!", errorCode);
    eepromTestResultOK = FALSE;
  }

  /*
   * Write/Read from eeprom
   */
//  printf("\nTest #5 - write string '%s' to address 0x0004", testString2);
  errorCode = eepromWrite(0x0004, testString2, sizeof(testString2));
  if (errorCode == I2C_CODE_OK)
;//    printf("\n        - done (status code OK)");
  else
  {
  	printf("\nTest #5 - write string '%s' to address 0x0004", testString2);
    printf("\n        - failed (error code = %d)!", errorCode);
    eepromTestResultOK = FALSE;
  }
  
  if (eepromPoll() == I2C_CODE_OK)
;//    printf("\n        - program cycle completed");
  else
  {
  	printf("\nTest #5 - write string '%s' to address 0x0004", testString2);
    printf("\n        - program cycle failed!");
    eepromTestResultOK = FALSE;
  }

//  printf("\nTest #6 - read string from address 0x0000");
  errorCode = eepromPageRead(0x0000, testBuf, MAX_LENGTH);
  if (errorCode == I2C_CODE_OK)
  {
    if (strlen(testBuf) == sizeof(testString2)-1+4)
;//      printf("\n        - string is '%s'", testBuf);
    else
    {
    	printf("\nTest #6 - read string from address 0x0000");
      printf("\n        - wrong length (read string is %d characters long)!", strlen(testBuf));
      eepromTestResultOK = FALSE;
    }
  }
  else
  {
  	printf("\nTest #6 - read string from address 0x0000");
    printf("\n        - failed (error code = %d)!", errorCode);
    eepromTestResultOK = FALSE;
  }
  
  return eepromTestResultOK;
}

