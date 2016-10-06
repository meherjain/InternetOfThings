/*
 * main.h
 *
 *  Created on: Sep 11, 2016
 *      Author: Meher
 */

#ifndef MAIN_H_
#define MAIN_H_

// Include Header Files
#include "stdio.h"
#include "stdbool.h"
#include "em_device.h"
#include "em_chip.h"
#include "em_int.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_letimer.h"


// User can select the energy mode by changing to EM0,EM1,EM2,EM3


// Defining the constants
#define LFXO_FREQ 	       		32768	    // 32.768KHz
#define ULFRCO_FREQ        		1000	    // 1KHz
#define ON_DUTY_CYCLE      		50          // 50ms
#define LED_BLINK_PERIOD   		2000 	    // 2000ms
#define LETIMER_MAX_COUNT  		65535       // 16bit Counter
#define ULFRCO_ADJUST_PERIOD 	300			// ULFRCO Adjustment because of unstable frequency
#define ULFRCO_ADJUST_ON_CYCLE 	56			// ULFRCO Adjustment because of unstable frequency for ON duty cycle


typedef enum {
	EM0 =0,
	EM1,
	EM2,
	EM3,
	EM4
}sleepstate_enum;


void letimer_initialize(sleepstate_enum);
void GPIO_enable(void);
void clock_init(sleepstate_enum);
void LED_ON(void);
void LED_OFF(void);
void LETIMER0_IRQHandler(void);






#endif /* MAIN_H_ */
