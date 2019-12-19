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


int16_t m_vnh_duty_cycle = 0;
uint32_t m_calc_last_time = 0;
float m_vnh_speed_mm_s = 0.0f;

static float m_sim_length = 25;

int16_t tdd_vnh5019_driver__get_length(void) {

	// map actuator speed to command
	m_vnh_speed_mm_s = regFenLim(m_vnh_duty_cycle, -50.0f, 50.0f, -16.0f, 16.0f);

	float delta = (float)m_vnh_speed_mm_s * (float)(millis() - m_calc_last_time) / 1000.0f;

	if (m_sim_length + delta > 200) {
		m_sim_length = 200;
	} else
	if (m_sim_length + delta <   0) {
		m_sim_length = 0;
	} else {
		m_sim_length += delta;
	}

	m_calc_last_time = millis();

	return (int16_t)m_sim_length + 300;
}

void vnh5019_driver__init(void) {

	w_task_delay(40);
}

void vnh5019_driver__setM1_duty(int16_t s_duty_cycle)
{
	m_vnh_duty_cycle = s_duty_cycle;
}

int16_t vnh5019_driver__getM1_duty(void)
{
	return m_vnh_duty_cycle;
}

void vnh5019_driver__setM1Brake(uint16_t brake)
{

}

// Return motor 1 current value in milliamps.
int32_t vnh5019_driver__getM1CurrentMilliamps(void)
{
	// 5V / 1024 ADC counts / 144 mV per A = 34 mA per count
	return 0; // analogRead(VNH_CS1) * 34;
}

// Return error status for motor 1
uint32_t vnh5019_driver__getM1Fault(void)
{
	return !gpio_get(VNH_DIAG1);
}
