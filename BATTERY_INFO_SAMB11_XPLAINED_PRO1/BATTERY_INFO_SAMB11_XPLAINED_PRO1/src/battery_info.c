/**
* \file
*
* \brief Battery Information Service - Application
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
* Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
*/

/****************************************************************************************
*							        Includes	                                     	*
****************************************************************************************/
#include <asf.h>
#include "console_serial.h"
#include "at_ble_api.h"
#include "platform.h"
#include "battery.h"
#include "timer_hw.h"
#include "ble_utils.h"
#include "battery_info.h"
#include "ble_manager.h"
#include "aon_sleep_timer_basic.h"

#define _AON_TIMER_

/**************Temperature **********************/
volatile at_ble_status_t status;
at_ble_handle_t htpt_conn_handle;
volatile bool Timer_Flag = false;
volatile bool Temp_Notification_Flag = false;



/****************************************************************/


/* UART Declaration */
struct uart_module uart_instance1;
#define BUFFER_LEN    6								// DMA Call Back after 6 bytes
static uint8_t string[BUFFER_LEN+1];

/*Temperature Buffer */
static uint8_t temperature_buffer[BUFFER_LEN+1];

/*DMA Resources */
struct dma_resource uart_dma_resource_tx;
struct dma_resource uart_dma_resource_rx;

/* DMA Descriptors */
struct dma_descriptor example_descriptor_tx;
struct dma_descriptor example_descriptor_rx;

/****************************************************************/

/* === GLOBALS ============================================================ */
#define BATTERY_UPDATE_INTERVAL	(1) //1 second
#define BATTERY_MAX_LEVEL		(100)
#define BATTERY_MIN_LEVEL		(0)

uint8_t db_mem[1024] = {0};
bat_gatt_service_handler_t bas_service_handler;

bool volatile timer_cb_done = false;
bool volatile flag = true;
bool volatile battery_flag = true;
at_ble_handle_t bat_connection_handle;

void resume_cb(void);


static void transfer_done_tx(struct dma_resource* const resource )
{
	dma_start_transfer_job(&uart_dma_resource_tx);
}


/* Added By Meher Jain 
	for Assingment 5
	Call back for DMA RX Complete
	Receives the Command from Leopard Gecko over UART.
	
	Command Details:
	Size: 6 Bytes
	Header: 1 Byte. 
	'T' for temperature command, Rest 5 bytes for temperature value 
	'L' for Light Command, ext byte '0' for LED ON, 'F' for LED OFF, 4 bytes padded to make size consistent	 
*/
static void transfer_done_rx(struct dma_resource* const resource )
{	
	dma_start_transfer_job(&uart_dma_resource_rx);											// Restart the DMA transfer
	int i=0;	
	uint8_t temp_buffer[BUFFER_LEN+1];													
	strcpy(temp_buffer,string);																// Copy DMA buffer to local buffer
	memset(string,0,sizeof(string));														// Reset the DMA Buffer
	printf("\n");
	//printf("MJ: %s", temp_buffer);
	for(i=0;i<BUFFER_LEN;i++)
 		printf("%c",temp_buffer[i]);
	//printf("\n");
	temp_buffer[BUFFER_LEN] = '\0';															// NULL terminate for string functions to work
	if(temp_buffer[0] == LIGHT_SENSOR)														// IF light command
	{
		if(temp_buffer[1] == LEDON)
			gpio_pin_set_output_level(LED_0_PIN,LED_ON);									// Turn ON LED if ON in leopard gecko
					
		else if(temp_buffer[1] == LEDOFF)
			gpio_pin_set_output_level(LED_0_PIN,LED_OFF);									// Turn OFF LED if OFF in leopard gecko
	}	
	else if(temp_buffer[0] == TEMP_SENSOR)													// If temperature Data
	{
		strcpy(temperature_buffer,(temp_buffer+1));											// Copy in temperature buffer
		//printf("Temp:");	
		//for(i=0;i<8;i++)
			//printf("%c",temp_buffer[i]);
	}
	//printf("%ld",strlen(temp_buffer));	
	
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
	temperature = at30tse_read_temperature();
	//temperature = atof(temperature_buffer);
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

/**
 * \Timer callback handler called on timer expiry
 */
static void aon_sleep_timer_callback(void)
{
	timer_cb_done = true;
	send_plf_int_msg_ind(USER_TIMER_CALLBACK, TIMER_EXPIRED_CALLBACK_TYPE_DETECT, NULL, 0);
}

/* Advertisement data set and Advertisement start */
static at_ble_status_t battery_service_advertise(void)
{
	at_ble_status_t status = AT_BLE_FAILURE;
	
	if((status = ble_advertisement_data_set()) != AT_BLE_SUCCESS)
	{
		DBG_LOG("advertisement data set failed reason :%d",status);
		return status;
	}
	
	/* Start of advertisement */
	if((status = at_ble_adv_start(AT_BLE_ADV_TYPE_UNDIRECTED, AT_BLE_ADV_GEN_DISCOVERABLE, NULL, AT_BLE_ADV_FP_ANY, APP_BAS_FAST_ADV, APP_BAS_ADV_TIMEOUT, 0)) == AT_BLE_SUCCESS)
	{
		DBG_LOG("BLE Started Adv");
		return AT_BLE_SUCCESS;
	}
	else
	{
		DBG_LOG("BLE Adv start Failed reason :%d",status);
	}
	return status;
}

/* Callback registered for AT_BLE_PAIR_DONE event from stack */
static at_ble_status_t ble_paired_app_event(void *param)
{
	timer_cb_done = false;
	ALL_UNUSED(param);
	return AT_BLE_SUCCESS;
}

/* Callback registered for AT_BLE_DISCONNECTED event from stack */
static at_ble_status_t ble_disconnected_app_event(void *param)
{
	timer_cb_done = false;
	flag = true;
	
	aon_sleep_timer_service_stop();
	battery_service_advertise();
	ALL_UNUSED(param);
	return AT_BLE_SUCCESS;
}

static at_ble_status_t ble_connected_app_event(void *param)
{
	at_ble_connected_t *connected = (at_ble_connected_t *)param;
	bat_connection_handle = connected->handle;
	#if !BLE_PAIR_ENABLE
		ble_paired_app_event(param);
	#else
		ALL_UNUSED(param);
	#endif
	return AT_BLE_SUCCESS;
}

/* Callback registered for AT_BLE_NOTIFICATION_CONFIRMED event from stack */
static at_ble_status_t ble_notification_confirmed_app_event(void *param)
{
	at_ble_cmd_complete_event_t *notification_status = (at_ble_cmd_complete_event_t *)param;
	if(!notification_status->status)
	{
		flag = true;
		DBG_LOG_DEV("sending notification to the peer success");				
	}
	return AT_BLE_SUCCESS;
}

/* Callback registered for AT_BLE_CHARACTERISTIC_CHANGED event from stack */
static at_ble_status_t ble_char_changed_app_event(void *param)
{
	uint16_t device_listening;
	at_ble_characteristic_changed_t *char_handle = (at_ble_characteristic_changed_t *)param;

	if(bas_service_handler.serv_chars.client_config_handle == char_handle->char_handle)
	{
		device_listening = char_handle->char_new_value[1]<<8| char_handle->char_new_value[0];
		if(!device_listening)
		{		
			aon_sleep_timer_service_stop();			
		}
		else
		{
			aon_sleep_timer_service_init(1);
			aon_sleep_timer_service_start(aon_sleep_timer_callback);
		}			
	}	
	return bat_char_changed_event(char_handle->conn_handle,&bas_service_handler, char_handle, &flag);	
}

static const ble_event_callback_t battery_app_gap_cb[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	ble_connected_app_event,
	ble_disconnected_app_event,
	NULL,
	NULL,
	ble_paired_app_event,
	NULL,
	NULL,
	NULL,
	NULL,
	ble_paired_app_event,
	NULL,
	NULL,
	NULL,
	NULL
};

static const ble_event_callback_t battery_app_gatt_server_cb[] = {
	ble_notification_confirmed_app_event,
	NULL,
	ble_char_changed_app_event,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

void resume_cb(void)
{
	init_port_list();
	//uart_init(UART_HW_MODULE_UART1,&uart_cfg);
	serial_console_init();
}

/**
* \Battery Service Application main function
*/
int main(void)
{
	at_ble_status_t status;
	uint8_t battery_level = BATTERY_MIN_LEVEL;	
	
	platform_driver_init();
	acquire_sleep_lock();

	/* Initialize the button */
	/* button_init(); */
	
	/* Initialize serial console */
	serial_console_init();
	led_config();
	
	/* Initialize the hardware timer */
	hw_timer_init();
	/* Register the callback */
	hw_timer_register_callback(timer_callback_handler);
	/* Start timer */
	hw_timer_start(1);
	
	DBG_LOG("Initializing Battery Service Application");
	
	/* initialize the ble chip  and Set the device mac address */
	ble_device_init(NULL);
	
	/* Initialize the temperature sensor */
	at30tse_init();
	/* configure the temperature sensor ADC */
	at30tse_write_config_register(AT30TSE_CONFIG_RES(AT30TSE_CONFIG_RES_12_bit));
	/* read the temperature from the sensor */
	htp_temperature_read();
	
	
	
	/* Initialize the battery service */
	bat_init_service(&bas_service_handler, &battery_level);
	/* Initialize the htp service */
	htp_init();
	
	
	/* Define the primary service in the GATT server database */
	if((status = bat_primary_service_define(&bas_service_handler))!= AT_BLE_SUCCESS)
	{
		DBG_LOG("defining battery service failed %d", status);
	}
	
	battery_service_advertise();
	
	register_ble_callbacks();
	
	/* Register callbacks for gap related events */ 
	ble_mgr_events_callback_handler(REGISTER_CALL_BACK,
									BLE_GAP_EVENT_TYPE,
									battery_app_gap_cb);
									
	/* Register callbacks for gatt server related events */
	ble_mgr_events_callback_handler(REGISTER_CALL_BACK,
									BLE_GATT_SERVER_EVENT_TYPE,
									battery_app_gatt_server_cb);
									
	register_resume_callback(resume_cb);
	
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
	
	//release_sleep_lock();
	/* Capturing the events  */ 
	while (1) {
		/* BLE Event Task */
		ble_event_task(BLE_EVENT_TIMEOUT);
		if (Timer_Flag & Temp_Notification_Flag)
		{
			htp_temperature_send();
		}		
		
		//acquire_sleep_lock();
		if (timer_cb_done)
		{
			timer_cb_done = false;			
			/* send the notification and Update the battery level  */			
			if(flag){
				if(bat_update_char_value(bat_connection_handle,&bas_service_handler, battery_level, &flag) == AT_BLE_SUCCESS)
				{
					DBG_LOG("Battery Level:%d%%", battery_level);
				}
				if(battery_level == BATTERY_MAX_LEVEL)
				{
					battery_flag = false;
				}
				else if(battery_level == BATTERY_MIN_LEVEL)
				{
					battery_flag = true;
				}
				if(battery_flag)
				{
					battery_level++;
				}
				else
				{
					battery_level--;
				}
			}
		}
		//release_sleep_lock();
	}	
}


