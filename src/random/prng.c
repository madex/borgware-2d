/**
 * \author	Daniel Otte
 * \date	2008-08-24
 * \license GPLv3 or later
 * \brief   random number generator based on noekeon running in CFB-mode
 * 
 */

#include <stdint.h>
#include <string.h>
#include "noekeon.h"

uint8_t random_state[16] __attribute__((aligned(4)));
uint8_t random_key[16]   __attribute__((aligned(4)));

uint8_t random8(void){
	static uint8_t sr[16];
	static uint8_t i=0;	
	
	if (i==0) {
		noekeon_enc(random_state, random_key);
		memcpy(sr, random_state, 16);
		i=15;
		return sr[15];
	}
	--i;
	return sr[i];
}
