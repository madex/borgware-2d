#define PIXEL_C

#include "config.h"
#include "EEPROM.h"

extern "C" {
	#include "pixel.h"
}

#include <Adafruit_NeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            5

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      256

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
typedef struct {
  unsigned char r, g, b;
} color_t;
color_t colorMap[NUMPLANE+1] = {{0, 0, 0}, {3, 1, 0}, {20, 1, 0}, {130, 0, 0}};
const unsigned char shl_table[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
uint8_t mapPix[16][16];
unsigned char pixmap[NUMPLANE][NUM_ROWS][LINEBYTES];

void initMap() {
  uint8_t i = 0;
  for (uint8_t x = 0; x < 16; x++) {
    for (uint8_t y = 0; y < 16; y++) {
      uint8_t y2 = ((x % 2) == 0) ? y : 15-y;
      mapPix[x][y2] = i++;
    }
  }
}

void pixel_init() {
  initMap();
  pixels.begin();
  Serial.begin(115200);
}

//#include "HexDump.h"
void show() {
  for (short x = 0; x < NUM_COLS; x++) {
    for (short y = 0; y < NUM_ROWS; y++) {
      short col = 0;
      for (short level = 0; level < NUMPLANE; level++) {
        if (pixmap[level][y % NUM_ROWS][x / 8] & (1 << (x % 8))) {
          col = level + 1;
        }
      }
      color_t *c = &colorMap[col];
      pixels.setPixelColor(mapPix[x][y], pixels.Color(c->r, c->g, c->b)); 
    }
  }
  //HexDump(Serial, pixels.getPixels(), pixels.numPixels()*3);
  pixels.show(); // This sends the updated pixel color to the hardware.
}

extern "C" {

  void eeprom_write_byte(uint8_t *p, uint8_t value) {
    EEPROM.write((uint32_t) p, value);
    EEPROM.commit();
  }
  void eeprom_write_word(uint16_t *p, uint16_t value) {
    EEPROM.write((uint32_t) p, value);
    EEPROM.commit();
  }
  uint8_t  eeprom_read_byte (const uint8_t *p) {
    return (uint8_t) EEPROM.read((uint32_t) p);
  }
  uint16_t eeprom_read_word (const uint16_t *p) {
    return (uint16_t) EEPROM.read((uint32_t)p);
  }
  
  void b2d_wait(int ms) {
    static long lastTime = millis();
    static long lastTimeShow = lastTime-8;
    long start = millis();
    if ((start - lastTimeShow) > 8) {
      lastTimeShow = start;
      show();
    }
    long cur;
    delay(1);
    do {
      cur = millis();
      if (lastTime > cur) {
        lastTime -= (1 << 31);
      }
    } while ((cur - lastTime) < ms);
    lastTime += ms;
  }
}
