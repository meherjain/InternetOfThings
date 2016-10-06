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
#include "em_acmp.h"
#include "em_timer.h"



// Defining the constants
#define LFXO_FREQ 	       			32768	    // 32.768KHz
#define LFXO_FREQ_PRESCALAR 		16384		// 16.384KHz
#define ULFRCO_FREQ        			1000	    // 1KHz
#define TIMER_PERIOD				3000        // 3seconds
#define CALIBRATION_COUNT_ULFRCO	1000 		// 1seconds
#define CALIBRATION_COUNT_LFXO		32768		// 1seconds
#define EXCITE_PERIOD				0.004		// 4ms
#define LETIMER_MAX_COUNT  			65535       // 16bit Counter
#define MAX_COUNT               	65536		// 0-65535
#define VDD_HIGH_REFERENCE      	0x3D		// High Voltage Level
#define VDD_LOW_REFERENCE			0x02		// Low Voltage Level

// ACMP Channel Definition //
#define ACMP_POSITIVE_CHANNEL		acmpChannel6		// Channel 6 of ACMP
#define ACMP_NEGATIVE_CHANNEL		acmpChannelVDD		// VDD Reference
#define ACMP_EXCITE_PORT			gpioPortD			// Excite Port
#define ACMP_SENSE_PORT				gpioPortC			// Sense Port

// User can define the ULFRCO Calibration Routine should be ran or not // 1 - CALIBRATION ON, 0- CALIBRATION OFF
#define ULFRCO_CALIBRATION	1

// User can select the energy mode by changing to EM0,EM1,EM2,EM3
#define EnergyMode     EM3


#if (ULFRCO_CALIBRATION == 1)
#define CALIBRATION
#endif


typedef enum {
	EM0 =0,
	EM1,
	EM2,
	EM3,
	EM4
}sleepstate_enum;

/***************************************Function Prototypes *************************************/

void letimer_initialize(sleepstate_enum);
void GPIO_init(void);
void clock_init(sleepstate_enum);
void ACMPInit(sleepstate_enum);
void ACMP0_IRQHandler(void);
void LETIMER0_IRQHandler(void);
void TIMER0_IRQHandler(void);
void TIMER_Initialize(void);
void TIMER_SETUP(void);
void LETIMER_Calibration(void);





#endif /* MAIN_H_ */
