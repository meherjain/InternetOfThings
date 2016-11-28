/*
 * i2c.h
 *
 *  Created on: Sep 28, 2016
 *      Author: Meher
 */

#ifndef I2C_H_
#define I2C_H_



#include "main.h"



// I2C Initialization Declaration //
#define I2C1_LOCATION 		0
#define I2C1_SCL_PORT		gpioPortC
#define I2C1_SDA_PORT		gpioPortC
#define I2C1_SCL_PIN		5
#define I2C1_SDA_PIN		4
#define I2C_GPIO_MODE		gpioModeWiredAnd
#define I2C_CLOCK 			i2cClockHLRStandard





#define APDS9960_SLAVE_ADDRESS				0x39
#define WRITE_CONTROL						0x00
#define APDS9960_WRITE_COMMAND_BYTE			((APDS9960_SLAVE_ADDRESS << 1) | WRITE_CONTROL)

#define READ_CONTROL						0x01
#define APDS9960_READ_CONTROL_BYTE			((APDS9960_SLAVE_ADDRESS << 1) | READ_CONTROL)


/* APDS9960 Proximity Sensor Declaration */

#define APDS9960_ENABLE						0x80
#define APDS9960_ATIME         			    0x81
#define APDS9960_WTIME          			0x83
#define APDS9960_PRO_LOW_THRESHOLD			0x89
#define APDS9960_PRO_HIGH_THRESHOLD			0x8B
#define APDS9960_CONFIG1        			0x8D
#define APDS9960_PRO_PERSISTENCE			0x8C
#define APDS9960_PRO_PULSE_LEN				0x8E
#define APDS9960_PRO_GAIN_CONT				0x8F
#define APDS9960_PRO_CONFIG2				0x90
#define APDS9960_PRO_STATUS					0x93
#define APDS9960_PRO_PDATA					0x9C
#define APDS9960_PRO_OFFSET_UR				0x9D
#define APDS9960_PRO_OFFSET_DL				0x9E
#define APDS9960_PRO_CONFIG3				0x9F
#define APDS9960_PRO_INT_CLEAR				0xE5
#define APDS9960_ALL_NON_GES_INT_CLEAR		0xE7


/* APDS89960 Gesture Declaration */
#define APDS9960_GPENTH         			0xA0
#define APDS9960_GEXTH          			0xA1
#define APDS9960_GCONF1         			0xA2
#define APDS9960_GCONF2         			0xA3
#define APDS9960_GOFFSET_U      			0xA4
#define APDS9960_GOFFSET_D      			0xA5
#define APDS9960_GOFFSET_L      			0xA7
#define APDS9960_GOFFSET_R      			0xA9
#define APDS9960_GPULSE         			0xA6
#define APDS9960_GCONF3         			0xAA
#define APDS9960_GCONF4         			0xAB
#define APDS9960_GFLVL          			0xAE
#define APDS9960_GSTATUS        			0xAF
#define APDS9960_IFORCE         			0xE4
#define APDS9960_PICLEAR        			0xE5
#define APDS9960_CICLEAR        			0xE6
#define APDS9960_AICLEAR        			0xE7
#define APDS9960_GFIFO_U        			0xFC
#define APDS9960_GFIFO_D        			0xFD
#define APDS9960_GFIFO_L        			0xFE
#define APDS9960_GFIFO_R        			0xFF




/* Data Fields */
#define APDS9960_PON						0b00000001
#define APDS9960_PEN						0b00000100
#define APDS9960_WEN						0b00001000
#define APSD9960_AIEN           			0b00010000
#define APDS9960_PIEN           			0b00100000
#define APDS9960_GEN            			0b01000000
#define APDS9960_GVALID         			0b00000001



/* ON/OFF Definitions */
#define ON									1
#define OFF									0

/* Modes */
#define POWER								0
#define AMBIENT_LIGHT						1
#define PROXIMITY							2
#define WAIT								3
#define AMBIENT_LIGHT_INT					4
#define PROXIMITY_INT						5
#define GESTURE								6
#define ALL									7

/* LED Drive values */
#define LED_DRIVE_100MA         0
#define LED_DRIVE_50MA          1
#define LED_DRIVE_25MA          2
#define LED_DRIVE_12_5MA        3


/* Proximity Gain (PGAIN) values */
#define PGAIN_1X                0
#define PGAIN_2X                1
#define PGAIN_4X                2
#define PGAIN_8X                3


/* Gesture Gain (GGAIN) values */
#define GGAIN_1X                0
#define GGAIN_2X                1
#define GGAIN_4X                2
#define GGAIN_8X                3


/* LED Boost values */
#define LED_BOOST_100           0
#define LED_BOOST_150           1
#define LED_BOOST_200           2
#define LED_BOOST_300           3


/* Gesture wait time values */
#define GWTIME_0MS              0
#define GWTIME_2_8MS            1
#define GWTIME_5_6MS            2
#define GWTIME_8_4MS            3
#define GWTIME_14_0MS           4
#define GWTIME_22_4MS           5
#define GWTIME_30_8MS           6
#define GWTIME_39_2MS           7

/* Gesture parameters */
#define GESTURE_THRESHOLD_OUT   10
#define GESTURE_SENSITIVITY_1   50
#define GESTURE_SENSITIVITY_2   20



/* Default Values */
#define DEFAULT_ATIME           219     // 103ms
#define DEFAULT_WTIME			246				// 27ms
#define DEFAULT_PROX_PULSE		0x87			// 16us, 8 Pulses
#define DEFAULT_POFFSET_UR		0				// 0 OFFSET
#define DEFAULT_POFFSET_DL		0				// 0 OFFSET
#define DEFAULT_CONFIG1			0x60
#define DEFAULT_LDRIVE			LED_DRIVE_100MA
#define DEFAULT_PGAIN			PGAIN_2X
#define DEFAULT_PILT			0
#define DEFAULT_PIHT			50
#define DEFAULT_AILT			0xFFFF
#define DEFAULT_PERS            0xA1    // 2 consecutive prox or ALS for int.
#define DEFAULT_CONFIG2         0x01    // No saturation interrupts or LED boost
#define DEFAULT_CONFIG3			0		// Enable All
#define DEFAULT_GPENTH			40		// Threshold for Entering Gesture Mode
#define DEFAULT_GEXTH			30		// Threshold for Exiting Gesture Mode
#define DEFAULT_GCONF1			0x40    // 4 Gesture events
#define DEFAULT_GGAIN			GGAIN_4X
#define DEFAULT_GLDRIVE			LED_DRIVE_100MA
#define DEFAULT_GWTIME			GWTIME_2_8MS
#define DEFAULT_GOFFSET			0 		// No Offset Scaling
#define DEFAULT_GPULSE			0xC9	// 32us 10 pulses
#define DEFAULT_GCONF3			0		// All photodiodes active during gesture
#define DEFAULT_GIEN			0		// Disable Gesture Interrupts
#define DEFAULT_GESTURE_PPULSE  0x89	// 16us 10 Pulses

/* I2C Specific Declaration */
#define	I2C_BUFFER_SIZE					1
#define I2C_DELAY						10000
#define I2C_SHORT_DELAY					1000

#define ERROR							0xFF

#define RESET_I2C_BUS()					do { \
											if (I2C1->STATE & I2C_STATE_BUSY){ \
					  	  	  	  	  	  	  I2C1->CMD = I2C_CMD_ABORT;} \
											}while(0)


/* I2C Specific Declaration */
void I2C_Initialize(void);
void I2C_PIN_Initialize(void);
void I2C_WriteByte(uint8_t,uint8_t);
uint8_t I2C_ReadByte(uint8_t);
void I2C_CommandWrite(uint8_t);
void I2C_Start(void);

/* Proximity Declaration */
void APDS9960_SetMode(uint8_t, uint8_t);
void APDS9960_setProximityGain(uint8_t);
void APDSS9960_setLEDDrive(uint8_t);
void APDS9960_setProximityIntEnable(uint8_t);
void APDS9960_enablePower(void);
uint8_t APDS9960_Proximity_Read(void);
void APDS9960_Proximity_Enable(bool);
void APDS9960_Proximity_Threshold(uint8_t,uint8_t);
void APDS9960_Init(void);
void APDS9960_Proximity_Start(bool);



/* Gesture Declaration */
void APDS9960_Gesture_Threshold(uint8_t, uint8_t);
void APDS9960_setGestureGain(uint8_t);
void APDS9960_setGestureLEDDrive(uint8_t);
void APDS9960_setGestureWaitTime(uint8_t);
void APDS9960_setGestureInterrupts(uint8_t);
void APDS9960_setLEDBoost(uint8_t);
void APDS9960_SetGestureMode(uint8_t);
void resetGestureParameters(void);
void ADPS9960_Gesture_Enable(bool);
void APDS9960_Gesture_Start(bool);
bool APDS9960_isGestureAvailable(void);
int APDS9960_readGesture(void);
bool APDS9960_processGestureData(void);
bool APDS9960_decodeGesture(void);


/* State definitions */
enum {
  NA_STATE,
  NEAR_STATE,
  FAR_STATE,
  ALL_STATE
};


/* Direction definitions */
enum {
	DIR_NONE,
	DIR_LEFT,
	DIR_RIGHT,
	DIR_UP,
	DIR_DOWN,
	DIR_NEAR,
	DIR_FAR,
	DIR_ALL
};


typedef struct {
	uint8_t u_data[32];
	uint8_t d_data[32];
	uint8_t l_data[32];
	uint8_t r_data[32];
	uint8_t index;
	uint8_t total_gestures;
	uint8_t in_threshold;
	uint8_t out_threshold;
    int ud_delta_;
	int lr_delta_;
	int ud_count_;
	int lr_count_;
	int near_count_;
	int far_count_;
	int state_;
	int motion_;
}gesture_data_type;

gesture_data_type gesture_data;







#endif /* I2C_H_ */
