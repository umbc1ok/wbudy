/*************************************************************************************
 *
 * @Description:
 * Przykład definiowania uniwersalnych typów.
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
#ifndef __GENERAL_H__
#define __GENERAL_H__


#define _BIT(n)                   (1<<(n))
#define _PIN(port,bit)            ((IOPIN##port & _BIT(bit)) != 0)

#endif  //__GENERAL_H__

