/***************************************************************************//**
 * @Function sleep.c
 * 			 unblockSleepMode.c
 * 			 blockSleepMode.c
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

# include "sleep.h"


int	sleep_block_counter[4] = {0,0,0,0}; 							// Global sleep counter//


/*
 * Unblocking the sleep mode leopard gecko can go in to.
 * Description: This function unblocks the minimum sleep mode for leopard gecko. Interrupts are disable and enabled at the start and end
 * of the function respectively. The function checks for the block counter for the current energy mode, if it is greater that zero
 * (sleepmode is blocked),decrement it (unblock sleep mode).
 * Args: enum for the energy modes
 * return: None
 * Disclaimer: This routine has been taken from mbed-src library function.
 * Frequency: As requirement
 */

void unblockSleepMode(sleepstate_enum minimumMode)
{
	INT_Disable();												    // Disable the interrupt
	if (sleep_block_counter[minimumMode]>0)							// Check if Energy mode is blocked or not
	{
		sleep_block_counter[minimumMode]--;							// Unblocking the energy mode
	}
	INT_Enable();												    // Enable the Interrupt
}



/*
 * Blocking the sleep mode leopard gecko can go in to.
 * Description: This function blocks the minimum sleep mode for leopard gecko. Interrupts are disable and enabled at the start and end
 * of the function respectively. The function increment the counter(block)for the minimum energy mode
 * Args: Enum for the energy modes
 * return: None
 * Frequency: As requirement
 * Disclaimer: This routine has been taken from mbed-src library function.
 */
void blockSleepMode (sleepstate_enum minimumMode)
{
	INT_Disable();													// Disable the interrupts
	sleep_block_counter[minimumMode]++;								// Increment the sleep counter (block the energy mode)
	INT_Enable();													// Enable the interrupts
}



/*
 * Sleep Function
 * Description: This function puts the processor in to the sleep mode based on the blocked sleep mode by blockSleepMode function. The
 * function finds the blocked energy mode by checking the global variable sleep_block_counter and put the processor in to appropriate
 * energy mode.
 * Args: None
 * return: None
 * Frequency: Indefinite
 * Disclaimer: This routine has been taken from mbed-src library function sleep();
 */

void sleep (void)
{
	if (sleep_block_counter[EM0] > 0){									// If EM1 is blocked, return for default (EM0) mode
		return;
	}
	else if (sleep_block_counter[EM1]>0){								// If EM2 is blocked
		EMU_EnterEM1();													// Enter EM1 mode
	}
	else if (sleep_block_counter[EM2]>0){								// If EM3 is blocked
		EMU_EnterEM2(true);												// Eneter EM2 mode
	}
	else if (sleep_block_counter[EM3]>0){								// If EM4 is blocked
		EMU_EnterEM3(true);												// Enter EM3 mode
	}
	else																// If nothing is blocked
		EMU_EnterEM4();													// Enter deep sleep (EM4) mode

}



