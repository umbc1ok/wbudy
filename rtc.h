#ifndef RTC_H
#define RTC_H

#include "general.h"
#include "lpc2xxx.h"
void showTime(tU8 sec,tU8 min,tU8 hour, tU8 highlight);
void setAlarm(tU8 sec, tU8 min, tU8 hour);


#endif
