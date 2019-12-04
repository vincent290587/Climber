/*
 * vnh5019_driver.c
 *
 *  Created on: 28 nov. 2019
 *      Author: Vincent
 */

#include <stdint.h>
#include "gpio.h"
#include "boards.h"
#include "millis.h"
#include "helper.h"
#include "segger_wrapper.h"
#include "vnh5019_driver.h"


int16_t m_vnh_speed_mm_s = 0;
uint32_t m_calc_last_time = 0;

int16_t m_sim_length = 0;

int16_t tdd_vnh5019_driver__get_length(void) {

	int16_t delta = m_vnh_speed_mm_s * (millis() - m_calc_last_time);

	if (m_sim_length + delta > 200) m_sim_length = 200;
	if (m_sim_length + delta <   0) m_sim_length = 0;

	m_calc_last_time = millis();

	return m_sim_length + 105;
}

void vnh5019_driver__init(void) {

}

void vnh5019_driver__setM1Speed(int16_t speed_mm_s)
{
	m_vnh_speed_mm_s = speed_mm_s;
}

void vnh5019_driver__setM1Brake(uint16_t brake)
{

}

// Return motor 1 current value in milliamps.
uint32_t vnh5019_driver__getM1CurrentMilliamps(void)
{
	// 5V / 1024 ADC counts / 144 mV per A = 34 mA per count
	return 0; // analogRead(VNH_CS1) * 34;
}

// Return error status for motor 1
uint32_t vnh5019_driver__getM1Fault(void)
{
	return !gpio_get(VNH_DIAG1);
}
