/*
 * millis_tdd.h
 *
 *  Created on: 25 mars 2019
 *      Author: Vincent
 */

#ifndef TDD_MILLIS_TDD_H_
#define TDD_MILLIS_TDD_H_


#ifdef __cplusplus
extern "C" {
#endif


void millis_init(void);

uint32_t millis(void);

void delay_ms(uint32_t delay_);

void delay_us(uint32_t delay_);


#ifdef __cplusplus
}
#endif

#endif /* TDD_MILLIS_TDD_H_ */
