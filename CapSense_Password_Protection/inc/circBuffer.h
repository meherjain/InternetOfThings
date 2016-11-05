/*
 * circBuffer.h
 *
 *  Created on: Oct 29, 2016
 *      Author: Meher
 */

#ifndef CIRCBUFFER_H_
#define CIRCBUFFER_H_


#include "main.h"



typedef struct
{
	uint8_t *circularBuffer;
	volatile uint8_t tail;
	volatile uint8_t head;
}circBuff;


#define CIRCBUFFSIZE   14

void circBufferInit();
bool circBufferPush(circBuff*, uint8_t* , uint8_t);
void circBufferPop();
bool circBufferIsEmpty(circBuff*);
void circBufferIsFull();


#endif /* CIRCBUFFER_H_ */
