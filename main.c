/*************************************************************************************
 *
 * @Description:
 * Program przykładowy - odpowiednik "Hello World" dla systemów wbudowanych
 * Rekomendujemy wkopiowywanie do niniejszego projektu nowych funkcjonalności
 *
 *
 * UWAGA! Po zmianie rozszerzenia na cpp program automatycznie będzie używać
 * kompilatora g++. Oczywiście konieczne jest wprowadzenie odpowiednich zmian w
 * pliku "makefile"
 *
 *
 * Program przykładowy wykorzystuje Timer #0 i Timer #1 do "mrugania" diodami
 * Dioda P1.16 jest zapalona i gaszona, a czas pomiędzy tymi zdarzeniami
 * odmierzany jest przez Timer #0.
 * Program aktywnie oczekuje na upłynięcie odmierzanego czasu (1s)
 *
 * Druga z diod P1.17 jest gaszona i zapalana w takt przerwań generowanych
 * przez timer #1, z okresem 500 ms i wypełnieniem 20%.
 * Procedura obsługi przerwań zdefiniowana jest w innym pliku (irq/irq_handler.c)
 * Sama procedura MUSI być oznaczona dla kompilatora jako procedura obsługi 
 * przerwania odpowiedniego typu. W przykładzie jest to przerwanie wektoryzowane.
 * Odpowiednia deklaracja znajduje się w pliku (irq/irq_handler.h)
 * 
 * Prócz "mrugania" diodami program wypisuje na konsoli powitalny tekst.
 * 
 * @Authors: Michał Morawski,
 *           Daniel Arendt, 
 *           Przemysław Ignaciuk,
 *           Marcin Kwapisz
 *
 * @Change log:
 *           2016.12.01: Wersja oryginalna.
 *
 ******************************************************************************/

#include "general.h"
#include <lpc2xxx.h>
#include <printf_P.h>
#include <printf_init.h>
#include <consol.h>
#include <config.h>
#include "irq/irq_handler.h"
#include "timer.h"
#include "VIC.h"
#include "stdbool.h" //TO MOZE NIE DZIALAC ??
#include "Common_Def.h"
#include <stdio.h>



/************************************************************************
 * @Description: opóźnienie wyrażone w liczbie sekund
 * @Parameter:
 *    [in] seconds: liczba sekund opĂłĹşnienia
 * @Returns: Nic
 * @Side effects:
 *    przeprogramowany Timer #0
 *************************************************************************/
static void sdelay (tU32 seconds)
{
	T0TCR = TIMER_RESET;                    //Zatrzymaj i zresetuj
    T0PR  = PERIPHERAL_CLOCK-1;             //jednostka w preskalerze
    T0MR0 = seconds;
    T0IR  = TIMER_ALL_INT;                  //Resetowanie flag przerwaĹ„
    T0MCR = MR0_S;                          //Licz do wartości w MR0 i zatrzymaj się
    T0TCR = TIMER_RUN;                      //Uruchom timer

    // sprawdź czy timer działa
    // nie ma wpisanego ogranicznika liczby pętli, ze względu na charakter procedury
    while (T0TCR & TIMER_RUN)
    {
    }
}

/************************************************************************
 * @Description: uruchomienie obsługi przerwań
 * @Parameter:
 *    [in] period    : okres generatora przerwań
 *    [in] duty_cycle: wypełnienie w %
 * @Returns: Nic
 * @Side effects:
 *    przeprogramowany timer #1
 *************************************************************************/
static void init_irq (tU32 period, tU8 duty_cycle)
{
	//Zainicjuj VIC dla przerwań od Timera #1
    VICIntSelect &= ~TIMER_1_IRQ;           //Przerwanie od Timera #1 przypisane do IRQ (nie do FIQ)
    VICVectAddr5  = (tU32)IRQ_Test;         //adres procedury przerwania
    VICVectCntl5  = VIC_ENABLE_SLOT | TIMER_1_IRQ_NO;
    VICIntEnable  = TIMER_1_IRQ;            // Przypisanie i odblokowanie slotu w VIC od Timera #1
  
    T1TCR = TIMER_RESET;                    //Zatrzymaj i zresetuj
    T1PR  = 0;                              //Preskaler nieużywany
    T1MR0 = ((tU64)period)*((tU64)PERIPHERAL_CLOCK)/1000;
    T1MR1 = (tU64)T1MR0 * duty_cycle / 100; //Wypełnienie
    T1IR  = TIMER_ALL_INT;                  //Resetowanie flag przerwań
    T1MCR = MR0_I | MR1_I | MR0_R;          //Generuj okresowe przerwania dla MR0 i dodatkowo dla MR1
    T1TCR = TIMER_RUN;                      //Uruchom timer
}

void showTime(unsigned long sec,unsigned long min,unsigned long hour){
    
    char tabS[3];
    sprintf(tabS, "%d", RTC_SEC);
    char tabMin[3];
    sprintf(tabMin, "%d", RTC_MIN);
    char tabHr[3];
    sprintf(tabHr, "%d", RTC_HOUR);

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



void main(void)
{
	//uruchomienie 'simple printf'
    printf_init();

    //powitanie
    printf("\n\n\n\n");
    printf("\n*********************************************************");
    printf("\n*");
    printf("\n* Systemy Wbudowane");
    printf("\n* Wydzial FTIMS");
    printf("\n* Moj (nasz) pierwszy program");
    printf("\n*");
    printf("\n*********************************************************");

    /*********************************************************************
     * Ta część inicjuje działanie programy
     *********************************************************************/
    // uruchomienie GPIO na nodze P1.16 i P1.17.
    PINSEL2 &= ~(1 << 3);
    // Kierunek out na nodze P1.16
    IODIR1 |= DIODA_LEWA; //(1 << 16);
    // Kierunek out na nodze P1.17
    IODIR1 |= DIODA_PRAWA; //(1 << 17);
    // Uruchomienie przerwań co 1/2 s.
    init_irq(500, 20);

    

    

    lcdInit();
    lcdColor(0xff, 0x00);
    lcdClrscr();

    //osSleep(100);

    RTC_CCR = 0x00000012;
    RTC_CCR = 0x00000010;
    RTC_ILR = 0x00000000;
    RTC_CIIR = 0x00000000;
    RTC_AMR = 0x00000000;

    RTC_SEC = 50;
    RTC_MIN = 59;
    RTC_HOUR = 23;

    unsigned long temp_SEC = RTC_SEC;
    unsigned long temp_MIN = RTC_MIN;
    unsigned long temp_HOUR = RTC_HOUR;


    RTC_CCR = 0x00000011; // ?? wtf is that

    // byc moze to bedzie problematyczne, bo bool jest z zewnetrznej biblioteki
    bool isTimeBeingSet = false;
    tU16 currentType = 0;

    while (true) {
        //check if P0.8 center-key is pressed
        if ((IOPIN & 0x00000100) == 0)
        {
            isTimeBeingSet = true;
        }
        if(isTimeBeingSet){
            //check if P0.8 center-key is pressed
            if ((IOPIN & 0x00000100) == 0)
            {
                currentType++;
                if(currentType<0 && currentType > 2){
                    isTimeBeingSet = false;
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
                unsigned long temp_SEC = RTC_SEC;
                unsigned long temp_MIN = RTC_MIN;
                unsigned long temp_HOUR = RTC_HOUR;
                lcdGotoxy(10, 80);
                lcdPuts("Wake up!\0");
            }
            
        }
        //odswiezaj co sekunde
        sdelay(1);
    }

} 


