/*
    Funkcje sluzace do obslugi czujnika wilgotności HTU21D(F)
*/

#include "htu21df.h"
/*!
* @brief Funkcja pobierajaca wartosc wilgotnosci przez I2C
* @param byteArray
*			Tablica 2 bajtow pobrana wczesniej przez I2C
* @returns Wyliczona wilgotnosc
 * @side effects: 
 *          Brak
*/ 
tS8 calculateHumidity(tU8 *byteArray)
{
    /*
        Konwersja tablicy 2 bajtów na jedną zmienną 16 bitową.
        Wyłączenie bitów stanu - dla wilgotności są to ostatnie 4 bity.
    */
	tU8 mask = ~(0x0F);
	byteArray[1] = (byteArray[1] & mask);
    tU16 readData = ((byteArray[0] << 8) | byteArray[1]);

    return ((((tU32)125 * (tU32)readData) / ((tU32)1 << 16)) - (tU32)6);
}
/*!
* @brief Funkcja pobierajaca wartosc wilgotnosci przez I2C
* @returns Wartosc wilogtnosci.
 * @side effects: 
 *          Brak
*/ 
tS8 measureHumidity(void)
{
    // Adres czujnika wilgotności HTU21DF
    tU8 htu21Address = 0x40;    // 0x40 = 0b100000
    // Adres wykorzystywany do zapisu do czujnika wilgotności HTU21DF
    tU8 htu21ReadAddress = ((tU8)(htu21Address << 1) | (tU8)1);
    // Adres wykorzystywany do odczytu z czujnika wilgotności HTU21DF
    tU8 htu21WriteAddress = ((tU8)(htu21Address << 1));
    // Tablica tU8 wykorzystywana do przechowania wartości wilgotności zczytanej z czujnika
    tU8 readHumidity[2] = {0};
    tS8 retCode;
    // Komenda odczytu wilgotności w trybie "No Hold Master".
    tU8 commandNoHoldMaster[1] = {0};
    commandNoHoldMaster[0] = 0xF5;
    
    retCode = i2cWrite(htu21WriteAddress, commandNoHoldMaster, 1);
    mdelay(16);
    tS8 actualHumidity = 0;
    if (retCode != I2C_CODE_ERROR) {
        retCode = i2cRead(htu21ReadAddress, readHumidity, 2);
        if (retCode != I2C_CODE_ERROR) {
            actualHumidity = calculateHumidity(readHumidity);
        }
    }
    return actualHumidity;
}
/*!
* @brief Funkcja wypisujaca na tablice aktualna wartosc wilgotnosci
 * @side effects: 
 *          Ustawienie wskaznika pozycji LCD na X = 20, Y = 45
*/ 
void printHumidity(void){
	const tU8 humidityPrompt[] = "Wilgotnosc:\0";
	const tU8 percentPrompt[] = "%\0";

    tU8 humidityString[10] = {0};

    tU16 humidity = measureHumidity();
    tU8 sprintfHolder;
    sprintfHolder = sprintf(humidityString,"%d",humidity);
    lcdGotoxy(0,30);
    lcdPuts(humidityPrompt);
    lcdGotoxy(0,45);
    lcdPuts(humidityString);
    lcdGotoxy(20,45);
    lcdPuts(percentPrompt);
}
