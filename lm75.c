/*
    Funkcje sluzace do pobierania temperatury z LM75
    oraz wypisywania ich na ekran
*/

#include "lm75.h"

/*!
* @brief Funkcja pobierajaca wartosc temperatury przez I2C
* @param addr
*			Adres termometru.
* @param addr
*			Bufor do ktorego przkeazywany jest odczyt
* @returns Wynik wykonania operacji I2C.
 * @side effects: 
 *          Brak
*/ 
tS8 measureTemperature(tU8 addr, tU8 *pBuf)
{
    return i2cRead(addr, pBuf, 2);
}


/*!
* @brief Funkcja wypisujaca na ekran (w aktualnym X,Y) wartosc temperatury pobrana przez I2C.
* @param byteArray
*			Tablica tU8 pobrana z I2C
* @returns void
 * @side effects: 
 *          Brak
*/ 
void calculateTemperatureValue(tU8 *byteArray)
{
    // Tablica przetrzymujaca temperature w formie znakowej, do wypisania na ekran
    tU8 charArray[10] = {0};

    // Zmienna przechowujaca wartosc temperatury
    // Jest typu tS8, poniewaz temperatura moze byc ujemna
    tS8 calculatedValue = byteArray[0];
    // Flagi okreslajace czy temperatura jest ujemna i czy nalezy do niej dodac 0.5
    tU8 isNegative = 0;
    tU8 appendHalf = 0;
    
    if (calculatedValue < 0) {
        isNegative = 1;
    }
    // Kiedy temperatura jest ujemna i najbardziej znaczacy bit w najmniej znaczacym BAJCIE jest rowny 1
    if ((isNegative == (tU8)1) && ((byteArray[1] & (tU8)0x80) == (tU8)0x80)){
        calculatedValue = calculatedValue + (tS8)1;
    	appendHalf = 1;
    } // Kiedy temperatura jest NIEujemna i najbardziej znaczacy bit w najmniej znaczacym BAJCIE jest rowny 1 
    else if ((isNegative == (tU8)0) && ((byteArray[1] & (tU8)0x80) == (tU8)0x80)) {
    	appendHalf = 1;
    } else {
        ;
    }

    // Zmienna przetrzymujaca wartosc zwracana przez funkcje sprintf (wymaganie MISRY)
    tU8 resultHolder;
	resultHolder = sprintf(charArray, "%d", calculatedValue);

    // Iterator dla petli
    tU8 i = 0;
    while(charArray[i] != (tU8)0) {
        i = i + (tU8)1;
    }

    charArray[i] = '.';
    if (appendHalf == (tU8)1) {
        charArray[i + (tU8)1] = '5';
    } else {
        charArray[i + (tU8)1] = '0';
    }

    lcdPuts(charArray);
}
