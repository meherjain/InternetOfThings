
/*
 * Name: Meher Jain
 * Email ID: meher.jain@colorado.edu
 * Date: 09/07/2016
 * Description: Assignment-1 ECEN 5023
 * This Code blinks the LED for 50ms every 2 seconds using LETIMER0 peripheral. The code supports processor going in to 4 energy modes
 * EM0,EM1,EM2,EM3. The Processor sleeps for most of the time and is only awaken by LETIMER0 interrupts to blink the LED.
 *
 */



#include "sleep.h"

/*
 *LETIMER interrupt handler routine
 *Description: This routine is executed when LETIMER interrupt occurs. In the routine, the further interrupts are disabled at the start
 *and are enabled at the end such that, no more interrupts can occur when the handler is executing. The routine checks the energy mode
 *processor is at clears the appropriate interrupts that invoked the handler in the first place. In this way processor can be ready for
 *next interrupt. After clearing the interrupts, GPIO LED is toggeled.
 *
 * args:None
 * return:None
 * Frequency: Interrupt handler is invoked two times in 2 seconds. To turn 0n-Off the LED.
 */

void LETIMER0_IRQHandler(void)
{
	INT_Disable();															// Disabling the Interrupts
	if (EnergyMode == EM3)
	{
		LETIMER_IntClear(LETIMER0,LETIMER_IF_COMP1);						// Clearing the COMP1 interrupt for EM3
		LETIMER_IntClear(LETIMER0,LETIMER_IF_UF);
	}
	else
		LETIMER_IntClear(LETIMER0,LETIMER_IF_UF);							// Clearing the UF interrupt for EM0,EM1,EM2

	LETIMER_IntClear(LETIMER0,LETIMER_IF_COMP0);							// Clearing the COMP0 interrupt for all cases
	GPIO_PinOutToggle(gpioPortE,2);											// Toggling the GPIO Port
	INT_Enable();															// Enabling the interrupts

	/*
	if(LETIMER_IF_COMP0 == 1){
		GPIO_PinOutSet(gpioPortE,2);
		LETIMER_IntClear(LETIMER0,LETIMER_IF_COMP0);
	}
	else{
		GPIO_PinOutClear(gpioPortE,2);
		LETIMER_IntClear(LETIMER0,LETIMER_IF_UF);
		LETIMER_IntClear(LETIMER0,LETIMER_IF_COMP1);
	}*/
}

/* Turn on the LED
 * Description: This routine turns the LEDon
 * Args: None
 * Return: Nome
 */
void LED_ON(void)
{
	GPIO_PinOutSet(gpioPortE,2);
}

/* Turn Off the LED
 * Description: This routine turns the LEDOFF
 * Args: None
 * Return: None
 */
void LED_OFF(void)
{
	GPIO_PinOutClear(gpioPortE,2);
}


/*
 * GPIO initialization function
 * Description: This function initializes the GPIO
 * Args: None
 * Return: None
 * Frequency: Once (To Initialize)
 */
void GPIO_enable()
{
	CMU_ClockEnable(cmuClock_GPIO, true);								// Enable the clock to GPIO
	GPIO_PinModeSet(gpioPortE,2,gpioModePushPull,0);					// Set the GPIO PIN
	if(EnergyMode == EM3)
		  LED_ON();														// If in EM3, set the GPIO pin to 1 (LED ON)
}

/*
 * Clock Initialization routine
 * Description: This function initializes the oscillator and clock tree based on the current energy mode of leopard gecko. It also
 * enable the recently initialized clock tree to the LETIMER0 peripheral which is used to blink LED
 * Args: Enum for the energy modes
 * Return: None
 * Frequency: Once (During Initialization)
 */
void clock_init(sleepstate_enum EMn)
{
	if(EMn == EM0 || EMn == EM1 || EMn == EM2)										// Check the current energy mode
	{
		CMU_OscillatorEnable(cmuOsc_LFXO,true,true);								// Enable the LFXO oscilator for EM0,1,2
		CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_LFXO);							// Selecting the LFA clock tree
	}
	else
	{
		CMU_OscillatorEnable(cmuOsc_ULFRCO,true,true);								// Enable the ULFRCO for EM3
		CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_ULFRCO);							// Selecting the LFA clock tree
	}
	CMU_ClockEnable(cmuClock_CORELE, true);											// Enable the CORELE
	CMU_ClockEnable(cmuClock_LETIMER0,true);										// Enable the clock for LETIMER0 peripheral
}

/*
 * LETIMER0 Initialization routine
 * Description: This function initializes and enable the LETIMER0 peripheral. It also enables the interrupts of the LETIEMR based on the
 * energy mode requirement. The counter for the LETIMER0 interrupts is selected based on the 50ms ON-duty cycle of the LED for all the
 * energy modes. The routine also enables the nested interrupt vector for LETIMER0 interrupts. The fucntion blocks the energy mode based on
 * the input argument
 * Args: Enum for the energy modes
 * Return: None
 * Frequency: Once (During Initialization)
 * LETIMER Interrupts used: COMP0 for EM0,EM1,EM2,EM3
 * 							UF    for EM0,EM1,EM2
 * 							COMP1 for EM3
 */
void letimer_initialize(sleepstate_enum EMn)
{
	int count_value = 0 ;
	uint32_t count_value_comp1 = 0;
//	blockSleepMode(EMn);																// Block the energy mode
	LETIMER_Init_TypeDef leTimer = LETIMER_INIT_DEFAULT;								// Initialize the LETIMER struct with default value

	LETIMER_IntClear(LETIMER0,LETIMER0->IFC);											// Clear the LETIMER interrupts
	LETIMER_IntEnable(LETIMER0,LETIMER_IF_COMP0);										// Enable the COMP0 interrupt

	if (EMn == EM0 || EMn == EM1 || EMn == EM2)											// Check the energy mode
	{

		count_value = (LED_BLINK_PERIOD-ON_DUTY_CYCLE)*LFXO_FREQ/1000;
		count_value = LETIMER_MAX_COUNT - count_value;
		LETIMER_CompareSet(LETIMER0,0,count_value);											     // Set the Compare Value in COMP0 register
		LETIMER_IntEnable(LETIMER0,LETIMER_IF_UF);										         // Enable the Under FLow interrupt
	}
	else																				// If EM3
	{
		leTimer.comp0Top  = true;														// set COMP0Top true, so that COMP0 value act as the top value
		count_value       =  LED_BLINK_PERIOD * ULFRCO_FREQ/1000;						// Cnt value for the COMP0
        count_value_comp1 =  ON_DUTY_CYCLE * ULFRCO_FREQ/1000;							// Cnt value for the COMP1 register

		LETIMER_CompareSet(LETIMER0,0,(count_value+ULFRCO_ADJUST_PERIOD));				// Set the count for COMP0 register (Had to be adjusted because ULFRCO is unstable)
		LETIMER_CompareSet(LETIMER0,1,(count_value_comp1 + ULFRCO_ADJUST_ON_CYCLE));	// Set the count for COMP1 register (Had to be adjusted because ULFRCO is unstable)
		LETIMER_IntEnable(LETIMER0,LETIMER_IF_COMP1);									// Enable the COMP1 interrupt
	}
	LETIMER_Init(LETIMER0, &leTimer);													// Initialize the LETIMER
	CMU->LFAPRESC0 |= CMU_LFAPRESC0_LETIMER0_DIV2;
	NVIC_EnableIRQ(LETIMER0_IRQn);														// Enable the Interrupt vector
	LETIMER_Enable(LETIMER0,true);														// Enable the LETIMER0

}


/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/

int main(void)
{
  /* Chip errata */
  CHIP_Init();
  blockSleepMode(EM3);									   // Block EM4, so that processor never goes below EM3


  GPIO_enable();											   // Initializing the GPIO peripherals (LED)
  clock_init(EnergyMode);									   // Initializing the Oscillators and Clock trees
  letimer_initialize(EnergyMode);							   // Initializing the LETIMER as per requirement


  /* Infinite loop */
  while (1) {
	  sleep();												   // Going in to sleep. Entered EM is based on the blocked EM
  }
}
