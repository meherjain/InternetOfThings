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
#include "em_adc.h"
#include "em_dma.h"
#include "dmactrl.h"
#include "em_i2c.h"
#include "i2c.h"



// Defining the constants
#define LFXO_FREQ 	       			32768	    // 32.768KHz
#define LFXO_FREQ_PRESCALAR 		8192		// 8.192KHz
#define ULFRCO_FREQ        			1000	    // 1KHz
#define TIMER_PERIOD				4996        // 4seconds
#define CALIBRATION_COUNT_ULFRCO	1000 		// 1seconds
#define CALIBRATION_COUNT_LFXO		32768		// 1seconds
#define EXCITE_PERIOD				0.004		// 4ms
#define LETIMER_MAX_COUNT  			65535       // 16bit Counter
#define MAX_COUNT               	65536		// 0-65535
#define VDD_HIGH_REFERENCE      	0x3D		// High Voltage Level
#define VDD_LOW_REFERENCE			0x02		// Low Voltage Level


//ADC Definition //
#define ADC_SAMPLES					400							// Converting 400 samples per ADC trigger//
#define ADC_SAMPLES_SEC				75000						// ADC running at 75ksps
#define ADC_TEMP_CHANNEL			adcSingleInputTemp			// Using Temperature sensor as ADC input channel
#define ADC_SINGLE_CONVERSION       adcStartSingle
#define ADC_RESOLUTION 				adcRes12Bit
#define ADC_ACQUSITION_TIME			adcAcqTime2
#define ADC_WARMUP					adcWarmupNormal
#define ADC_REFERENCE_LEVEL			adcRef1V25
#define prescale10ksps				12							//Tksps = (Ta + N)*OSR, TA = Acquisition Time, N= ADC_Bits, OSR = OverSamplingRatio
																// To get the 75ksps, acquisition time of 8 ADC_Clock Cycles is used*/

// Temperature Definition
#define MAX_TEMP_LIMIT				35							// Maximum Temperature Limit
#define MIN_TEMP_LIMIT				15							// Minimum Temperature Limit

//Define the Calibration Add
#define DEV_CAL_ADDRESS			   (*(volatile unsigned long *) (0x0FE081B0UL))				// Address in the memory (Calibration Value is stored)
#define DEV_ADC_CAL_ADDRESS 	   (*(volatile unsigned long *) (0x0FE081BCUL))

// DMA Definition //
#define DMA_CHANNEL_ADC				0							// Using DMA channel 0 for ADC transfer

// Load Power Management Pin
#define LPM_GPIO_Port				gpioPortD
#define LPM_GPIO_Pin				0

// GPIO Interrupt Pins
#define GPIO_INT_PORT				gpioPortD
#define GPIO_INT_PIN				1

// LED Channel Definition//
#define LED							gpioPortE					// LED port
#define LED0						2							// LED 0 Number
#define LED1						3							// LED 1 Number


// ACMP Channel Definition //
#define ACMP_POSITIVE_CHANNEL		acmpChannel6		// Channel 6 of ACMP
#define ACMP_NEGATIVE_CHANNEL		acmpChannelVDD		// VDD Reference
#define ACMP_EXCITE_PORT			gpioPortD			// Excite Port
#define ACMP_SENSE_PORT				gpioPortC			// Sense Port


#define INTERNAL_LIGHT_SENSOR 0

#if (INTERNAL_LIGHT_SENSOR ==1)
#define INTERNAL_SENSOR_ON
#endif

// User can define the ULFRCO Calibration Routine should be ran or not // 1 - CALIBRATION ON, 0- CALIBRATION OFF
#define ULFRCO_CALIBRATION	1

#if (ULFRCO_CALIBRATION == 1)
#define CALIBRATION
#endif

// User can select the energy mode by changing to EM0,EM1,EM2,EM3
#define EnergyMode     EM3


// User can choose to use DMA
#define USE_DMA			1

#if (USE_DMA == 1)
#define DMA_ON
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
void adcConfig(void);
void dmaConfig(void);
float convertToCelsius(int32_t);
void ADC0_IRQHandler(void);






#endif /* MAIN_H_ */
