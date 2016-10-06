
/*
 * Name: Meher Jain
 * Email ID: meher.jain@colorado.edu
 * Date: 09/18/2016
 * Description: Assignment-3 ECEN 5023 */


/***************************************************Include Header Files ******************************/

#include "sleep.h"

/********************************************Declaring the global constants ***************************/
uint32_t ACMP_OUT             = 0;										// ACMP Output //
uint32_t LETIMER_COMP0        = 0;										// LETIMERE_COMP0 Register Value */
uint32_t count_value_EM2      = 0;										// COMP0 count value for EM2
uint32_t count_value_EM3      = 0;										// COMP0 count value for EM3
uint32_t excite_flag 	      = 1;										// excite flag for ACMP excitation
uint32_t Calibrated_ULFRCO    = 0;
volatile int16_t BufferAdcData[ADC_SAMPLES];							// Global Variable for DMA to store data
DMA_CB_TypeDef callback;												// DMA Callback Descriptors
uint16_t adcCount_DMA_OFF	  = ADC_SAMPLES;									// ADC Counts for Interrupt handler


 // Initializing the structure for ACMP //
ACMP_Init_TypeDef acmp_init =
	{
			false,						 // fullBias Current
			true,						 // Half Bias Current
			0x00,						 // Bias Program
			false,						 // Enable interrupt for falling edge
			false,						 // Enable Interrupt for rising edge
			acmpWarmTime256,			 // Warm-up time in clock cycles
			acmpHysteresisLevel4,	     // Hysteresis Configuration
			false,						 // Inactive comparator output value during warmup
			true,						 // Low power reference
			VDD_LOW_REFERENCE,			 // Vdd reference scaling
			true						 // Enable ACMP after Init
	};






/***************************************************************************//**
 * @Function convetToCelsius
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 ******************************************************************************/
float convertToCelsius(int32_t adcSample)
{
	float temp;
	/* Factory calibration temperature from device information page */
	float cal_temp_0 = (float)((DEV_CAL_ADDRESS & _DEVINFO_CAL_TEMP_MASK)
					 >> _DEVINFO_CAL_TEMP_SHIFT);
	float cal_value_0 = (float)((DEV_ADC_CAL_ADDRESS
						& _DEVINFO_ADC0CAL2_TEMP1V25_MASK)
						>> _DEVINFO_ADC0CAL2_TEMP1V25_SHIFT);

	/* Temperature gradient (from datasheet)*/
	float t_grad = -6.27;
	temp = (cal_temp_0 - ((cal_value_0-adcSample) / t_grad));
	return temp;
}

/*
 * Call Back Function for DMA Channel 0
 * Description: This function is called when DMA completes the ADC_SAMPLES transfers. The average is taken of those ADC samples and average
 * is converted to actual temperature (Celsius). Using the average temp, decision is taken to lit the LED or not based on the requirement.
 * args: Channel, primary, *user
 * Return: None
 * Frequency: When DMA completes 1000 transfers every 4 seconds
 * */

void transferComplete(unsigned int channel, bool primary, void *user)
{
	uint32_t averageSample = 0;
	float averageTemp = 0.0;
	int i =0;
	ADC0->CMD |= ADC_CMD_SINGLESTOP;															// Stopping the ADC
	unblockSleepMode(EM1);																		// Unblocking the sleep mode EM1

 	for(i=0;i<ADC_SAMPLES;i++)																	// Summation of 1000 ADC Samples
	{
		averageSample +=BufferAdcData[i];
	}
	averageSample /=ADC_SAMPLES;																// Taking Average
	averageTemp = convertToCelsius(averageSample);												// Converting ADC Sample to Celsius (Using Silicon Labs Routine)

	if((averageTemp >= MIN_TEMP_LIMIT) && (averageTemp<=MAX_TEMP_LIMIT))						// Checking the Temp Range
	{
		GPIO_PinOutClear(LED,LED1);																// LED OFF if inside the range
	}

	else if(averageTemp < MIN_TEMP_LIMIT || (averageTemp>=MAX_TEMP_LIMIT))
	{
		GPIO_PinOutSet(LED,LED1);																// LED ON if outside the range
	}

}


/*
 * Timer0 Interrupt Handler
 * Description: Clears the overflow interrupt of the timer0
 * Args: None
 * Return: None
 */

void TIMER0_IRQHandler(void)
{
	TIMER_IntClear(TIMER0,TIMER_IF_OF);
}

/*
 * ACMP Interrupt Handler
 * Description: Clears the EDGE interrupt of the ACMP
 * Args: None
 * Return: None
 */
void ACMP0_IRQHandler(void){

	ACMP0->IFC = ACMP_IFC_EDGE;
	//GPIO_PinOutToggle(LED,3);
}

/*
 * ADC Interrupt Handler
 * Description: This routine takes average of 1000 ADC samples from temperature sensors. After collecting 1000 samples, count is reset and it checks
 *  the range of average temperature, if outside the range, LED1 is turned ON until the average temperature comes back to normal range. ADC is turned
 *  off untill it is started back in the LETIMER after 4 seconds. In the end, the EM1 energy mode is unblocked, so that processor can go back to
 *  deep sleep untill next time ADC is started.
 * Args: None
 * Return: None
 * Frequency: every 4 sec untill 1000 conversions are complete (approx 100ms)
 * Disclaimer: ADC interrupts are only used if DMA is OFF
 */
void ADC0_IRQHandler(void){

	static uint32_t adcSample;
	float averageTemp = 0.0;
	INT_Disable();																					// Disable the interrupts

	ADC_IntClear(ADC0,ADC_IEN_SINGLE);																// Clear the ADC interrupt

	if(adcCount_DMA_OFF!=0)																			// Till 1000 conversions
	{
		adcSample+= ADC0->SINGLEDATA;																// Take Average
		adcCount_DMA_OFF--;																			// Decrement Count
	}
	else
	{
		adcCount_DMA_OFF = 1000;																	// Reinitialize the Count
		adcSample /=ADC_SAMPLES;																	// Take Average
		averageTemp = convertToCelsius(adcSample);													// Convert to Celsius

		if((averageTemp >= MIN_TEMP_LIMIT) && (averageTemp<=MAX_TEMP_LIMIT))						// Checking the Temp limit
		{
			GPIO_PinOutClear(LED,LED1);																// Clear LED 1
		}

		else if(averageTemp < MIN_TEMP_LIMIT || (averageTemp>=MAX_TEMP_LIMIT))
		{
			GPIO_PinOutSet(LED,LED1);																// Light LED 1
		}
		ADC0->CMD |= ADC_CMD_SINGLESTOP;															// Stop the ADC
		unblockSleepMode(EM1);																		// Unblock the Sleep Mode EM1
	}

	INT_Enable();																					// Enable the interrupts

}

/*
 *LETIMER interrupt handler routine
 *Description: This routine is executed when LETIMER interrupt occurs. In the routine, the further interrupts are disabled at the start
 *and are enabled at the end such that, no more interrupts can occur when the handler is executing. The routine checks the energy mode
 *processor is at clears the appropriate interrupts that invoked the handler in the first place. In this way processor can be ready for
 *next interrupt.The routine loads the new value for COMP0 based on the timing requirement every time it is invoked. The routine then
 *sets the negative reference level for ACMP based on the current ACMP_OUT and excite_flag. (This new reference level makes sure that
 *LED is ON till the VDD_HIGH_REFERENCE is reached). The handler also start the ADC every 4seconds and activate the DMA to transfer
 *1000 samples in to the RAM. If DMA is not on, ADC is polled until it converts the 1000 samples and average of the samples is taken
 * in the interrupt to make the temperature range decision.
 * args:None
 * return:None
 * Frequency: Interrupt handler is invoked every in 3 seconds. It is also invoked after 4ms to excite the ACMP //
 */

void LETIMER0_IRQHandler(void)
{
	int cnt_val = 0;
	INT_Disable();


	ACMP_OUT = ((ACMP0->STATUS & ACMP_STATUS_ACMPOUT) >> _ACMP_STATUS_ACMPOUT_SHIFT);					// Checking the ACMPOUT Value
	//LETIMER_IntClear(LETIMER0, LETIMER_IEN_COMP0);
	LETIMER0->IFC = LETIMER_IEN_COMP0;																	// Clear the interrupt flag for COMP0
	if (EnergyMode == EM2)																				// Check Energy Mode
	{
		if (excite_flag)																				// If 4ms interrupt
		{
			excite_flag --;
			GPIO_PinOutSet(gpioPortD,6);																// Excite the ACMP
			LETIMER0->CNT = 0;																			// Reinitialize the LETIME CNT
			//LETIMER_CompareSet(LETIMER0,0,(LETIMER_MAX_COUNT-(int)(EXCITE_PERIOD*LFXO_FREQ_PRESCALAR)));
			LETIMER0->COMP0 = LETIMER_MAX_COUNT-(int)(EXCITE_PERIOD*LFXO_FREQ_PRESCALAR);				// Reload the new value on COMP0
		}
		else																							// If 3 seconds interrupt
		{
			excite_flag ++;
			GPIO_PinOutClear(gpioPortD,6);																// Turn Off the ACMP excite channel
			LETIMER0->CNT = 0;																			// Reinitialize the LETIMER CNT
			//LETIMER_CompareSet(LETIMER0,0,count_value_EM2);
			LETIMER0->COMP0 = count_value_EM2;															// Reload the new value on COMP0
		}
	}
	else if (EnergyMode == EM3)																			// If EM3
	{
		if (excite_flag)																				// If 4ms interrupt
		{
			excite_flag --;
			GPIO_PinOutSet(ACMP_EXCITE_PORT,6);															// Excite the ACMP
			LETIMER0->CNT = 0;																			// Reinitialize the LETIME CNT
			//LETIMER_CompareSet(LETIMER0,0,(LETIMER_MAX_COUNT-(int)(EXCITE_PERIOD*ULFRCO_FREQ)));
			cnt_val = (int)(EXCITE_PERIOD * Calibrated_ULFRCO);
			LETIMER0->COMP0 = LETIMER_MAX_COUNT- cnt_val/2; 											// Reload the new value on COMP0
																										// cnt_val/2 because of int typecast//
		}
		else
		{
			excite_flag ++;
			GPIO_PinOutClear(ACMP_EXCITE_PORT,6);														// Turn Off the ACMP excite channel
			LETIMER0->CNT = 0;
			//LETIMER_CompareSet(LETIMER0,0,count_value_EM3);
			LETIMER0->COMP0 = count_value_EM3;
		}
	 }

	if(excite_flag && ACMP_OUT)																			// Checking the excitation
	{
		GPIO_PinOutClear(LED,LED0);																	// Clearing the ACMP Channel
		//ACMP0->INPUTSEL |=0x00000200;
		acmp_init.vddLevel = VDD_LOW_REFERENCE;															// Changing the reference
		ACMP_Init(ACMP0,&acmp_init);																	// Reinitialize the ACMP
		ACMP_ChannelSet(ACMP0,ACMP_NEGATIVE_CHANNEL,ACMP_POSITIVE_CHANNEL);								// Reset the Channel
	}
	else if (excite_flag && !ACMP_OUT)
	{
		GPIO_PinOutSet(LED,LED0);																	// Exciting the ACMP channel
		//ACMP0->INPUTSEL |= 0x00003D00;
		acmp_init.vddLevel = VDD_HIGH_REFERENCE;														// Changing the reference
		ACMP_Init(ACMP0,&acmp_init);																	// Reinitialize the ACMP
		ACMP_ChannelSet(ACMP0,ACMP_NEGATIVE_CHANNEL,ACMP_POSITIVE_CHANNEL);								// Reset the channel
	}


	if(excite_flag)																					// If 4sec interrupt
	{
		blockSleepMode(EM1);																		// Block EM1 so that ADC can work
#if defined(DMA_ON)																					// If using DMA
		DMA_ActivateBasic(DMA_CHANNEL_ADC,															// Activate the DMA to start transfer
		                    true,
		                    false,
		                    (void *)BufferAdcData,
		                    (void *)&(ADC0->SINGLEDATA),
		                    ADC_SAMPLES - 1);

		ADC_Start(ADC0,ADC_SINGLE_CONVERSION);																// Start the ADC

#else
		ADC_Start(ADC0,ADC_SINGLE_CONVERSION);
#endif
	}
	INT_Enable();																					// Enabling the interrupts
}



/*
 * Clock Initialization routine
 * Description: This function initializes the oscillator and clock tree based on the current energy mode of leopard gecko. It also
 * enable the recently initialized clock tree to the LETIMER0 peripheral which is used to give sense ambient light sensor
 * Args: Enum for the energy modes
 * Return: None
 * Frequency: Once (During Initialization)
 */
void clock_init(sleepstate_enum EMx)
{
	if(EMx == EM2)																			// Check the current energy mode
	{
		CMU_OscillatorEnable(cmuOsc_LFXO,true,true);										// Enable the LFXO oscillator for EM2
		CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_LFXO);									// Selecting the LFA clock tree
	}
	else if (EMx == EM3)																	// Check the current energy mode
	{
		CMU_OscillatorEnable(cmuOsc_ULFRCO,true,true);										// Enable the ULFRCO for EM3
		CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_ULFRCO);									// Selecting the LFA clock tree
	}
	CMU_ClockEnable(cmuClock_HFPER,true);									//Enabling the High Frequency peripheral
	CMU_ClockEnable(cmuClock_CORELE,true);													// Enable the CORELE
}

/*
 * LETIMER0 Initialization routine
 * Description: This function initializes and enable the LETIMER0 peripheral. It also enables the interrupts of the LETIEMR based on the
 * energy mode requirement. The prescalar is set for LETIMER0 to increase the time period as per requirement. The COMP0 interrupt is
 * set such that it occurs every 3secs based on the input energy mode. The routine also enables the nested interrupt vector for LETIMER0 interrupts.
 * The function blocks also the energy mode based on the input argument.
 * Args: Enum for the energy modes
 * Return: None
 * Frequency: Once (During Initialization)
*/
void letimer_initialize(sleepstate_enum EMx)
{

	blockSleepMode(EMx);															// Block the energy mode
	CMU_ClockEnable(cmuClock_LETIMER0,true);										// Enable the clock for LETIMER0 peripheral
	LETIMER_Init_TypeDef leTimer = LETIMER_INIT_DEFAULT;							// Initialize the LETIMER struct with default value

	LETIMER_IntClear(LETIMER0,LETIMER0->IFC);										// Clear the LETIMER interrupts

	LETIMER_IntEnable(LETIMER0,LETIMER_IF_COMP0);									// Enable the COMP0 interrupt
	//LETIMER_IntEnable(LETIMER0,LETIMER_IF_COMP1);
	if (EMx == EM2)
	{
		CMU->LFAPRESC0 |= CMU_LFAPRESC0_LETIMER0_DIV2; 						// Setting the prescalar such that LFXO frequency becomes half
		count_value_EM2 = TIMER_PERIOD*LFXO_FREQ_PRESCALAR/1000;
		count_value_EM2 = LETIMER_MAX_COUNT - count_value_EM2;				// Calculate the COMP0 value based on given timeperiod
		LETIMER_CompareSet(LETIMER0,0,count_value_EM2);						// Set the value for COMP0
	}
	else if(EMx == EM3)
	{
#if defined(CALIBRATION)
		count_value_EM3 = (int)(TIMER_PERIOD* Calibrated_ULFRCO/1000);
#else
		count_value_EM3 = (int)(TIMER_PERIOD* ULFRCO_FREQ/1000);
#endif
		count_value_EM3 = LETIMER_MAX_COUNT - count_value_EM3;			 	// Calculate the COMP0 value based on given timeperiod
		LETIMER_CompareSet(LETIMER0,0,(count_value_EM3));				    // Set the value for COMP0
	}
	LETIMER_Init(LETIMER0, &leTimer);													// Initialize the LETIMER
	NVIC_EnableIRQ(LETIMER0_IRQn);														// Enable the Interrupt vector
	LETIMER_Enable(LETIMER0,true);														// Enable the LETIMER0
}

/*
 * ACMP initialization function
 * Description: This function initializes the ACMP as per the requirement
 * Args: sleepstate_enum for the energy mode
 * Return: None
 * Frequency: Once (To Initialize)
 */
void ACMPInit(sleepstate_enum EMx)
{

	CMU_ClockEnable(cmuClock_ACMP0,true);								// Enable the clock for ACMP0 peripheral
	ACMP_Init(ACMP0,&acmp_init);										// Initializing the ACMP
	ACMP_ChannelSet(ACMP0,ACMP_NEGATIVE_CHANNEL,ACMP_POSITIVE_CHANNEL);

	ACMP_IntClear(ACMP0, ACMP0->IFC);									// Clearing the Interrupt

	ACMP_IntEnable(ACMP0,ACMP_IEN_EDGE);


	while (!(ACMP0->STATUS & ACMP_STATUS_ACMPACT)) ;					// Check if ACMP has been warmed up
	NVIC_ClearPendingIRQ(ACMP0_IRQn);									// Clearing the Pending interrupt
	NVIC_EnableIRQ(ACMP0_IRQn);											// Enable the IVT for ACMP
	//blockSleepMode(EMx);												// block sleep mode
}

/*
 * GPIO initialization function
 * Description: This function initializes the GPIO
 * Args: None
 * Return: None
 * Frequency: Once (To Initialize)
 */
void GPIO_init(void)
{
	CMU_ClockEnable(cmuClock_GPIO, true);								// Enable the clock to GPIO
	GPIO_PinModeSet(LED,LED0,gpioModePushPull,0);					    // Set the GPIO PIN for the LED 0
	GPIO_PinModeSet(LED,LED1,gpioModePushPull,0);					    // Set the GPIO PIN for the LED 1
	GPIO_PinModeSet(ACMP_EXCITE_PORT,6,gpioModePushPull,0);				// Set the ACMP Excite GPIO
	GPIO_PinModeSet(ACMP_SENSE_PORT,6,gpioModeDisabled,0);				// Disable the ACMP sense from PushPull Mode//
}


/*
 * TIMER initialization function
 * Description: This function initializes the timer to run in a 32 bit cascaded mode. Timer 0 is initialized to run
 * on High Frequency Clock and TIMER1 is clocked by overflow of TIMER0. In this way we can get 32 bit timing period.
 * Args: None
 * Return: None
 * Frequency: Once (To Initialize)
 */
void TIMER_SETUP(void)
{
	CMU_ClockEnable(cmuClock_TIMER0,true);									//Enable the clock for TIMER0
	CMU_ClockEnable(cmuClock_TIMER1,true);									//Enable the clock for TIMER1

	// Initialize the struct for timer0 which runs high frequency peripheral
	TIMER_Init_TypeDef timer0_init =
	{
			.enable      = false,										// Don't start on init
			.debugRun    = false,
			.prescale    = timerPrescale1,
			.clkSel      = timerClkSelHFPerClk,							// Run on HFPER
			.fallAction  = timerInputActionNone,
			.riseAction  = timerInputActionNone,
			.mode        = timerModeUp,									// Count Upwards
			.dmaClrAct   = false,
			.quadModeX4  = false,
			.oneShot     = false,
			.sync        = false
	};

	// Initialize the timer1 structure which runs on overflow of timer 0
	TIMER_Init_TypeDef timer1_init =
	{
			.enable      = false,										// Don't start on init
			.debugRun    = false,
			.prescale    = timerPrescale1,
			.clkSel      = timerClkSelCascade,							// Run on overflow of timer0
			.fallAction  = timerInputActionNone,
			.riseAction  = timerInputActionNone,
			.mode        = timerModeUp,									// Count Upwards
			.dmaClrAct   = false,
			.quadModeX4  = false,
			.oneShot     = false,
			.sync        = true											// Enable to be clocked by another timer
	};

	TIMER_Init(TIMER0, &timer0_init);									// Initialize the timers
	TIMER_Init(TIMER1, &timer1_init);
	TIMER_IntClear(TIMER0,TIMER0->IFC);									// Clear and enables the interrupt
	TIMER_IntEnable(TIMER0, TIMER_IF_OF);
	NVIC_EnableIRQ(TIMER0_IRQn);

}


/*
 * LETIMER calibration function
 * Description: This function is used to calibrate the ULFRCO using LETIMER and TIMERs. The LETIMER is used to count
 * 1 seconds running on LFXO and ULFRCO. At the end of 1 seconds, the count of teh 32 bit TIMER (Accurate) is stored
 * for both LFXO and ULFRCO. The ratio of these count gives the offset in the ULFRCO. New ULFRCO is calculated using
 * the ratio*1000.
 * Args: None
 * Return: None
 * Frequency: Once (To Calibrate)
 */
void LETIMER_Calibration(void)
{
	uint32_t LFXO = 0;
	uint32_t ULFRCO = 0;
	blockSleepMode(EM1);															// block the EM1
	TIMER_SETUP();
	CMU_OscillatorEnable(cmuOsc_LFXO,true,true);									// Enable the LFXO oscillator for EM2
	CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_LFXO);								// Selecting the LFA clock tree
	CMU_ClockEnable(cmuClock_CORELE,true);
	CMU_ClockEnable(cmuClock_LETIMER0,true);										// Enable the clock for LETIMER0 peripheral
	LETIMER_Init_TypeDef leTimer = LETIMER_INIT_DEFAULT;							// Initialize the LETIMER struct with default value

	leTimer.repMode = letimerRepeatOneshot;											// LETIMER Should run only once
	leTimer.comp0Top = true;														// COMP0 value should be top value
	leTimer.enable   = false;														// Don't Enable the timer on init
	LETIMER0->COMP0 = CALIBRATION_COUNT_LFXO;														// Setting COMP0 Value for 1 secs

	LETIMER_Init(LETIMER0, &leTimer);												// Initialize the LETIMER
	LETIMER0->CNT = CALIBRATION_COUNT_LFXO;															// Loading the CNT value for 1 sec
	LETIMER_Enable(LETIMER0,true);													// Enable the LETIMER
	TIMER_Enable(TIMER0,true);														// Enable the TIMER0
	TIMER_Enable(TIMER1,true);														// Enable the TIMER1
	LFXO = LETIMER0->CNT;
	while(LFXO != 0)																// Polling te LETIMER
	{
		LFXO = LETIMER0->CNT;
	}
	LFXO = TIMER0->CNT + TIMER1->CNT * MAX_COUNT;									// Checking the count of Timer after 1 sec


	CMU_OscillatorEnable(cmuOsc_LFXO,false,true);									// Disabling the LFXO oscillator
	CMU_OscillatorEnable(cmuOsc_ULFRCO,true,true);									// Enable the ULFRCO
	CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_ULFRCO);


	TIMER_Enable(TIMER0,false);														// Disabling the timers
	TIMER_Enable(TIMER1,false);

	LETIMER0->COMP0 = CALIBRATION_COUNT_ULFRCO;															// Loading the COMP0 with 1 sec value w.r.t ULFRCO
	//LETIMER_Init(LETIMER0, &leTimer);
	LETIMER0->CNT = CALIBRATION_COUNT_ULFRCO;
	TIMER0->CNT = 0;																// Reinitializing the timers
	TIMER1->CNT = 0;

	LETIMER_Enable(LETIMER0,true);													// Renabling the timers
	TIMER_Enable(TIMER0,true);
	TIMER_Enable(TIMER1,true);

	ULFRCO = LETIMER0->CNT;															// Checking the timer count after 1 secs counted by ULFRCO
	while(ULFRCO != 0)
	{
		ULFRCO = LETIMER0->CNT;
	}
	ULFRCO = TIMER0->CNT + TIMER1->CNT * MAX_COUNT;

	//ULFRCO = TIMER1->CNT;
	Calibrated_ULFRCO = ((float)LFXO/ULFRCO) * ULFRCO_FREQ;							// Calculating the calibration ratio

	TIMER_Enable(TIMER0,false);														// Disable the timers
	TIMER_Enable(TIMER1,false);														// Disable the timers
	CMU_OscillatorEnable(cmuOsc_ULFRCO,false,true);									// Disabling the ULFRCO
	unblockSleepMode(EM1);															// Unblock the EM1

}


/*
 * ADC Configuration function
 * Description: This function initializes the ADC to run in a single conversion mode. ADC converts the temperature sensor readings
 * in to digital samples. The ADC is configured to convert 10ksps with 12 bit resolution.
 * Args: None
 * Return: None
 * Frequency: Once (To Initialize)
 */
void adcConfig()
{
	CMU_ClockEnable(cmuClock_ADC0,true);								// Enable the clock for ACMP0 peripheral

	ADC_Init_TypeDef adc_init =											// Initializing the ADC structure//
	{
		.lpfMode 		= adcLPFilterBypass,							// No filter
		.ovsRateSel	 	= 0,											// No Oversampling
		.prescale       = prescale10ksps,								// 107 prescalar
		.tailgate       = false,										// No Tail-gating
		.warmUpMode     = adcWarmupNormal,								// Don't warm up ADC b/w conversions
		.timebase       = _ADC_CTRL_TIMEBASE_DEFAULT					// Default time base for timing of ADC
	};

	ADC_Init(ADC0,&adc_init);											// Initialize the ADC

	ADC_InitSingle_TypeDef  adcInitSingle =								// Initialize the ADC single covnersion structure
	{
		.acqTime 		= adcAcqTime1,									// 1 ADC cycle acquisiton time
		.diff			= false,										// No differential mode
		.input 			= ADC_TEMP_CHANNEL,								// Temperature sensor input channel
		.leftAdjust	 	= false,										// No left Adjustment
		.prsEnable 		= false,										// PRS channel trigger disabled
		.prsSel 		= adcPRSSELCh0,									// Set PRS Channel 0 to use (Disabled)
		.reference 		= adcRef1V25,									// 1.25 V as reference channel
		.rep			= true,											// rep mode true
		.resolution 	= adcRes12Bit									// 12bit Resolution
	};

	ADC_InitSingle(ADC0,&adcInitSingle);								// Initialize the structure

#if !defined (DMA_ON)													// If DMA is not used enable the ADC interrupts
	ADC_IntClear(ADC0,ADC_IFC_SINGLE);									// Clear the ADC interrupts
	ADC_IntEnable(ADC0,ADC_IEN_SINGLE);									// Enable the ADC SINGLE CONVERSION INTERRUPT
	NVIC_ClearPendingIRQ(ADC0_IRQn);									// Clearing the Pending interrupt
	NVIC_EnableIRQ(ADC0_IRQn);											// Enable the IVT for ADC
#endif
}

/*
 * DMA Configuration Function
 * Description: This routine configures the DMA to transfer 1000 samples from ADC0 to the RAM. DMA channel 0 is used to transfer the data.
 * ADC Channel is set to high priority. Arbritration is set to 0 as only single peripheral is using DMA (No contention)
 * args: None
 * Return: None
 * Frequency: Once
 * Disclaimer: Only Called if DMA is used
 */
void dmaConfig(void)
{
	CMU_ClockEnable(cmuClock_DMA, true);
	/* Initializing DMA descriptors */
	DMA_Init_TypeDef		dmaInit;
	DMA_CfgChannel_TypeDef	chnlCfg;
	DMA_CfgDescr_TypeDef	descrCfg;

	dmaInit.hprot		 = 0;
	dmaInit.controlBlock = dmaControlBlock;
	DMA_Init(&dmaInit);																// Initializing the DMA

	callback.cbFunc  	 = transferComplete;										// Call back function on completion of DMA transfer
	callback.userPtr 	 = NULL;

	chnlCfg.cb 		 	 = &callback;
	chnlCfg.enableInt	 = true;													// Enable the interuupt on completion of DMA transfer
	chnlCfg.highPri      = true;													// set ADC as high priority channel
	chnlCfg.select       = DMAREQ_ADC0_SINGLE;										// ADC channel as a source for DMA signals
	DMA_CfgChannel(DMA_CHANNEL_ADC, &chnlCfg);										// Configuration of DMA channels

	descrCfg.arbRate	 = 0;														// No arbritration, since, ADC is only peripheral using DMA
	descrCfg.dstInc		 = dmaDataInc2;												// 2 bytes of address increment at destination (12 bit ADC)
	descrCfg.srcInc		 = dmaDataIncNone;											// No address increment at source (ADC buffer)
	descrCfg.size        = dmaDataSize2;											// 2 bytes of data transfer
	descrCfg.hprot		 = 0;
	DMA_CfgDescr(DMA_CHANNEL_ADC, true, &descrCfg);									// Configuration of DMA


}




/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  /* Chip errata */

  CHIP_Init();
  blockSleepMode(EM3);												// Block EM4, so that processor never goes below EM3
#if defined(CALIBRATION)											// If calibration on
  LETIMER_Calibration();											// Calibrate the ULFRCO
#endif
  GPIO_init();														// Configuring the GPIO Pins
  adcConfig();														// Configuring the ADC
#if defined(DMA_ON)													// If DMA is used
  dmaConfig();														// Configuring the DMA
#endif
  ACMPInit(EnergyMode);												// Initializing the ACMP
  clock_init(EnergyMode);							 				// Initializing the Oscillators and Clock trees
  letimer_initialize(EnergyMode);									// Initializing the LETIMER0


  /* Infinite loop */
  while (1) {
	  sleep();														// Going in to sleep. Entered EM is based on the blocked EM
  }
}
