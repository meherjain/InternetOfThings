

/*
 * Name: Meher Jain
 * Email ID: meher.jain@colorado.edu
 * Date: 09/18/2016
 * Description: All the fucntion related to I2C communication with APDS9960 Gesture/Proximity Sensor */


# include "APDS9960_i2c.h"



/*
 * I2C_Start Function
 * Description: Sends I2C Start Sequence with TSL2561 slave address. The routine uses ACK Pollin.
 * Args: None
 * Return: None
 */


void I2C_Start(void)
{

	I2C1->TXDATA = APDS9960_WRITE_COMMAND_BYTE;						// Send the slave address
	I2C1->CMD	 = I2C_CMD_START;							// Send Start Bit
	
	while (!(I2C1->IF & I2C_IF_ACK));						// Polling the ACK
	I2C1->IFC	= I2C_IFC_ACK;								// Clearing the ACK


}


/*
 * I2C Byte Write Function
 * Description: Follows Write-Write Protocol. This function writes the command register and data. The routine uses ACK Polling.
 * Args: command_register address, data
 * Return: None
 */

void I2C_WriteByte(uint8_t command_register, uint8_t write_data)
{
	I2C_Start();
	I2C1->TXDATA	= command_register;						// Sending the Command Register
	while (!(I2C1->IF & I2C_IF_ACK));						// Polling for the ACK
	I2C1->IFC = I2C_IFC_ACK;								// Clearing the ACK

	I2C1->TXDATA	= write_data;							// Write the data
	while (!(I2C1->IF & I2C_IF_ACK));						// Polling for the ACK
	I2C1->IFC = I2C_IFC_ACK;								// Clearing the ACK

	I2C1->CMD		= I2C_CMD_STOP;							// Sending stop bit to stop the transfer
	while (!(I2C1->IF & I2C_IF_MSTOP));						// Waiting for the stop bit to be sent
	I2C1->IFC   = I2C_IFC_MSTOP;
}



/*
 * I2C Byte Read Function
 * Description: Follows Write-Read Protocol. This function writes to the command register and sends the repeat start. After sucessfull
 * ACK, it writes the read control byte to the slave. The routine polls for the valid rx data available in the buffer and sends NACK and
 * stop bit to the slave to stop the transmission.
 * Args: command_register address
 * Return: I2C_Data
 */
uint8_t I2C_ReadByte(uint8_t command_register)
{
	uint8_t	read_data = 0;
	I2C_Start();
	I2C1->TXDATA	= command_register;				// Send command register
	while(!(I2C1->IF & I2C_IF_ACK));
	I2C1->IFC 		= I2C_IFC_ACK;

	I2C1->CMD		= I2C_CMD_START;				// Repeat Start
	I2C1->TXDATA	= APDS9960_READ_CONTROL_BYTE;	// Send Read Control Byte
	while(!(I2C1->IF & I2C_IF_ACK));
	I2C1->IFC 		= I2C_IFC_ACK;

	while(!(I2C1->IF & I2C_IF_RXDATAV));			// Polls for rx data to become valid
	read_data = I2C1->RXDATA;						// Reads the valid data

	I2C1->CMD		= I2C_CMD_NACK;					// Send NACK Bit

	I2C1->CMD		= I2C_CMD_STOP;					// Send Stop Bit
	while(!(I2C1->IF & I2C_IF_MSTOP));
	I2C1->IFC		= I2C_IFC_MSTOP;

	return read_data;
}



uint8_t I2C_PageRead(uint8_t command_register, uint8_t * fifo_data, uint8_t len)
{
	uint8_t i;
	I2C_Start();
	I2C1->TXDATA	=	command_register;			// Sending command
	while(!(I2C1->IF & I2C_IF_ACK));				// Pooling for ACK
	I2C1->IFC 		= I2C_IFC_ACK;					// Clearing ACK
	//I2C1->IFC	= I2C_IFC_NACK;						// Clearing NACK

	I2C1->CMD		= I2C_CMD_START;				// Repeat Start
	I2C1->TXDATA	= APDS9960_READ_CONTROL_BYTE;	// Send the Read Control Byte
	while(!(I2C1->IF & I2C_IF_ACK));				// Polling for ACK
	I2C1->IFC 		= I2C_IFC_ACK;					// Clearing ACK
	//I2C1->IFC	= I2C_IFC_NACK;						// Clearing NACK

	for (i=0;i<len-1;i++)
	{
		while(!(I2C1->IF & I2C_IF_RXDATAV));
		fifo_data[i] = I2C1->RXDATA;
		I2C1->CMD= I2C_CMD_ACK;
	}
	while(!(I2C1->IF & I2C_IF_RXDATAV));
			fifo_data[i] = I2C1->RXDATA;
//	for (i=0;i<I2C_DELAY;i++);
	I2C1->CMD = I2C_CMD_NACK;
	I2C1->CMD = I2C_CMD_STOP;
	while(!(I2C1->IF & I2C_IF_MSTOP));
	I2C1->IFC = I2C_IFC_MSTOP;					// Send Stop Bit

	return i;
}




void APDS9960_SetMode(uint8_t mode, uint8_t enable)
{
	uint8_t reg_value = 0;

	reg_value = I2C_ReadByte(APDS9960_ENABLE);


	enable = enable & 0x01;
	if(mode >=0 && mode <=6)
	{
		if(enable)
			reg_value |= (1 << mode);
		else
			reg_value |= ~(1 << mode);
	}
	else if (mode == ALL)
	{
		if(enable)
			reg_value = 0x7F;
		else
			reg_value = 0x00;
	}

	I2C_WriteByte(APDS9960_ENABLE,reg_value);
}


void APDS9960_setProximityGain(uint8_t drive)
{
	uint8_t value;

	value = I2C_ReadByte(APDS9960_PRO_GAIN_CONT);

	drive &= 0b00000011;
	drive = drive << 2;
	value &= 0b11110011;
	value |= drive;

	I2C_WriteByte(APDS9960_PRO_GAIN_CONT,value);
}


void APDSS9960_setLEDDrive(uint8_t drive)
{
	uint8_t value;
	value = I2C_ReadByte(APDS9960_PRO_GAIN_CONT);

	drive &= 0b00000011;
	drive = drive << 6;
	value &= 0b00111111;
	value |= drive;

	I2C_WriteByte(APDS9960_PRO_GAIN_CONT,value);
}


void APDS9960_setProximityIntEnable(uint8_t enable)
{
	uint8_t value;

	value = I2C_ReadByte(APDS9960_ENABLE);

	enable &= 0b00000001;
	enable = enable << 5;
	value  &= 0b11011111;
	value  |=enable;

	I2C_WriteByte(APDS9960_ENABLE,value);
}

void APDS9960_enablePower(void)
{
	APDS9960_SetMode(POWER,1);
}


uint8_t APDS9960_Proximity_Read()
{
	return (I2C_ReadByte(APDS9960_PRO_PDATA));
}


void APDS9960_Proximity_Threshold(uint8_t command_register, uint8_t threshold)
{
	I2C_WriteByte(command_register,threshold);
}




void APDS9960_Gesture_Threshold(uint8_t command_register, uint8_t threshold)
{
	I2C_WriteByte(command_register,threshold);
}


void APDS9960_setGestureGain(uint8_t gain)
{
	uint8_t value;

	value = I2C_ReadByte(APDS9960_GCONF2);

	gain &= 0b00000011;
	gain <<= 5;
	value &= 0b10011111;
	value |= gain;

	I2C_WriteByte(APDS9960_GCONF2,value);

}

void APDS9960_setGestureLEDDrive(uint8_t drive)
{
	uint8_t value;

	value = I2C_ReadByte(APDS9960_GCONF2);

	drive &= 0b00000011;
	drive <<= 3;
	value &= 0b11100111;
	value |= drive;

	I2C_WriteByte(APDS9960_GCONF2,value);
}



void APDS9960_setGestureWaitTime(uint8_t time)
{

	uint8_t value;

	value = I2C_ReadByte(APDS9960_GCONF2);

	time &= 0b00000111;
	value &= 0b11111000;
	value |= time;

	I2C_WriteByte(APDS9960_GCONF2,value);
}


void APDS9960_setGestureInterrupts(uint8_t enable)
{
	uint8_t value;

	value = I2C_ReadByte(APDS9960_GCONF4);

	enable &= 0b00000001;
	enable  = enable << 1;
	value  &= 0b11111101;
	value  |= enable;

	I2C_WriteByte(APDS9960_GCONF4, value);
}

void APDS9960_setLEDBoost(uint8_t boost)
{
	uint8_t value;

	value = I2C_ReadByte(APDS9960_PRO_CONFIG2);

	boost &= 0b00000011;
	boost = boost << 4;
	value &= 0b11001111;
	value |= boost;

	I2C_WriteByte(APDS9960_PRO_CONFIG2, value);

}

void APDS9960_SetGestureMode(uint8_t mode)
{
	uint8_t value;

	value = I2C_ReadByte(APDS9960_GCONF4);

	mode &= 0b00000001;
	value  &= 0b11111110;
	value  |= mode;

	I2C_WriteByte(APDS9960_GCONF4, value);
}



bool APDS9960_processGestureData(void)
{

    uint8_t u_first = 0;
	uint8_t d_first = 0;
	uint8_t l_first = 0;
	uint8_t r_first = 0;
	uint8_t u_last = 0;
	uint8_t d_last = 0;
	uint8_t l_last = 0;
	uint8_t r_last = 0;
	int ud_ratio_first;
	int lr_ratio_first;
	int ud_ratio_last;
	int lr_ratio_last;
	int ud_delta;
	int lr_delta;
	int i;

	if(gesture_data.total_gestures <=4){
		return false;
	}

	if((gesture_data.total_gestures <=32) && (gesture_data.total_gestures >0))
	{
		for(i=0; i<gesture_data.total_gestures; i++)
		{
			if( (gesture_data.u_data[i] > GESTURE_THRESHOLD_OUT) && \
				(gesture_data.d_data[i] > GESTURE_THRESHOLD_OUT) && \
				(gesture_data.l_data[i] > GESTURE_THRESHOLD_OUT) && \
				(gesture_data.r_data[i] > GESTURE_THRESHOLD_OUT) )
			{
                u_first = gesture_data.u_data[i];
                d_first = gesture_data.d_data[i];
                l_first = gesture_data.l_data[i];
                r_first = gesture_data.r_data[i];
                break;
			}
		}

		/* If one of the _first values is 0, then there is no good data */
		if( (u_first == 0) || (d_first == 0) || \
		    (l_first == 0) || (r_first == 0) )
		{
			return false;
		}

		for(i= gesture_data.total_gestures-1;i<=0; i--)
		{
			if( (gesture_data.u_data[i] > GESTURE_THRESHOLD_OUT) && \
				(gesture_data.d_data[i] > GESTURE_THRESHOLD_OUT) && \
				(gesture_data.l_data[i] > GESTURE_THRESHOLD_OUT) && \
				(gesture_data.r_data[i] > GESTURE_THRESHOLD_OUT) )
			{
				u_last = gesture_data.u_data[i];
				d_last = gesture_data.d_data[i];
				l_last = gesture_data.l_data[i];
				r_last = gesture_data.r_data[i];
				break;
			}
		}
	}

	/* Calculate the first vs. last ratio of up/down and left/right */
	ud_ratio_first = ((u_first - d_first) * 100) / (u_first + d_first);
	lr_ratio_first = ((l_first - r_first) * 100) / (l_first + r_first);
	ud_ratio_last = ((u_last - d_last) * 100) / (u_last + d_last);
	lr_ratio_last = ((l_last - r_last) * 100) / (l_last + r_last);

    /* Determine the difference between the first and last ratios */
	ud_delta = ud_ratio_last - ud_ratio_first;
	lr_delta = lr_ratio_last - lr_ratio_first;

	/* Accumulate the UD and LR delta values */
	gesture_data.ud_delta_ += ud_delta;
	gesture_data.lr_delta_ += lr_delta;


    /* Determine U/D gesture */
    if( gesture_data.ud_delta_ >= GESTURE_SENSITIVITY_1 ) {
        gesture_data.ud_count_ = 1;
    } else if( gesture_data.ud_delta_ <= -GESTURE_SENSITIVITY_1 ) {
        gesture_data.ud_count_ = -1;
    } else {
        gesture_data.ud_count_ = 0;
    }

    /* Determine L/R gesture */
    if( gesture_data.lr_delta_ >= GESTURE_SENSITIVITY_1 ) {
        gesture_data.lr_count_ = 1;
    } else if( gesture_data.lr_delta_ <= -GESTURE_SENSITIVITY_1 ) {
        gesture_data.lr_count_ = -1;
    } else {
        gesture_data.lr_count_ = 0;
    }


    /* Determine Near/Far gesture */
	if( (gesture_data.ud_count_ == 0) && (gesture_data.lr_count_ == 0) ) {
		if( (abs(ud_delta) < GESTURE_SENSITIVITY_2) && \
			(abs(lr_delta) < GESTURE_SENSITIVITY_2) ) {

			if( (ud_delta == 0) && (lr_delta == 0) ) {
				gesture_data.near_count_++;
			} else if( (ud_delta != 0) || (lr_delta != 0) ) {
				gesture_data.far_count_++;
			}

			if( (gesture_data.near_count_ >= 10) && (gesture_data.far_count_ >= 2) ) {
				if( (ud_delta == 0) && (lr_delta == 0) ) {
					gesture_data.state_ = NEAR_STATE;
				} else if( (ud_delta != 0) && (lr_delta != 0) ) {
					gesture_data.state_ = FAR_STATE;
				}
				return true;
			}
		}
	} else {
		if( (abs(ud_delta) < GESTURE_SENSITIVITY_2) && \
			(abs(lr_delta) < GESTURE_SENSITIVITY_2) ) {

			if( (ud_delta == 0) && (lr_delta == 0) ) {
				gesture_data.near_count_++;
			}

			if( gesture_data.near_count_ >= 10 ) {
				gesture_data.ud_count_ = 0;
				gesture_data.lr_count_ = 0;
				gesture_data.ud_delta_ = 0;
				gesture_data.lr_delta_ = 0;
			}
		}
	}
	return false;
}


bool APDS9960_decodeGesture(void)
{
	/* Return if near or far event is detected */
	if( gesture_data.state_ == NEAR_STATE ) {
		gesture_data.motion_ = DIR_NEAR;
		return true;
	} else if ( gesture_data.state_ == FAR_STATE ) {
		gesture_data.motion_ = DIR_FAR;
		return true;
	}

/* Determine swipe direction */
	if( (gesture_data.ud_count_ == -1) && (gesture_data.lr_count_ == 0) ) {
		gesture_data.motion_ = DIR_UP;
	} else if( (gesture_data.ud_count_ == 1) && (gesture_data.lr_count_ == 0) ) {
		gesture_data.motion_ = DIR_DOWN;
	} else if( (gesture_data.ud_count_ == 0) && (gesture_data.lr_count_ == 1) ) {
		gesture_data.motion_ = DIR_RIGHT;
	} else if( (gesture_data.ud_count_ == 0) && (gesture_data.lr_count_ == -1) ) {
		gesture_data.motion_ = DIR_LEFT;
	} else if( (gesture_data.ud_count_ == -1) && (gesture_data.lr_count_ == 1) ) {
		if( abs(gesture_data.ud_delta_) > abs(gesture_data.lr_delta_) ) {
			gesture_data.motion_ = DIR_UP;
		} else {
			gesture_data.motion_ = DIR_RIGHT;
		}
	} else if( (gesture_data.ud_count_ == 1) && (gesture_data.lr_count_ == -1) ) {
		if( abs(gesture_data.ud_delta_) > abs(gesture_data.lr_delta_) ) {
			gesture_data.motion_ = DIR_DOWN;
		} else {
			gesture_data.motion_ = DIR_LEFT;
		}
	} else if( (gesture_data.ud_count_ == -1) && (gesture_data.lr_count_ == -1) ) {
		if( abs(gesture_data.ud_delta_) > abs(gesture_data.lr_delta_) ) {
			gesture_data.motion_ = DIR_UP;
		} else {
			gesture_data.motion_ = DIR_LEFT;
		}
	} else if( (gesture_data.ud_count_ == 1) && (gesture_data.lr_count_ == 1) ) {
		if( abs(gesture_data.ud_delta_) > abs(gesture_data.lr_delta_) ) {
			gesture_data.motion_ = DIR_DOWN;
		} else {
			gesture_data.motion_ = DIR_RIGHT;
		}
	} else {
		return false;
	}

	return true;

}



bool APDS9960_isGestureAvailable(void)
{
	uint8_t value;

	value = I2C_ReadByte(APDS9960_GSTATUS);

	value &= APDS9960_GVALID;

	if(value == 1)
		return true;
	else
		return false;
}


int APDS9960_readGesture(void)
{
	uint8_t fifo_level = 0;
	uint8_t bytes_read = 0;
	uint8_t fifo_data[128];
	uint8_t gstatus;
	int motion;
	int i;

	if(!APDS9960_isGestureAvailable())
		return DIR_NONE;

	while(1)
	{
		for(i=0;i<I2C_DELAY;i++);

		gstatus = I2C_ReadByte(APDS9960_GSTATUS);

		/* Valid FIFO DATA */
		if((gstatus & APDS9960_GVALID) == APDS9960_GVALID)
		{
			fifo_level = I2C_ReadByte(APDS9960_GFLVL);

			if (fifo_level > 0)
			{
				bytes_read = I2C_PageRead(APDS9960_GFIFO_U,(uint8_t*)fifo_data,(fifo_level*4));


				if (bytes_read >=4)
				{
					for (i=0;i<bytes_read;i+=4)
					{
						gesture_data.u_data[gesture_data.index] = fifo_data[i+0];
						gesture_data.d_data[gesture_data.index] = fifo_data[i+1];
						gesture_data.l_data[gesture_data.index] = fifo_data[i+2];
						gesture_data.r_data[gesture_data.index] = fifo_data[i+3];
						gesture_data.index++;
						gesture_data.total_gestures++;
					}


					if(APDS9960_processGestureData())
					{
						if(APDS9960_decodeGesture())
						{

						}
					}

					gesture_data.total_gestures = 0;
					gesture_data.index = 0;
				}
			}
		}
		else
		{
			for(i=0;i<I2C_DELAY;i++);
			APDS9960_decodeGesture();
			gstatus = I2C_ReadByte(APDS9960_GSTATUS);
			fifo_level = I2C_ReadByte(APDS9960_GFLVL);

			motion = gesture_data.motion_;
			resetGestureParameters();
			return motion;
		}
	}
}




void APDS9960_Init(void)
{

	APDS9960_SetMode(ALL,OFF);

	I2C_WriteByte(APDS9960_ATIME,DEFAULT_ATIME);
	I2C_WriteByte(APDS9960_WTIME,DEFAULT_WTIME);
	I2C_WriteByte(APDS9960_PRO_PULSE_LEN,DEFAULT_PROX_PULSE);
	I2C_WriteByte(APDS9960_PRO_OFFSET_UR,DEFAULT_POFFSET_UR);
	I2C_WriteByte(APDS9960_PRO_OFFSET_DL,DEFAULT_POFFSET_DL);
	I2C_WriteByte(APDS9960_CONFIG1, DEFAULT_CONFIG1);
	APDSS9960_setLEDDrive(DEFAULT_LDRIVE);
	APDS9960_setProximityGain(DEFAULT_PGAIN);
	APDS9960_Proximity_Threshold(APDS9960_PRO_LOW_THRESHOLD,DEFAULT_PILT);
	APDS9960_Proximity_Threshold(APDS9960_PRO_HIGH_THRESHOLD,DEFAULT_PIHT);
	I2C_WriteByte(APDS9960_PRO_PERSISTENCE,DEFAULT_PERS);
	I2C_WriteByte(APDS9960_PRO_CONFIG2,DEFAULT_CONFIG2);
	I2C_WriteByte(APDS9960_PRO_CONFIG3,DEFAULT_CONFIG3);


	/* Writing to Gesture Sensor Registers */

	APDS9960_Gesture_Threshold(APDS9960_GPENTH,DEFAULT_GPENTH);
	APDS9960_Gesture_Threshold(APDS9960_GEXTH,DEFAULT_GEXTH);
	I2C_WriteByte(APDS9960_GCONF1, DEFAULT_GCONF1);

	APDS9960_setGestureGain(DEFAULT_GGAIN);
	APDS9960_setGestureLEDDrive(DEFAULT_GLDRIVE);
	APDS9960_setGestureWaitTime(DEFAULT_GWTIME);

	I2C_WriteByte(APDS9960_GOFFSET_U, DEFAULT_GOFFSET);
	I2C_WriteByte(APDS9960_GOFFSET_D, DEFAULT_GOFFSET);
	I2C_WriteByte(APDS9960_GOFFSET_L, DEFAULT_GOFFSET);
	I2C_WriteByte(APDS9960_GOFFSET_R, DEFAULT_GOFFSET);

	I2C_WriteByte(APDS9960_GPULSE, DEFAULT_GPULSE);
	I2C_WriteByte(APDS9960_GCONF3, DEFAULT_GCONF3);

	APDS9960_setGestureInterrupts(DEFAULT_GIEN);

}



void APDS9960_Proximity_Enable(bool interrupts)
{
	//APDS9960_SetMode(ALL,OFF);
	APDS9960_setProximityGain(DEFAULT_PGAIN);
	APDSS9960_setLEDDrive(DEFAULT_LDRIVE);
	//APDS9960_Proximity_Threshold(APDS9960_PRO_LOW_THRESHOLD,DEFAULT_PILT);
	//APDS9960_Proximity_Threshold(APDS9960_PRO_HIGH_THRESHOLD,DEFAULT_PIHT);

	if (interrupts)
		APDS9960_setProximityIntEnable(1);
	else
		APDS9960_setProximityIntEnable(0);

	APDS9960_enablePower();
	APDS9960_SetMode(PROXIMITY,1);																		// Enable the GPIO Interrupt
	GPIO_PinOutSet(GPIO_INT_PORT,GPIO_INT_PIN);
	NVIC_EnableIRQ(GPIO_ODD_IRQn);
	//APDS9960_Proximity_Read();

}


void APDS9960_Proximity_Start(bool interrupts)
{
	APDS9960_setProximityGain(DEFAULT_PGAIN);
	//APDS9960_Proximity_Threshold(APDS9960_PRO_LOW_THRESHOLD,DEFAULT_PILT);
	//APDS9960_Proximity_Threshold(APDS9960_PRO_HIGH_THRESHOLD,DEFAULT_PIHT);
	if (interrupts)
		APDS9960_Proximity_Enable(true);
	else
		APDS9960_Proximity_Enable(false);

}


void resetGestureParameters()
{
	gesture_data.index = 0;
	gesture_data.total_gestures = 0;

	gesture_data.ud_delta_ = 0;
	gesture_data.lr_delta_ = 0;

	gesture_data.ud_count_   = 0;
	gesture_data.lr_count_   = 0;

	gesture_data.near_count_ = 0;
	gesture_data.far_count_  = 0;

	gesture_data.state_  = 0;
	gesture_data.motion_ = 0;

}

void ADPS9960_Gesture_Enable(bool interrupts)
{
	resetGestureParameters();
	I2C_WriteByte(APDS9960_WTIME, 0xFF);

	I2C_WriteByte(APDS9960_PRO_PULSE_LEN,DEFAULT_GESTURE_PPULSE);

	APDS9960_setLEDBoost(LED_BOOST_300);

	if(interrupts)
		APDS9960_setGestureInterrupts(1);
	else
		APDS9960_setGestureInterrupts(0);

	APDS9960_SetGestureMode(1);
	APDS9960_enablePower();
	APDS9960_SetMode(PROXIMITY,1);
	APDS9960_SetMode(GESTURE,1);
	GPIO_PinOutSet(GPIO_INT_PORT,GPIO_INT_PIN);
	NVIC_EnableIRQ(GPIO_ODD_IRQn);

}



void APDS9960_Gesture_Start(bool interrupts)
{
	if(interrupts)
		ADPS9960_Gesture_Enable(true);
	else
		ADPS9960_Gesture_Enable(false);
}




/*
 * Description: This function write in to TSL2561 command register
 * Args: command register address
 * Return: None
 */
void I2C_CommandWrite(uint8_t command_register)
{
	I2C_Start();
	I2C1->TXDATA	= command_register;
	while (!(I2C1->IF & I2C_IF_ACK));
	I2C1->IFC = I2C_IFC_ACK;

	I2C1->CMD = I2C_CMD_STOP;
}

/*
 * Description: Initializes the GPIO Pins used for I2C communications
 * Args: None
 * Return: None
 */

void I2C_PIN_Initialize(void)
{
	  GPIO_PinModeSet(I2C1_SCL_PORT, I2C1_SCL_PIN, I2C_GPIO_MODE, 1);			// SCL
	  GPIO_PinModeSet(I2C1_SDA_PORT, I2C1_SDA_PIN, I2C_GPIO_MODE, 1);			// SDA
}


/*
 * Description: Initializes the I2C1 Bus on location 0. It also resets the I2C BUS
 * Args: None
 * Return: Nones
 */

void I2C_Initialize(void)
{
	CMU_ClockEnable(cmuClock_I2C1, true);						// Enabling Clock for I2C1
	I2C_PIN_Initialize();										// Enable the I2C Pins

	/* I2C1 Init Structure*/
	I2C_Init_TypeDef i2cInit =
	{
			.enable  = true,
			.master  = true,									// use as master
			.refFreq = 0,
			.freq 	 = I2C_FREQ_STANDARD_MAX,					// 9200
			.clhr	 = I2C_CLOCK								// Use equal duty cycle for SCL and SDA (4:4s)
	};


	  /* Enable pins at location 1 (which is used on the TSL2651) */
	  I2C1->ROUTE = I2C_ROUTE_SDAPEN |
	                I2C_ROUTE_SCLPEN |
	                (I2C1_LOCATION << _I2C_ROUTE_LOCATION_SHIFT);

	  I2C_Init(I2C1,&i2cInit);									// Initialize teh I2C1
	  RESET_I2C_BUS();											// Reset the Bus

}




