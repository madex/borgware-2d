#ifndef UTIL_H
#define UTIL_H
#include <stdbool.h>
typedef struct {
  unsigned char r, g, b;
} color_t;
void b2d_wait(int ms);
void set_color_pixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
void set_color_frame(void);

// good idea to remember
#ifndef BITS_H  
#define BITS_H

#define BIT4(b3, b2, b1, b0) (((b3 & 1) << 3) | ((b2 & 1) << 2) | ((b1 & 1) << 1) | (b0 & 1))
#define BYTE(b7, b6, b5, b4, b3, b2, b1, b0) \
    (BIT4(b7, b6, b5, b4) << 4) | BIT4(b3, b2, b1, b0)
#define BYTE_REV(b0, b1, b2, b3, b4, b5, b6, b7) \
	BYTE(b7, b6, b5, b4, b3, b2, b1, b0)

#endif /* BITS_H */

#endif
