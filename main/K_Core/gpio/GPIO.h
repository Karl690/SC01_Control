#pragma once

#include "main.h"

typedef enum {
	GPIO_INPUT,
	GPIO_OUTPUT,
} GPIO_MODE;

void gpio_init();
void gpio_set_mode(gpio_num_t pin, GPIO_MODE mode);