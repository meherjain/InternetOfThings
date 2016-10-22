/**
 * \file
 *
 * \brief BLE Startup Template
 *
 * Copyright (c) 2016 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel
 * Support</a>
 */

/**
 * \mainpage
 * \section preface Preface
 * This is the reference manual for the Startup Template
 */
/*- Includes ---------------------------------------------------------------*/
#include <asf.h>
#include <string.h>
#include "platform.h"
#include "at_ble_api.h"
#include "console_serial.h"
#include "timer_hw.h"
#include "ble_manager.h"
#include "ble_utils.h"
#include "button.h"
#include "startup_template_app.h"

volatile at_ble_status_t status;
at_ble_handle_t htpt_conn_handle;
volatile bool Timer_Flag = false;
volatile bool Temp_Notification_Flag = false;

/* UART Declaration */
struct uart_module uart_instance1;
#define BUFFER_LEN    8											// DMA Callback Size for UART
static uint8_t string[BUFFER_LEN+1];

/*Temperature Buffer */
static uint8_t temp_buffer[BUFFER_LEN+1];						// Temperature Buffer for BLE

/*DMA Resources */
struct dma_resource uart_dma_resource_tx;
struct dma_resource uart_dma_resource_rx;

/* DMA Descriptors */
struct dma_descriptor example_descriptor_tx;
struct dma_descriptor example_descriptor_rx;


static void transfer_done_tx(struct dma_resource* const resource )
{
	dma_start_transfer_job(&uart_dma_resource_tx);
}

/*
Added by Meher Jain
Parse the leuart message received from leopard gecko

Message Format:

Size: 8 bytes
Header: 1 Byte 'T' for temperature sensor and 'L' for light sensor
Light Sensor: 2nd Byte 'O' for LED ON on leopard gecko and 'F' for LED ON on leopard gecko

Temperature Sensor: next 7 bytes for the floating point temperature from leopard gecko in string format		 
*/

static void transfer_done_rx(struct dma_resource* const resource )
{
	int i=0;
	printf("\n");
	for(i=0;i<8;i++)
		printf("%c",string[i]);
	printf("\n");
	string[BUFFER_LEN] = '\0';												// Adding NULL at the end of the string for string function to work properly
	if(string[0] == LIGHT_SENSOR)											// Parsing the leuart message header 
	{
		if(string[1] == LEDON)
			gpio_pin_set_output_level(LED_0_PIN,LED_ON);					// Turn the LED ON when ON message is received from leopard gecko 
					
		else if(string[1] == LEDOFF)
			gpio_pin_set_output_level(LED_0_PIN,LED_OFF);					// Turn the LED OFF when OFF message is received from leopard gecko			
	}	
	else if(string[0] == TEMP_SENSOR)										// Temp Sensor Header
	{
		strcpy(temp_buffer,(string+1));										// Copying the string buffer to another buffer for BLE to send
		//printf("Temp:");
		//for(i=0;i<8;i++)
			//printf("%c",temp_buffer[i]);
	}
	//printf("%ld",strlen(temp_buffer));
			
	dma_start_transfer_job(&uart_dma_resource_rx);							// Reinitiating the DMA Transfer //
	
}
//! [transfer_done_rx]

//! [config_dma_resource_tx]
static void configure_dma_resource_tx(struct dma_resource *resource)
{
	//! [setup_tx_1]
	struct dma_resource_config config;
	//! [setup_tx_1]

	//! [setup_tx_2]
	dma_get_config_defaults(&config);
	//! [setup_tx_2]

	//! [setup_tx_3]
	config.des.periph = UART1TX_DMA_PERIPHERAL;
	config.des.enable_inc_addr = false;
	config.src.periph = UART1TX_DMA_PERIPHERAL;
	//! [setup_tx_3]

	//! [setup_tx_4]
	dma_allocate(resource, &config);
	//! [setup_tx_4]
}
//! [config_dma_resource_tx]


//! [setup_dma_transfer_tx_descriptor]
static void setup_transfer_descriptor_tx(struct dma_descriptor *descriptor)
{

	//! [setup_tx_5]
	dma_descriptor_get_config_defaults(descriptor);
	//! [setup_tx_5]

	//! [setup_tx_6]
	descriptor->buffer_size = BUFFER_LEN;
	descriptor->read_start_addr = (uint32_t)string;
	descriptor->write_start_addr =
	(uint32_t)(&uart_instance1.hw->TRANSMIT_DATA.reg);
	//! [setup_tx_6]
}
//! [setup_dma_transfer_tx_descriptor]


//! [config_dma_resource_rx]
static void configure_dma_resource_rx(struct dma_resource *resource)
{
	//! [setup_rx_1]
	struct dma_resource_config config;
	//! [setup_rx_1]

	//! [setup_rx_2]
	dma_get_config_defaults(&config);
	//! [setup_rx_2]

	//! [setup_rx_3]
	config.src.periph = UART1RX_DMA_PERIPHERAL;
	config.src.enable_inc_addr = false;
	config.src.periph_delay = 1;
	//! [setup_rx_3]

	//! [setup_rx_4]
	dma_allocate(resource, &config);
	//! [setup_rx_4]
}
//! [config_dma_resource_rx]


//! [setup_dma_transfer_rx_descriptor]
static void setup_transfer_descriptor_rx(struct dma_descriptor *descriptor)
{
	//! [setup_rx_5]
	dma_descriptor_get_config_defaults(descriptor);
	//! [setup_rx_5]

	//! [setup_tx_6]
	descriptor->buffer_size = BUFFER_LEN;
	descriptor->read_start_addr =
	(uint32_t)(&uart_instance1.hw->RECEIVE_DATA.reg);
	descriptor->write_start_addr = (uint32_t)string;
	//! [setup_tx_6]
}
//! [setup_dma_transfer_rx_descriptor]


//! [setup_usart]
static void configure_usart(void)
{
	//! [setup_config]
	struct uart_config config_uart1;
	//! [setup_config]

	//! [setup_config_defaults]
	uart_get_config_defaults(&config_uart1);
	//! [setup_config_defaults]

	//! [setup_change_config]
	config_uart1.baud_rate = 9600;
	config_uart1.pin_number_pad[0] = EDBG1_CDC_SERCOM_PIN_PAD0;
	config_uart1.pin_number_pad[1] = EDBG1_CDC_SERCOM_PIN_PAD1;
	config_uart1.pin_number_pad[2] = EDBG1_CDC_SERCOM_PIN_PAD2;
	config_uart1.pin_number_pad[3] = EDBG1_CDC_SERCOM_PIN_PAD3;
	config_uart1.pinmux_sel_pad[0] = EDBG1_CDC_SERCOM_MUX_PAD0;
	config_uart1.pinmux_sel_pad[1] = EDBG1_CDC_SERCOM_MUX_PAD1;
	config_uart1.pinmux_sel_pad[2] = EDBG1_CDC_SERCOM_MUX_PAD2;
	config_uart1.pinmux_sel_pad[3] = EDBG1_CDC_SERCOM_MUX_PAD3;
	//! [setup_change_config]

	//! [setup_set_config]
	while (uart_init(&uart_instance1,
	EDBG1_CDC_MODULE, &config_uart1) != STATUS_OK) {
	}
	//! [setup_set_config]

	//! [enable_interrupt]
	uart_enable_transmit_dma(&uart_instance1);
	uart_enable_receive_dma(&uart_instance1);
	//! [enable_interrupt]
}
//! [setup_usart]

//! [setup_callback]
static void configure_dma_callback(void)
{
	//! [setup_callback_register]
	dma_register_callback(&uart_dma_resource_tx, transfer_done_tx, DMA_CALLBACK_TRANSFER_DONE);
	dma_register_callback(&uart_dma_resource_rx, transfer_done_rx, DMA_CALLBACK_TRANSFER_DONE);
	//! [setup_callback_register]

	//! [setup_enable_callback]
	dma_enable_callback(&uart_dma_resource_tx, DMA_CALLBACK_TRANSFER_DONE);
	dma_enable_callback(&uart_dma_resource_rx, DMA_CALLBACK_TRANSFER_DONE);
	//! [setup_enable_callback]

	//! [enable_inic]
	NVIC_EnableIRQ(PROV_DMA_CTRL0_IRQn);
	//! [enable_inic]
}
//! [setup_callback]



/* Timer callback */
static void timer_callback_handler(void)
{
	/* Stop timer */
	hw_timer_stop();
	/* Set timer Alarm flag */
	Timer_Flag = true;
	/* Restart Timer */
	hw_timer_start(10);
}

/* Sending the temperature value after reading it from IO1 Xplained Pro */
static void htp_temperature_send(void) {
	at_ble_prf_date_time_t timestamp;
	float temperature;
	/* Read Temperature Value from IO1 Xplained Pro */
	//temperature = at30tse_read_temperature();
	temperature = atof(temp_buffer);
#ifdef HTPT_FLAG_FAHRENHEIT
	temperature = (((temperature * 9.0)/5.0) + 32.0);
#endif
	/* Read Temperature Value from IO1 Xplained Pro */
	timestamp.day = 1;
	timestamp.hour = 9;
	timestamp.min = 2;
	timestamp.month = 8;
	timestamp.sec = 36;
	timestamp.year = 15;
	/* Read Temperature Value from IO1 Xplained Pro */
	if(at_ble_htpt_temp_send(convert_ieee754_ieee11073_float((float)temperature),
	&timestamp,
	(at_ble_htpt_temp_flags)(HTPT_FLAG_CELSIUS | HTPT_FLAG_TYPE),
	HTP_TYPE_ARMPIT,
	1
	) == AT_BLE_SUCCESS)
	{
#ifdef HTPT_FLAG_FAHRENHEIT
		//printf("Temperature: %d Fahrenheit\n", (uint16_t)temperature);
#else
		//printf("Temperature: %d Deg Celsius\n", (uint16_t)temperature);
#endif
	}
}

static at_ble_status_t  app_htpt_cfg_indntf_ind_handler(void *params)
{
	at_ble_htpt_cfg_indntf_ind_t htpt_cfg_indntf_ind_params;
	memcpy((uint8_t *)&htpt_cfg_indntf_ind_params, params, sizeof(at_ble_htpt_cfg_indntf_ind_t));
	if (htpt_cfg_indntf_ind_params.ntf_ind_cfg == 1) {
		printf("Started HTP Temperature Notification");
		Temp_Notification_Flag = true;
	}
	else {
		printf("HTP Temperature Notification Stopped");
		Temp_Notification_Flag = false;
	}
	return AT_BLE_SUCCESS;
}

static const ble_event_callback_t app_htpt_handle[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	app_htpt_cfg_indntf_ind_handler,
	NULL,
	NULL,
	NULL
};


static void htp_init (void){
	printf("\nAssignment 4.1: Init Health temperature service ");
	/* Create htp service in GATT database*/
	status = at_ble_htpt_create_db(
	HTPT_TEMP_TYPE_CHAR_SUP,
	HTP_TYPE_ARMPIT,
	1,
	30,
	1,
	HTPT_AUTH,
	&htpt_conn_handle);
	if (status != AT_BLE_SUCCESS){
		printf("HTP Data Base creation failed");
		while(1);
	}
}


static void ble_advertise (void)
{
	printf("\nAssignment 2.1 : Start Advertising");
	status = ble_advertisement_data_set();
	if(status != AT_BLE_SUCCESS)
	{
		printf("\n\r## Advertisement data set failed : error %x",status);
		while(1);
	}
	/* Start of advertisement */
	status = at_ble_adv_start(AT_BLE_ADV_TYPE_UNDIRECTED,\
	AT_BLE_ADV_GEN_DISCOVERABLE,\
	NULL,\
	AT_BLE_ADV_FP_ANY,\
	1000,\
	655,\
	0);
	if(status != AT_BLE_SUCCESS)
	{
		printf("\n\r## Advertisement data set failed : error %x",status);
		while(1);
	}
}

/* Callback registered for AT_BLE_CONNECTED event*/
static at_ble_status_t ble_paired_cb (void *param)
{
	at_ble_pair_done_t *pair_params = param;
	printf("\nAssignment 3.2: Application paired ");
	/* Enable the HTP Profile */
	printf("\nAssignment 4.1: enable health temperature service ");
	status = at_ble_htpt_enable(pair_params->handle,
	HTPT_CFG_STABLE_MEAS_IND);
	if(status != AT_BLE_SUCCESS){
		printf("*** Failure in HTP Profile Enable");
		while(1);
	}
	ALL_UNUSED(param);
	return AT_BLE_SUCCESS;
}

/* Callback registered for AT_BLE_DISCONNECTED event */
static at_ble_status_t ble_disconnected_cb (void *param)
{
	printf("\nAssignment 3.2: Application disconnected ");
	ble_advertise();
	ALL_UNUSED(param);return AT_BLE_SUCCESS;
}

static const ble_event_callback_t app_gap_cb[] = {
	NULL, // AT_BLE_UNDEFINED_EVENT
	NULL, // AT_BLE_SCAN_INFO
	NULL, // AT_BLE_SCAN_REPORT
	NULL, // AT_BLE_ADV_REPORT
	NULL, // AT_BLE_RAND_ADDR_CHANGED
	NULL, // AT_BLE_CONNECTED
	ble_disconnected_cb, // AT_BLE_DISCONNECTED
	NULL, // AT_BLE_CONN_PARAM_UPDATE_DONE
	NULL, // AT_BLE_CONN_PARAM_UPDATE_REQUEST
	ble_paired_cb, // AT_BLE_PAIR_DONE
	NULL, // AT_BLE_PAIR_REQUEST
	NULL, // AT_BLE_SLAVE_SEC_REQUEST
	NULL, // AT_BLE_PAIR_KEY_REQUEST
	NULL, // AT_BLE_ENCRYPTION_REQUEST
	NULL, // AT_BLE_ENCRYPTION_STATUS_CHANGED
	NULL, // AT_BLE_RESOLV_RAND_ADDR_STATUS
	NULL, // AT_BLE_SIGN_COUNTERS_IND
	NULL, // AT_BLE_PEER_ATT_INFO_IND
	NULL  // AT_BLE_CON_CHANNEL_MAP_IND
};

static void register_ble_callbacks (void)
{
	/* Register GAP Callbacks */
	printf("\nAssignment 3.2: Register bluetooth events callbacks");
	status = ble_mgr_events_callback_handler(REGISTER_CALL_BACK,\
	BLE_GAP_EVENT_TYPE,app_gap_cb);
	if (status != true) {
		printf("\n##Error when Registering  SAMB11 gap callbacks");
	}
	status = ble_mgr_events_callback_handler(REGISTER_CALL_BACK,\
	BLE_GATT_HTPT_EVENT_TYPE,app_htpt_handle);
	if (status != true) {
		printf("\n##Error when Registering  SAMB11 htpt callbacks");
	}
}


static void htp_temperature_read(void)
{
	float temperature;
	/* Read Temperature Value from IO1 Xplained Pro */
	temperature = at30tse_read_temperature();
	/* Display temperature on com port */
#ifdef HTPT_FLAG_FAHRENHEIT
	printf("Temperature: %d Fahrenheit", (uint16_t)temperature);
#else
	printf("Temperature: %d Deg Celsius", (uint16_t)temperature);
#endif
}

void led_config(void)
{
	struct gpio_config pin_conf;
	gpio_get_config_defaults(&pin_conf);
		
	pin_conf.direction = GPIO_PIN_DIR_OUTPUT;
	gpio_pin_set_config(LED0_PIN,&pin_conf);
	gpio_pin_set_output_level(LED_0_PIN,LED_OFF);
	
}


int main (void)
{
	platform_driver_init();
	acquire_sleep_lock();
	/* Initialize serial console */
	serial_console_init();
	//configure_usart();
	led_config();
	/* Initialize the hardware timer */
	hw_timer_init();
	/* Register the callback */
	hw_timer_register_callback(timer_callback_handler);
	/* Start timer */
	hw_timer_start(1);
	/* initialize the BLE chip and Set the Device Address */
	ble_device_init(NULL);
	/* Initialize the temperature sensor */
	at30tse_init();
	/* configure the temperature sensor ADC */
	at30tse_write_config_register(AT30TSE_CONFIG_RES(AT30TSE_CONFIG_RES_12_bit));
	/* read the temperature from the sensor */
	htp_temperature_read();
	/* Initialize the htp service */
	htp_init();
	/* Register Bluetooth events Callbacks */
	register_ble_callbacks();
	/* Start Advertising process */
	ble_advertise();
	
	//! [setup_usart]
	configure_usart();
	//! [setup_usart]

	
	//! [setup_dma_resource]
	configure_dma_resource_tx(&uart_dma_resource_tx);
	configure_dma_resource_rx(&uart_dma_resource_rx);
	//! [setup_dma_resource]

	//! [setup_transfer_descriptor]
	setup_transfer_descriptor_tx(&example_descriptor_tx);
	setup_transfer_descriptor_rx(&example_descriptor_rx);
	//! [setup_transfer_descriptor]
	
	//! [add_descriptor_to_resource]
	dma_add_descriptor(&uart_dma_resource_tx, &example_descriptor_tx);
	dma_add_descriptor(&uart_dma_resource_rx, &example_descriptor_rx);
	//! [add_descriptor_to_resource]

	//! [configure_callback]
	configure_dma_callback();
	//! [configure_callback]
	//! [setup_init]

	//! [main]
	//! [main_1]
	dma_start_transfer_job(&uart_dma_resource_rx);
	//! [main_1]
	
	
	while(1) {
		ble_event_task(655);  // 655: 10 min
		if (Timer_Flag & Temp_Notification_Flag)
		{
			htp_temperature_send();
		}

	}
}

