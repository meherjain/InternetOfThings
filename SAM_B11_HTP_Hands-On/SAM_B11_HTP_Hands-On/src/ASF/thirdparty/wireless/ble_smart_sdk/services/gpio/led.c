/*
 * led.c
 *
 * Created: 2015-11-04 오후 2:38:05
 *  Author: mksong
 */

#include <asf.h>
#include "led.h"

void led_init(void)
{
	struct gpio_config config_gpio_pin;

	gpio_get_config_defaults(&config_gpio_pin);

	config_gpio_pin.direction = GPIO_PIN_DIR_OUTPUT;
	config_gpio_pin.input_pull = GPIO_PIN_PULL_NONE;

	gpio_pin_set_config(LED_0_PIN, &config_gpio_pin);
}
