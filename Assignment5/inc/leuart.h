/*
 * leuart.h
 *
 *  Created on: Oct 17, 2016
 *      Author: Meher
 */

#ifndef LEUART_H_
#define LEUART_H_

#include "main.h"
#include "sleep.h"

#define LEUART_TX_PORT		gpioPortD
#define LEUART_TX_PIN		4
#define LEUART_RX_PORT		gpioPortD
#define LEUART_RX_PIN		5

#define LEUART_GPIO_Tx_MODE	gpioModePushPull
#define LEUART_GPIO_Rx_MODE	gpioModeInputPull

#define LEUART_BAUD_RATE	9600
#define LEUART_DATA_BITS	leuartDatabits8
#define LEUART_PARITY		leuartNoParity
#define LEUART_STOP_BITS	leuartStopbits1
#define LEUART_ENABLE_RxTx	leuartEnable

#define LEUART_TX_SIZE		6



void leuart_pin_initialize(void);
void leuart_initialize(void);
void leuart_tx (uint8_t*);
uint8_t leuart_rx(void);
void dmaconfig_LEUART(void);

#endif /* LEUART_H_ */
