
/*
 * Name: Meher Jain
 * Email ID: meher.jain@colorado.edu
 * Date: 09/18/2016
 * Description: All the fucntion related to I2C communication with TSL2561 Luminosity sensor
 * */




#include "circBuffer.h"


circBuff circbuff_t;



void circBufferInit()
{
	circbuff_t.circularBuffer = malloc(sizeof(uint8_t)* CIRCBUFFSIZE);
	circbuff_t.head = 0;
	circbuff_t.tail = 0;
}



bool circBuffIsEmpty(circBuff* cb)
{
	if (cb->head == cb->tail)
		return true;
	else
		return false;
}


bool circBuffIsFull(circBuff* cb)
{
	if((cb->head + 1)%CIRCBUFFSIZE == cb->tail)
		return true;
	else
		return false;
}

bool circBufferPush(circBuff* cb, uint8_t* data,uint8_t size)
{
	int i = 0;
	if(circBuffIsFull(cb))
		return false;

	for(i=0;i<size;i++)
	{
		cb->circularBuffer[cb->head] = data[i];
		cb->head = ((cb->head+1)%CIRCBUFFSIZE);
	}
	return true;
}

void circBufferPop()
{

	circbuff_t.tail = (circbuff_t.tail+1)%CIRCBUFFSIZE;
}
