

/*
 * Name: Meher Jain
 * Email ID: meher.jain@colorado.edu
 * Date: 09/18/2016
 * Description: All the fucntion related to I2C communication with TSL2561 Luminosity sensor */


# include "i2c.h"

/*
 * I2C_Start Function
 * Description: Sends I2C Start Sequence with TSL2561 slave address. The routine uses ACK Pollin.
 * Args: None
 * Return: None
 */

void I2C_Start(void)
{

	I2C1->TXDATA = WRITE_COMMAND_BYTE;						// Send the slave address
	I2C1->CMD	 = I2C_CMD_START;							// Send Start Bit
	
	while (!(I2C1->IF & I2C_IF_ACK));						// Polling the ACK
	I2C1->IFC	= I2C_IFC_ACK;								// Clearing the ACK
	I2C1->IFC	= I2C_IFC_NACK;

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
	I2C1->IFC	= I2C_IFC_NACK;

	I2C1->TXDATA	= write_data;							// Write the data
	while (!(I2C1->IF & I2C_IF_ACK));						// Polling for the ACK
	I2C1->IFC = I2C_IFC_ACK;								// Clearing the ACK
	I2C1->IFC	= I2C_IFC_NACK;

	I2C1->CMD		= I2C_CMD_STOP;							// Sending stop bit to stop the transfer
	while (!(I2C1->IF & I2C_IF_MSTOP));						// Waiting for the stop bit to be sent
	I2C1->IFC	= I2C_IFC_NACK;
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
	I2C1->IFC	= I2C_IFC_NACK;

	I2C1->CMD		= I2C_CMD_START;				// Repeat Start
	I2C1->TXDATA	= READ_CONTROL_BYTE;			// Send Read Control Byte
	while(!(I2C1->IF & I2C_IF_ACK));
	I2C1->IFC 		= I2C_IFC_ACK;
	I2C1->IFC	= I2C_IFC_NACK;

	while(!(I2C1->IF & I2C_IF_RXDATAV));			// Polls for rx data to become valid
	I2C1->IFC	= I2C_IFC_NACK;
	read_data = I2C1->RXDATA;						// Reads the valid data

	I2C1->CMD		= I2C_CMD_NACK;					// Send NACK Bit
	I2C1->CMD		= I2C_CMD_STOP;					// Send Stop Bit

	return read_data;
}

/*
 * I2C Word Read Function
 * Description: Follows Write-Read-Read Protocol. This function writes to the command register and sends the repeat start. After successful
 * ACK, it writes the read control byte to the slave. The routine polls for the valid rx data available in the buffer and since TSL2561
 * keep on sending the data untill stop bit is received from master, the routine reads another byte and sends NACK, stop bit to the slave to stop the transmission.
 * Args: Command_register address
 * Return: I2C_Data (2 Byte)
 */

uint16_t I2C_WordRead(uint8_t command_register)
{
	uint16_t read_data=0;
	I2C_Start();
	I2C1->TXDATA	=	command_register;			// Sending command
	while(!(I2C1->IF & I2C_IF_ACK));				// Pooling for ACK
	I2C1->IFC 		= I2C_IFC_ACK;					// Clearing ACK
	I2C1->IFC	= I2C_IFC_NACK;						// Clearing NACK

	I2C1->CMD		= I2C_CMD_START;				// Repeat Start
	I2C1->TXDATA	= READ_CONTROL_BYTE;			// Send the Read Control Byte
	while(!(I2C1->IF & I2C_IF_ACK));				// Polling for ACK
	I2C1->IFC 		= I2C_IFC_ACK;					// Clearing ACK
	I2C1->IFC	= I2C_IFC_NACK;						// Clearing NACK

	while(!(I2C1->IF & I2C_IF_RXDATAV));			// Waiting for Received Data
	I2C1->IFC	= I2C_IFC_NACK;						//Clearing NACK
	read_data = I2C1->RXDATA;

	I2C1->CMD	= I2C_CMD_ACK;						// Sending ACK for the next byte

	while(!(I2C1->IF & I2C_IF_RXDATAV));			// Waiting for Received Data
	I2C1->IFC	= I2C_IFC_NACK;
	read_data += (I2C1->RXDATA << 8);				// Converting to 16 bits

	return read_data;

}

/* Initialize TL2561 Lux Sensor
 * Description: This function initialize the Lux sensor with appropriate boot sequence on I2C1 Bus.
 * Args: None
 * Return: None
 */
void TL2561_init(void)
{
	I2C_WriteByte(CONTROL_COMMAND,CONTROL_DATA);							// Send Power-ON Sequence
	I2C_WriteByte(TIMING_COMMAND,TIMING_DATA);								// 101ms Integration Time
	I2C_WriteByte(THRESHOLD_LOW_COMMAND_LOW,THRESHOLD_LOW_DATA_LOW);		// Low Threshold (2 Byte)
	I2C_WriteByte(THRESHOLD_LOW_COMMAND_HIGH,THRESHOLD_LOW_DATA_HIGH);
	I2C_WriteByte(THRESHOLD_HIGH_COMMAND_LOW,THRESHOLD_HIGH_DATA_LOW);		// High Threshold (2 Byte)
	I2C_WriteByte(THRESHOLD_HIGH_COMMAND_HIGH,THRESHOLD_HIGH_DATA_HIGH);
	I2C_WriteByte(INTERRUPT_COMMAND, INTERRUPT_DATA);						// Interrupt persistence (4ms)

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




