#ifndef __CLOCK_H__
#define __CLOCK_H__

#include "config.h"
#include "at32f415_ertc.h"

void clock_init(void);
int clock_available(void);
void clock_get_time(void);

#endif // __CLOCK_H__
