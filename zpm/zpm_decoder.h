/*
 * decoder.h
 *
 *  Created on: 17 déc. 2019
 *      Author: vgol
 */

#ifndef ZPM_ZPM_DECODER_H_
#define ZPM_ZPM_DECODER_H_

#include <stdint.h>


#if defined(__cplusplus)
extern "C" {
#endif

void zpm_decoder__handle(uint8_t *data, uint16_t length);

#if defined(__cplusplus)
}
#endif

#endif /* ZPM_ZPM_DECODER_H_ */
