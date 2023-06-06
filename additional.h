#ifndef ADDITIONAL_H
#define ADDITIONAL_H

#include "general.h"
#include "lpc2xxx.h"
#include "config.h"

void udelay(tU32 delayInuS);
void mdelay(unsigned int delayInMs);
void sdelay(unsigned int delayInS);

#endif