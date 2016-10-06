/*
 * sleep.h
 *
 *  Created on: Sep 11, 2016
 *      Author: Meher
 */

#ifndef SLEEP_H_
#define SLEEP_H_


#include "main.h"


#define EnergyMode     EM3


void unblockSleepMode(sleepstate_enum);
void sleep(void);
void blockSleepMode (sleepstate_enum);



#endif /* SLEEP_H_ */
