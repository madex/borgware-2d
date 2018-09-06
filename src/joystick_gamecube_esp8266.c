// This is a mash-up of the Due show() code + insights from Michael Miller's
// ESP8266 work for the NeoPixelBus library: github.com/Makuna/NeoPixelBus
// Needs to be a separate .c file to enforce ICACHE_RAM_ATTR execution.

#ifdef ESP8266
/*
#include <Arduino.h>
#include <eagle_soc.h>
extern void __pinMode(uint8_t pin, uint8_t mode);
extern int ICACHE_RAM_ATTR __digitalRead(uint8_t pin);
static uint32_t _getCycleCount(void) __attribute__((always_inline));
static inline uint32_t _getCycleCount(void) {
  uint32_t ccount;
  __asm__ __volatile__("rsr %0,ccount":"=a" (ccount));x^
  return ccount;
}

void ICACHE_RAM_ATTR joyEsp(uint8_t pin, uint8_t *data, uint32_t numBytes) {

#define CYCLES_1US  (F_CPU / 1000000)
#define CYCLES_2US  (F_CPU /  500000)
#define CYCLES_3US  (F_CPU /  333333)
#define CYCLES_4US  (F_CPU /  250000)

    uint8_t *p, *end, pix, mask;
    uint32_t t, time0, time1, period, c, startTime, pinMask;
    uint8_t cmd[] = {0x40, 0x03, 0x02};
    pinMask   = _BV(pin);
    p         =  cmd;
    end       =  p + sizeof(cmd);
    pix       = *p++;
    mask      = 0x80;
    startTime = 0;
    time0  = CYCLES_3US;
    time1  = CYCLES_1US;
    period = CYCLES_4US;
    __pinMode(pin, OUTPUT);
    // send
    for (t = time0;; t = time0) {
        if (pix & mask) t = time1;                             // Bit high duration
        while (((c = _getCycleCount()) - startTime) < period); // Wait for bit start
        GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pinMask);        // Set high
        startTime = c;                                         // Save start time
        while(((c = _getCycleCount()) - startTime) < t);       // Wait high duration
        GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pinMask);        // Set low
        if (!(mask >>= 1)) {                                   // Next bit/byte
            if (p >= end) break;
            pix  = *p++;
            mask = 0x80;
        }
    }
    while((_getCycleCount() - startTime) < period); // Wait for last bit
    p = data;
    //yield();
    __pinMode(pin, INPUT);
    time0 = CYCLES_2US;
    uint8_t bit = 0;
    uint8_t buf = 0;
    for (;;) {                         // Bit high duration
        startTime = c;
        while (((c = _getCycleCount()) - startTime) < CYCLES_2US); // Wait for bit start
        buf |= __digitalRead(pin);
        buf <<= 1;
        bit++;
        if (bit >= 8) {
            *p++ = buf;
            buf = 0; bit = 0;
            numBytes--;
            if (numBytes == 0) break;
        }
        while((_getCycleCount() - startTime) < CYCLES_3US); // Wait for last bit
        while((__digitalRead(pin) == 1) || 
              ((_getCycleCount() - startTime) < (CYCLES_3US+CYCLES_2US)));
    }

}*/

#endif // ESP8266
