
/*
 * Name: Meher Jain
 * Email ID: meher.jain@colorado.edu
 * Date: 09/11/2016
 * Description: Assignment-2 ECEN 5023 */


/***************************************************Include Header Files ******************************/

#include "sleep.h"

/********************************************Declaring the global constants ***************************/
uint32_t ACMP_OUT             = 0;										// ACMP Output //
uint32_t LETIMER_COMP0        = 0;										// LETIMERE_COMP0 Register Value */
uint32_t count_value_EM2      = 0;										// COMP0 count value for EM2
uint32_t count_value_EM3      = 0;										// COMP0 count value for EM3
uint32_t excite_flag 	      = 1;										// excite flag for ACMP excitation
uint32_t Calibrated_ULFRCO    = 0;


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



void TIMER0_IRQHandler(void)
{
	TIMER_IntClear(TIMER0,TIMER_IF_OF);
}

void ACMP0_IRQHandler(void){

	ACMP0->IFC = ACMP_IFC_EDGE;
	//GPIO_PinOutToggle(gpioPortE,3);
}

/*
 *LETIMER interrupt handler routine
 *Description: This routine is executed when LETIMER interrupt occurs. In the routine, the further interrupts are disabled at the start
 *and are enabled at the end such that, no more interrupts can occur when the handler is executing. The routine checks the energy mode
 *processor is at clears the appropriate interrupts that invoked the handler in the first place. In this way processor can be ready for
 *next interrupt.The routine loads the new value for COMP0 based on the timing requirement every time it is invoked. The routine then
 *sets the negative reference level for ACMP based on the current ACMP_OUT and excite_flag. (This new reference level makes sure that
 *LED is ON till the VDD_HIGH_REFERENCE is reached) *
 * args:None
 * return:None
 * Frequency: Interrupt handler is invoked every in 3 seconds. It is also invoked after 4ms to excite the ACMP //
 */

void LETIMER0_IRQHandler(void)
{
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
			GPIO_PinOutSet(ACMP_EXCITE_PORT,6);																// Excite the ACMP
			LETIMER0->CNT = 0;																			// Reinitialize the LETIME CNT
			//LETIMER_CompareSet(LETIMER0,0,(LETIMER_MAX_COUNT-(int)(EXCITE_PERIOD*ULFRCO_FREQ)));
			LETIMER0->COMP0 = LETIMER_MAX_COUNT-(int)(EXCITE_PERIOD*Calibrated_ULFRCO/2);						// Reload the new value on COMP0
		}
		else
		{
			excite_flag ++;
			GPIO_PinOutClear(ACMP_EXCITE_PORT,6);																// Turn Off the ACMP exite channel
			LETIMER0->CNT = 0;
			//LETIMER_CompareSet(LETIMER0,0,count_value_EM3);
			LETIMER0->COMP0 = count_value_EM3;
		}
	 }

	if(excite_flag && ACMP_OUT)
	{
		GPIO_PinOutClear(gpioPortE,2);
		//ACMP0->INPUTSEL |=0x00000200;
		acmp_init.vddLevel = VDD_LOW_REFERENCE;
		ACMP_Init(ACMP0,&acmp_init);
		ACMP_ChannelSet(ACMP0,ACMP_NEGATIVE_CHANNEL,ACMP_POSITIVE_CHANNEL);
	}
	else if (excite_flag && !ACMP_OUT)
	{
		GPIO_PinOutSet(gpioPortE,2);
		//ACMP0->INPUTSEL |= 0x00003D00;
		acmp_init.vddLevel = VDD_HIGH_REFERENCE;
		ACMP_Init(ACMP0,&acmp_init);
		ACMP_ChannelSet(ACMP0,ACMP_NEGATIVE_CHANNEL,ACMP_POSITIVE_CHANNEL);
	}


	INT_Enable();																											// Enabling the interrupts
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
		CMU->LFAPRESC0 |= CMU_LFAPRESC0_LETIMER0_DIV2;						// Setting the prescalar such that LFXO frequency becomes half
		count_value_EM2 = TIMER_PERIOD*LFXO_FREQ_PRESCALAR/1000;
		count_value_EM2 = LETIMER_MAX_COUNT - count_value_EM2;				// Calculate the COMP0 value based on given timeperiod
		LETIMER_CompareSet(LETIMER0,0,count_value_EM2);						// Set the value for COMP0
	}
	else if(EMx == EM3)
	{
#if defined(CALIBRATION)
		count_value_EM3 = TIMER_PERIOD* Calibrated_ULFRCO/1000;
#else
		count_value_EM3 = TIMER_PERIOD* ULFRCO_FREQ/1000;
#endif
		count_value_EM3 = LETIMER_MAX_COUNT - count_value_EM3;				// Calculate the COMP0 value based on given timeperiod
		LETIMER_CompareSet(LETIMER0,0,(count_value_EM3));				// Set the value for COMP0
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
	GPIO_PinModeSet(gpioPortE,2,gpioModePushPull,0);					// Set the GPIO PIN for the LED 0
	//GPIO_PinModeSet(gpioPortE,3,gpioModePushPull,0);					// Set the GPIO PIN for the LED 1
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
	CMU_ClockEnable(cmuClock_HFPER,true);									//Enabling the High Frequency peripheral
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
  GPIO_init();
  ACMPInit(EnergyMode);												// Initializing the ACMP
  clock_init(EnergyMode);							 				// Initializing the Oscillators and Clock trees
  letimer_initialize(EnergyMode);									// Initializing the LETIMER0




  /* Infinite loop */
  while (1) {
	  sleep();														// Going in to sleep. Entered EM is based on the blocked EM
  }
}
