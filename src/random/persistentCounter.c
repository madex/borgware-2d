/**
 * \file	persistentCounter.c
 * \author	Daniel Otte 
 * \brief	a persistent 24-bit counter in EEPROM for ATmega µC
 * 
 */

#include <stdint.h>
#include "../compat/interrupt.h" /* cli() & sei() */
#include "../compat/eeprom.h"
#include "../config.h"
#include "persistentCounter.h"

#ifdef ERROR_HANDLING
#	include "error-handling.h"
#	define PERSISTENT_COUNTER_OVERFLOW		(void*)0, 2,4,1
#	define PERSISTENT_COUNTER_WRITER_ERROR	(void*)0, 2,4,2
#endif

uint8_t g_reset_counter_idx = 0xff;
percnt_t g_reset_counter;

#ifdef INIT_EEPROM
void init_buffer(percnt_t *percnt, uint8_t *ring_index) {
	uint8_t i;
	eeprom_busy_wait();
	eeprom_write_word((uint16_t*) (EEP_PER_COUNTER + offsetof(percnt_t, B08_23)), 0x0000);
	for (i = 0; i < RING_SIZE; ++i) {
		eeprom_busy_wait();
		eeprom_write_byte((uint8_t*) (EEP_PER_COUNTER + offsetof(percnt_t, B0_7[0]) + i), 0x00);
	}
}
#endif

void percnt_init(percnt_t *percnt, uint8_t *ring_index) {
	uint8_t i;
	uint8_t t, max = eeprom_read_byte((uint8_t*) (EEP_PER_COUNTER + offsetof(percnt_t, B0_7[0])));
#ifdef INIT_EEPROM
	/* test if the 2 MSB == 0xFFFF */
	if (eeprom_read_word((uint16_t*) (EEP_PER_COUNTER + offsetof(percnt_t, B08_23))) == 0xFFFF) {
		/* test the first two bytes of ringbuffer */
		if (eeprom_read_word((uint16_t*) (EEP_PER_COUNTER + offsetof(percnt_t, B0_7[0]))) == 0xFFFF) {
			init_buffer(percnt, ring_index);
		}
	}
#endif
	eeprom_busy_wait();
	uint8_t lasti = RING_SIZE - 1;
	uint16_t last = eeprom_read_byte((uint8_t*) (EEP_PER_COUNTER + offsetof(percnt_t, B0_7[0]) + lasti));
	/* might be faster, but such optimizations are prone to timing attacks */
	for (i = 0; i < RING_SIZE; ++i) {
		eeprom_busy_wait();
		t = eeprom_read_byte((uint8_t*) (EEP_PER_COUNTER + offsetof(percnt_t, B0_7[0]) + i));
		if (((last + 1) & 0xff) == t) { 
			lasti = i;
			last = t;
		} else if (last != t) { // find diff bigger 1 but not init state
			break; // found
		}
	}
	*ring_index = lasti;
}

uint32_t percnt_get(percnt_t *percnt, uint8_t *ring_index) {
	uint32_t ret = 0;

	if (*ring_index == 0xff)
		percnt_init(percnt, ring_index);
	cli();
	eeprom_busy_wait();
	ret  = (uint32_t)eeprom_read_word((uint16_t*) (EEP_PER_COUNTER + offsetof(percnt_t, B08_23))) << 8;
	eeprom_busy_wait();
	ret |= eeprom_read_byte((uint8_t*) (EEP_PER_COUNTER + offsetof(percnt_t, B0_7[0]) + *ring_index));
	sei();
	return ret;
}

void percnt_inc(percnt_t *percnt, uint8_t *ring_index) {
	/* we must make this resistant agaist power off while this is running ... */
	uint32_t u;

	if (*ring_index == 0xff)
		percnt_init(percnt, ring_index);

	u = percnt_get(percnt, ring_index);
	cli();
	/* it's important to write msb first! */
	if ((u & 0x000000ff) == 0xff) {
		if ((u & 0x0000ffff) == 0xffff) {
			if ((u & 0x00ffffff) == 0xffffff) {
				/* we can turn the lights off now. it's time to die */
#ifdef ERROR_HANDLING
				error(PERSISTENT_COUNTER_OVERFLOW);
#endif
			}
			eeprom_busy_wait();
			eeprom_write_byte((uint8_t*) (EEP_PER_COUNTER + offsetof(percnt_t, B08_23) + 1), // highByte
					((u + 1) >> 16) & 0xff);
		}
		eeprom_busy_wait();
		eeprom_write_byte((uint8_t*) (EEP_PER_COUNTER + offsetof(percnt_t, B08_23)),
				((u + 1) >> 8) & 0xff);
	}
	/* set least significant byte (in ringbuffer) */
	*ring_index = (*ring_index + 1) % RING_SIZE;
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t*) (EEP_PER_COUNTER + offsetof(percnt_t, B0_7[0]) + *ring_index), (u + 1) & 0xff);
	eeprom_busy_wait();

#ifdef ERROR_HANDLING
	if (u + 1 != percnt_get(percnt, ring_index)) {
		error(PERSISTENT_COUNTER_WRITER_ERROR);
	}
#endif	

	sei();
}

