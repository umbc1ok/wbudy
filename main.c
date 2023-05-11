#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>
#include <lpc2xxx.h>
#include <consol.h>
#include "i2c.h"
#include "adc.h"
#include "lcd.h"
#include "pca9532.h"
#include "ea_97x60c.h"
#include "timer.h"

#include <lpc2xxx.h>
#include <printf_P.h>
#include <consol.h>
#include <config.h>
#include "irq/irq_handler.h"
#include "timer.h"
#include "VIC.h"
#include "Common_Def.h"


#define PROC1_STACK_SIZE 1024
#define PROC2_STACK_SIZE 1024
#define INIT_STACK_SIZE  400

static tU8 proc1Stack[PROC1_STACK_SIZE];
static tU8 proc2Stack[PROC2_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];
static tU8 pid1;
static tU8 pid2;

static void proc1(void* arg);
static void proc2(void* arg);
static void initProc(void* arg);

volatile tU32 msClock;
volatile tU8 killProc1 = FALSE;
volatile tU8 rgbSpeed = 10;

/******************************************************************************
 ** Function name:		udelay
 **
 ** Descriptions:
 **
 ** parameters:			delay length
 ** Returned value:		None
 **
 ******************************************************************************/
void udelay(unsigned int delayInUs) {
	/*
	 * setup timer #1 for delay
	 */
	T1TCR = 0x02; //stop and reset timer
	T1PR = 0x00; //set prescaler to zero
	T1MR0 = (((long) delayInUs - 1) * (long) CORE_FREQ / 1000) / 1000;
	T1IR = 0xff; //reset all interrrupt flags
	T1MCR = 0x04; //stop timer on match
	T1TCR = 0x01; //start timer

	//wait until delay time has elapsed
	while (T1TCR & 0x01)
		;
}
/*****************************************************************************
 *
 * Description:
 *    The first function to execute 
 *
 ****************************************************************************/
int main(void) {
	tU8 error;
	tU8 pid;

	osInit();
	osCreateProcess(initProc, initStack, INIT_STACK_SIZE, &pid, 1, NULL, &error);
	osStartProcess(pid, &error);

	osStart();
	return 0;
}

/*****************************************************************************
 *
 * Description:
 *    A process entry function 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void proc1(void* arg) {
	printf("\n\n\n\n\n*******************************************************\n");
	printf("*                                                     *\n");
	printf("* This is the test program for                        *\n");
	printf("* LPC2138 Education Board v1.1 (2009-05-06).          *\n");
	printf("*                                                     *\n");
	printf("* (C) Embedded Artists 2009                           *\n");
	printf("*                                                     *\n");
	printf("*******************************************************\n");

	IODIR |= 0x00008000; //P0.15

	IODIR |= 0x00260000; //RGB
	IOSET = 0x00260000;

	IODIR1 |= 0x000F0000; //LEDs
	IOSET1 = 0x000F0000;
	osSleep(25);
	IOCLR1 = 0x00030000;
	osSleep(20);
	IOCLR1 = 0x00050000;
	osSleep(15);
	IOCLR1 = 0x000c0000;
	osSleep(10);
	IOCLR1 = 0x00090000;
	osSleep(5);
	IOSET1 = 0x000F0000;
	IODIR1 &= ~0x00F00000; //Keys

	for (;;) {
		static tU8 cnt;
		tU8 rxChar;

		//detect if P0.14 key is pressed
		if ((IOPIN & 0x00004000) == 0) {
			printf("\nP0.14");
		}

		//detect if P1.20 key is pressed
		if ((IOPIN1 & 0x00100000) == 0)
			IOCLR1 = 0x00020000;
		else
			IOSET1 = 0x00020000;

		//detect if P1.21 key is pressed
		if ((IOPIN1 & 0x00200000) == 0)
			IOCLR1 = 0x00010000;
		else
			IOSET1 = 0x00010000;

		//detect if P1.22 key is pressed
		if ((IOPIN1 & 0x00400000) == 0)
			IOCLR1 = 0x00040000;
		else
			IOSET1 = 0x00040000;

		//detect if P1.23 key is pressed
		if ((IOPIN1 & 0x00800000) == 0)
			IOCLR1 = 0x00080000;
		else
			IOSET1 = 0x00080000;

		cnt++;
		if ((cnt % rgbSpeed) == 0) {
			IOSET = 0x00260000;
			if (cnt == rgbSpeed)
				IOCLR = 0x00020000;
			else if (cnt == (2 * rgbSpeed))
				IOCLR = 0x00040000;
			else {
				IOCLR = 0x00200000;
				cnt = 0;
			}
		}

		//echo terminal
		if (TRUE == consolGetChar(&rxChar))
			consolSendCh(rxChar);

		osSleep(5);

		if (TRUE == killProc1) {
			printf("\nProc #1 kill itself!!!\n");
			osDeleteProcess();
		}
	}
}

typedef enum _ButtonError_t {
	BUTT_OK = 0, BUTT_TO_ERROR, BUTT_SHORT_ERROR
} ButtonError_t;

#define _BUT_MAX_SCAN_PER   200
#define _BUT_MIN_SCAN_PER   10

#define _PDIR_OFFSET        (((unsigned int)&IODIR0 - (unsigned int)&IOPIN0)/sizeof(unsigned int))
#define _PSET_OFFSET        (((unsigned int)&IOSET0 - (unsigned int)&IOPIN0)/sizeof(unsigned int))
#define _PCLR_OFFSET        (((unsigned int)&IOCLR0 - (unsigned int)&IOPIN0)/sizeof(unsigned int))
#define _PIN_OFFSET         (((unsigned int)&IOPIN0 - (unsigned int)&IOPIN0)/sizeof(unsigned int))

#define _PORT1_BUT1_BIT     24
#define _PORT1_BUT2_BIT     25

//#define OS_DI() do {VICIntSelect &= ~0x10;} while(0)
//#define OS_EI() do {VICIntSelect |= 0x10; VICIntEnable = VICIntEnable | 0x10;} while(0)
#define OS_DI() m_os_dis_int()
#define OS_EI() m_os_ena_int()

typedef struct _ButtonCtrl_t {
	volatile unsigned long * pButBaseReg;
	unsigned int ButBit;
} ButtonCtrl_t, *pButtonCtrl_t;

typedef struct _ButtonsPairCtrl_t {
	ButtonCtrl_t Ba;
	ButtonCtrl_t Bb;
} ButtonsPairCtrl_t, *pButtonsPairCtrl_t;

const ButtonsPairCtrl_t ButtonsCtrl[1] = { { .Ba=
{
	.pButBaseReg = &IOPIN1, // CAP_BUTT_1 = P1.24
		.ButBit = _PORT1_BUT1_BIT,
	}, .Bb=
	{
		.pButBaseReg = &IOPIN1, // CAP_BUTT_2 = P1.25
		.ButBit = _PORT1_BUT2_BIT,
	} } };

tU8 readTouch(tU8 id, tU32 *pCount) {
	volatile unsigned int To;
	unsigned int Count, Hold, MasterMask, SlaveMask;
	volatile unsigned int *pMasterReg, *pSlaveReg;
	tSR localSR;

	To = _BUT_MAX_SCAN_PER;

	if (id & 1) {
		pMasterReg = ButtonsCtrl[id >> 1].Ba.pButBaseReg;
		MasterMask = 1UL << ButtonsCtrl[id >> 1].Ba.ButBit;
		pSlaveReg = ButtonsCtrl[id >> 1].Bb.pButBaseReg;
		SlaveMask = 1UL << ButtonsCtrl[id >> 1].Bb.ButBit;
	} else {
		pMasterReg = ButtonsCtrl[id >> 1].Bb.pButBaseReg;
		MasterMask = 1UL << ButtonsCtrl[id >> 1].Bb.ButBit;
		pSlaveReg = ButtonsCtrl[id >> 1].Ba.pButBaseReg;
		SlaveMask = 1UL << ButtonsCtrl[id >> 1].Ba.ButBit;
	}
#if 0
	// Button scan algorithm
	// 1. Starting state Ba-o1 (Port Ba Output H), Bb-o1
	*(pSlaveReg + _PSET_OFFSET) = SlaveMask;
	*(pSlaveReg + _PSET_OFFSET) = MasterMask;
	*(pSlaveReg + _PDIR_OFFSET) |= SlaveMask;
	*(pSlaveReg + _PDIR_OFFSET) |= MasterMask;

	// 2. Set Ba i (input)
	*(pSlaveReg + _PDIR_OFFSET) &= ~SlaveMask;
	//printf("\nS=%x, ", *(pSlaveReg + _PIN_OFFSET) & SlaveMask);

	// 3. Set Bb o0
	OS_DI();
	T1TCR = 1; // enable Timer
	*(pMasterReg + _PCLR_OFFSET) = MasterMask;

	// 4. wait and counting until Ba state get 0
	Count = T1TC;
	while(*(pSlaveReg + _PIN_OFFSET) & SlaveMask)
	{
		if(!To)
		{
			break;
		}
		--To;
	}
	Hold = T1TC - Count;
	OS_EI();
	//printf("S=%x, hold = %d  (%d)  ", *(pSlaveReg + _PIN_OFFSET) & SlaveMask, Hold, To);
	To = _BUT_MAX_SCAN_PER;
#endif

	// 5. Ba o0
	*(pSlaveReg + _PCLR_OFFSET) = SlaveMask;
	*(pSlaveReg + _PDIR_OFFSET) |= SlaveMask;

	// 6. Set Ba i
	*(pSlaveReg + _PDIR_OFFSET) &= ~SlaveMask;
	//printf(", S=%d, ", IOPIN1 & (1UL<<25));

	// 7. Set Bb o1
	OS_DI();
	T1TCR = 1; // enable Timer
	Count = T1TC;
	*(pMasterReg + _PSET_OFFSET) = MasterMask;

	// 8. wait and counting until Ba state get 1
	while (!(*(pSlaveReg + _PIN_OFFSET) & SlaveMask)) {
		if (!To) {
			break;
		}
		--To;
	}
	//  Hold += T1TC - Count;
	Hold = T1TC - Count;
	OS_EI();
	//printf(", S=%d; %d (%d) ", IOPIN1 & (1UL<<25), Hold, To);
	//printf("; %d (%d) ", Hold, To);
	T1TCR = 0; // disable Timer

	// 9. Set Ba o1
	*(pSlaveReg + _PSET_OFFSET) = SlaveMask;
	*(pSlaveReg + _PDIR_OFFSET) |= SlaveMask;

	//  if (id == 0)
	// printf("\n");
	//else
	// printf("\n                          ");
	if (!To) {
		//printf("BUTT_TO_ERROR");
		return (BUTT_TO_ERROR);
	}

	if (To == _BUT_MAX_SCAN_PER) {
		//printf("BUTT_SHORT_ERROR");
		return (BUTT_SHORT_ERROR);
	}

	*pCount = Hold;
	//printf("BUTT_OK (%d)", Hold);
	return (BUTT_OK);
}

/*****************************************************************************
 *
 * Description:
 *    A process entry function 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
void showTime(unsigned long sec,unsigned long min,unsigned long hour){

    char tabS[3];
    sprintf(tabS, "%lu", RTC_SEC);
    char tabMin[3];
    sprintf(tabMin, "%lu", RTC_MIN);
    char tabHr[3];
    sprintf(tabHr, "%lu", RTC_HOUR);

    tabS[2] = '\0';
    tabMin[2] = '\0';
    tabHr[2] = '\0';

    if (RTC_SEC < 10) {
        char temp1 = tabS[0];
        tabS[1] = temp1;
        tabS[0] = '0';
    }
    if (RTC_MIN < 10) {
        char temp2 = tabMin[0];
        tabMin[1] = temp2;
        tabMin[0] = '0';
    }
    if (RTC_HOUR < 10) {
        char temp3 = tabHr[0];
        tabHr[1] = temp3;
        tabHr[0] = '0';
    }

    lcdGotoxy(10, 50);
    lcdPuts(tabHr);

    lcdGotoxy(30, 50);
    lcdPuts(":");

    lcdGotoxy(50, 50);
    lcdPuts(tabMin);

    lcdGotoxy(70, 50);
    lcdPuts(":");

    lcdGotoxy(90, 50);
    lcdPuts(tabS);
}

static void proc2(void* arg) {
	tU8 pca9532Present = FALSE;

	osSleep(50);

	//check if connection with PCA9532
	pca9532Present = pca9532Init();

	if (TRUE == pca9532Present) {
		PINSEL2 &= ~(1 << 3);

		//init_irq(500, 20);

		lcdInit();
		lcdColor(0xff, 0x00);
		lcdClrscr();
		lcdIcon(16, 0, 97, 60, _ea_97x60c[2], _ea_97x60c[3], &_ea_97x60c[4]);
		lcdGotoxy(16, 66);
		lcdPuts("Designed and");
		lcdGotoxy(20, 80);
		lcdPuts("produced by");
		lcdGotoxy(0, 96);
		lcdPuts("Embedded Artists");
		lcdGotoxy(8, 112);
		lcdPuts("(C)2009 (v1.1)");

		unsigned long temp_SEC = RTC_SEC;
		    unsigned long temp_MIN = RTC_MIN;
		    unsigned long temp_HOUR = RTC_HOUR;
		osSleep(100);

		/*
		 * Test to eeprom
		 */
		printf("\n***************************************************");
		printf("\n* EEPROM and RTC test                             *");
		printf("\n***************************************************");
		lcdClrscr();
		lcdGotoxy(0, 0);
		lcdPuts("Test results:");

		lcdGotoxy(0, 16);
		if (testEEPROM() == TRUE) {
			lcdPuts("EEPROM: OK");
			printf("\n\nSummary of tests: Passed all tests!");
		} else {
			lcdPuts("EEPROM: FAILED!");
			printf("\n\nSummary of tests: Failed at least one test!");
		}

		//Test RTC
		//
		//Test the RTC (Real-Time Clock)
		//
		printf("\n\nTest #2: RTC initializing");
		lcdGotoxy(0, 32);
		//lcdPuts("RTC: ");

		RTC_CCR = 0x00000012;
		RTC_CCR = 0x00000010;
		RTC_ILR = 0x00000000;
		RTC_CIIR = 0x00000000;
		RTC_AMR = 0x00000000;
		//PREINT  = 449;
		//PREFRAC = 0;
		osSleep(50);
		RTC_SEC = 50;
		RTC_MIN = 59;
		RTC_HOUR = 23;
		osSleep(50);
		RTC_CCR = 0x00000011;

		osSleep(50);
		lcdPuts(".");
		printf(".");
		osSleep(50);
		lcdPuts(".");
		printf(".");
		osSleep(50);
		lcdPuts(".");
		printf(".");
		osSleep(50);
		lcdPuts(".");
		printf(".");
		osSleep(50);
		lcdPuts(".");
		printf(".");
		osSleep(50);
		lcdPuts(".");
		printf(".");
		osSleep(20);

		tU8 isTimeBeingSet = 0;
		    tU16 currentType = 0;

		    while (1==1) {
		        //check if P0.8 center-key is pressed
		        if ((IOPIN & 0x00000100) == 0)
		        {
		            isTimeBeingSet = 1;
		        }
		        if(isTimeBeingSet==1){
		            //check if P0.8 center-key is pressed
		            if ((IOPIN & 0x00000100) == 0)
		            {
		                currentType++;
		                if(currentType > 2){
		                    isTimeBeingSet = 0;
		                }
		            }
		            //check if P0.10 up-key is pressed
		            else if((IOPIN & 0x0000400) == 0){
		                switch(currentType){
		                    case 0:
		                        temp_HOUR++;
		                        break;
		                    case 1:
		                        temp_MIN++;
		                        break;
		                    case 2:
		                        temp_SEC++;
		                        break;
		                }
		            }
		            //check if P0.12 down-key is pressed
		            else if((IOPIN & 0x00001000) == 0){
		                switch(currentType){
		                    case 0:
		                        temp_HOUR--;
		                        break;
		                    case 1:
		                        temp_MIN--;
		                        break;
		                    case 2:
		                        temp_SEC--;
		                        break;
		                }
		            }
		            showTime(temp_SEC,temp_MIN,temp_HOUR);
		        }
		        else{
		            if(RTC_HOUR == temp_HOUR && RTC_MIN == temp_MIN && RTC_SEC == temp_SEC){
		                showTime(RTC_SEC,RTC_MIN,RTC_HOUR);
		            }
		            else{
		                showTime(RTC_SEC,RTC_MIN,RTC_HOUR);
		                temp_SEC = RTC_SEC;
		                temp_MIN = RTC_MIN;
		                temp_HOUR = RTC_HOUR;
		                lcdGotoxy(10, 80);
		                lcdPuts("Wake up!\0");
		            }

		        }
		        //odswiezaj co sekunde
		        //sdelay(1);
		    }



		printf("\n\nRTC test: RTC initializing");

		RTC_CCR = 0x00000012;
		RTC_CCR = 0x00000010;
		RTC_ILR = 0x00000000;
		RTC_CIIR = 0x00000000;
		RTC_AMR = 0x00000000;
		//PREINT  = 449;
		//PREFRAC = 0;
		osSleep(50);
		RTC_SEC = 0;
		RTC_MIN = 0;
		RTC_HOUR = 0;
		osSleep(50);
		RTC_CCR = 0x00000011;

		osSleep(50);
		printf(".");

		osSleep(50);
		printf(".");

		osSleep(50);
		printf(".");

		osSleep(50);
		printf(".");

		osSleep(50);
		printf(".");

		osSleep(50);
		printf(".");

		osSleep(20);

		if ((RTC_SEC == 3) && (RTC_MIN == 0) && (RTC_HOUR == 0)) {
			printf(" test OK!\n");
		} else {
			printf(" test FAILED! [%d:%d:%d], but should be [0:0:3]\n", RTC_HOUR, RTC_MIN, RTC_SEC);
			while (1) {
				IOSET = 0x00260000;
				osSleep(15);
				IOCLR = 0x00040000;
				osSleep(15);
			}
		}

	}

	//Initialize ADC
	initAdc();

	lcdGotoxy(0, 100);
	lcdPuts("Start audio with");
	lcdGotoxy(0, 116);
	lcdPuts("P0.14 key");

	T1TCR = 0; // counter disable
	T1PR = 0; // set prescaler /1
	T1MCR = 0; // disable match act
	T1EMR = 0; // disable external match act
	IOSET1 = ((1UL << 25) | (1UL << 24));
	IODIR1 |= ((1UL << 25) | (1UL << 24));

	for (;;) {
		osSleep(10);
		if (TRUE == pca9532Present) {
			static tU8 message[] = "Current time is: 1315";
			tU16 adValue;

			//read analog input
			lcdGotoxy(0, 48);

			adValue = getAnalogueInput(AIN1);

			message[14] = ((adValue / 1) % 10) + '0';
			message[13] = ((adValue / 10) % 10) + '0';
			message[12] = ((adValue / 100) % 10) + '0';
			message[11] = ((adValue / 1000) % 10) + '0';
			if (message[11] == '0') {
				message[11] = ' ';
				if (message[12] == '0') {
					message[12] = ' ';
					if (message[13] == '0')
						message[13] = ' ';
				}
			}
			lcdPuts(message);
		}

		else {
			rgbSpeed = (getAnalogueInput(AIN1) >> 7) + 3;
		}

		//detect if P0.14 key is pressed
		if ((IOPIN & 0x00004000) == 0) {
			tSR localSR;

			//read analog input
			lcdGotoxy(0, 100);
			lcdPuts("Audio echo app-");
			lcdGotoxy(0, 116);
			lcdPuts("lication active");

			//      killProc1 = TRUE;
			//      osSleep(1);
			m_os_dis_int();

			//
			//Initialize DAC: AOUT = P0.25
			//
			PINSEL1 &= ~0x000C0000;
			PINSEL1 |= 0x00080000;

			for (;;) {
				static tU16 buffer[10000];
				static tU16 counter = 0;
				tU16 adValue;

				adValue = getAnalogueInput(AIN2);

				//set analogue output
				DACR = (buffer[counter] << 6) | //actual value to output
						(1 << 16); //BIAS = 1, 2.5uS settling time

				buffer[counter++] = adValue;
				if (counter >= 10000)
					counter = 0;

				udelay(30);
			}
		}

		{
			tU32 dummy;
			lcdGotoxy(20, 72);
			if (BUTT_OK == readTouch(0, &dummy)) {
				IOCLR1 = 0x00010000;
				lcdPuts("T#1");
			} else
				lcdPuts("___");

			lcdGotoxy(75, 72);
			if (BUTT_OK == readTouch(1, &dummy)) {
				IOCLR1 = 0x00080000;
				lcdPuts("T#2");
			} else
				lcdPuts("___");
		}

#if 0
		for(int i = 0; i < _CAP_BUTTONS_NUMB; ++i)
		{
			if(BSP_GetButton(i,&ButtCnt) != BUTT_OK)
			{
				ButtCnt = 0;
				CapKey.KeyErrorMask |= 1UL << i;
			}
			else
			{
				// analyzing key counter and calculate key press level
				// assume that unpressed level is minimum value of button's counter
				if(ButtCnt < KeyUnpressLevel[i])
				{
					KeyUnpressLevel[i] = ButtCnt;
					KeyPressLevel[i] = ButtCnt + (ButtCnt/2);
				}
				if (ButtCnt >= KeyPressLevel[i])
				{
					// The key is pressed
					CapKey.PressedKeysMask |= 1UL << i;
				}
			}
		}
#endif

	}
}

/*****************************************************************************
 *
 * Description:
 *    The entry function for the initialization process. 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void initProc(void* arg) {
	tU8 error;

	eaInit(); //initialize printf
	i2cInit(); //initialize I2C
	osCreateProcess(proc1, proc1Stack, PROC1_STACK_SIZE, &pid1, 3, NULL, &error);
	osStartProcess(pid1, &error);
	osCreateProcess(proc2, proc2Stack, PROC2_STACK_SIZE, &pid2, 3, NULL, &error);
	osStartProcess(pid2, &error);

	osDeleteProcess();
}

/*****************************************************************************
 *
 * Description:
 *    The timer tick entry function that is called once every timer tick
 *    interrupt in the RTOS. Observe that any processing in this
 *    function must be kept as short as possible since this function
 *    execute in interrupt context.
 *
 * Params:
 *    [in] elapsedTime - The number of elapsed milliseconds since last call.
 *
 ****************************************************************************/
void appTick(tU32 elapsedTime) {
	msClock += elapsedTime;
}
