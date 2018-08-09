#include "config.h"
extern "C" {
    #include <setjmp.h>
    #include "pixel.h"
    //#include "mcuf/mcuf.c"
    #include "display_loop.h"
    #include "user/user_loop.c"
    #include "eeprom_reserve.c"
    #include "scrolltext/font_c64.c"
    #include "scrolltext/font_arial8.c"
    #include "scrolltext/font_small6.c"
    #include "scrolltext/font_uni53.c"
    #include "scrolltext/scrolltext3.c"
    #include "menu/menu.c"
    //#include "joystick/rfm12_joystick.c"
    #include "joystick/null_joystick.c"
    //#include "joystick/nes_pad.c"
    //#include "joystick/hc165_joystick.c"
    //#include "joystick/joystick.c"
    //#include "joystick/lolshield_joystick.c"
    #include "random/noekeon.c"
    #include "random/prng.c"
    #include "random/persistentCounter.c"
    #include "random/memxor_c.c"
    #include "smallani/rowwalk.c"
    #include "smallani/rowbounce.c"
    #include "smallani/colwalk.c"
    #include "smallani/colbounce.c"
    #include "games/kart/kart.c"
    #include "games/tetris/variant_fp.c"
    #include "games/tetris/variant_bastet.c"
    #include "games/tetris/input.c"
    #include "games/tetris/variant_std.c"
    #include "games/tetris/highscore.c"
    #include "games/tetris/piece.c"
    #include "games/tetris/bucket.c"
    #include "games/tetris/tetris_main.c"
    #include "games/tetris/view.c"
    #include "games/space_invaders/invader_proc.c"
    #include "games/space_invaders/invaders2.c"
    #include "games/space_invaders/invader_init.c"
    #include "games/space_invaders/invader_draw.c"
    #include "games/breakout/level.c"
    #include "games/breakout/playfield.c"
    #include "games/breakout/rebound.c"
    #include "games/breakout/score.c"
    #include "games/breakout/messages.c"
    #include "games/breakout/ball.c"
    #include "games/breakout/breakout.c"
    #include "games/snake/snake_game.c"
    //#include "uart/uart_commands.c"
    //#include "uart/uart.c"
    #include "animations/breakout_demo.c"
    #include "animations/flyingdots.c"
    #include "animations/squares.c"
    #include "animations/ltn_ant.c"
    #include "animations/stonefly.c"
    #include "animations/program.c"
    #include "animations/snake.c"
    #include "animations/gameoflife.c"
    #include "animations/dna.c"
    #include "animations/blackhole.c"
    #include "animations/fpmath_patterns.c"
    #include "animations/mherweg.c"
    #include "animations/bitmapscroller/amphibian.c"
    #include "animations/bitmapscroller/outofspec.c"
    #include "animations/bitmapscroller/fairydust.c"
    #include "animations/bitmapscroller/laborlogo.c"
    #include "animations/bitmapscroller/bitmapscroller.c"
    #include "animations/bitmapscroller/thisisnotdetroit.c"
    #include "animations/moire.c"
    //#include "animations/borg_time.c"
    #include "animations/matrix.c"
    void pixel_init();
}
#include "HexDump.h"
volatile unsigned char fakeport = 0;
uint8_t waitForFire = 0;

#include "EEPROM.h"

extern "C" {
  #include "pixel.h"
}

#include <Adafruit_NeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            D1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      256

// joystick Pins
const int pin_joy_fire  =  D0; 
const int pin_joy_up    =  D3;
const int pin_joy_down  =  D6;
const int pin_joy_right =  D7;
const int pin_joy_left  =  D4;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
typedef struct {
  unsigned char r, g, b;
} color_t;
color_t colorMap[NUMPLANE+1] = {{0, 0, 0}, {3, 1, 0}, {20, 1, 0}, {130, 0, 0}};
uint8_t mapPix[16][16];
unsigned char pixmap[NUMPLANE][NUM_ROWS][LINEBYTES];

void initMap() {
  uint8_t i = 0;
  for (uint8_t x = 0; x < 16; x++) {
    for (uint8_t y = 0; y < 16; y++) {
      uint8_t y2 = ((x % 2) == 0) ? y : 15-y;
      mapPix[15-x][y2] = i++;
    }
  }
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
  //hexdump(pixels.getPixels(), pixels.numPixels()*3);
  static jmp_buf jmp_buf_old; 
  if (memcmp(jmp_buf_old, newmode_jmpbuf, sizeof jmp_buf_old) != 0) {
    HexDump(Serial, newmode_jmpbuf, sizeof newmode_jmpbuf); 
    memcpy(jmp_buf_old, newmode_jmpbuf, sizeof jmp_buf_old);
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
}

extern "C" {
//=============================================================================

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
    static long lastTime = 0;
    static long lastTimeShow = -20;
    long start = millis();
    if ((start - lastTimeShow) >= 20) {
      lastTimeShow = start;
      show();
      delay(0);
    }
    static uint8_t fakeport_old;
    fakeport = ((digitalRead(pin_joy_fire)  == LOW) ? 0x01 : 0) |
               ((digitalRead(pin_joy_left)  == LOW) ? 0x02 : 0) |
               ((digitalRead(pin_joy_right) == LOW) ? 0x04 : 0) |
               ((digitalRead(pin_joy_down)  == LOW) ? 0x08 : 0) |
               ((digitalRead(pin_joy_up)    == LOW) ? 0x10 : 0);
    if (fakeport_old != fakeport) {
        fakeport_old = fakeport;
        Serial.print("fakeport = ");
        Serial.println(fakeport, HEX);
    }
    long cur;
    #ifdef JOYSTICK_SUPPORT
		if (waitForFire) {
			if (JOYISFIRE) {
        //Serial.println(fakeport, HEX);
				startGameMenu();
			}
		}
    #endif
    do {
      cur = millis();
      if (lastTime > cur) {
        lastTime -= (1 << 31);
      }
      if ((cur - start) > 2) {
        delay(1);
        start = cur;
      }   
    } while ((cur - lastTime) < ms);
    lastTime = cur;
  }
}

void setup() {
  initMap();
  pinMode(pin_joy_fire,  INPUT_PULLUP);
  pinMode(pin_joy_up,    INPUT_PULLUP);
  pinMode(pin_joy_down,  INPUT_PULLUP);
  pinMode(pin_joy_right, INPUT_PULLUP);
  pinMode(pin_joy_left,  INPUT_PULLUP);
  pixels.begin();
  EEPROM.begin(512);
  Serial.begin(115200);
#ifdef DEBUG_ESP_PORT
  Serial.println("hat DEBUG_ESP_PORT");
#endif
  
}

void loop() {
    display_loop();
}

