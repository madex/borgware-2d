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
    #include "animations/borg_time_esp.c"
    #include "animations/matrix.c"
    void pixel_init();
}
//#include "HexDump.h"
/* Configuration of NTP */
#define MY_NTP_SERVER "at.pool.ntp.org"           
#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"

volatile unsigned char fakeport = 0;
volatile unsigned char fakeportNet = 0;
uint8_t waitForFire = 0;

#include "EEPROM.h"

extern "C" {
  #include "pixel.h"
}
#define IRAM_ATTR __attribute__((section(".iram.text")))
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "wlan_key.h"
#include <time.h>                   // for time() ctime()
AsyncWebServer server(80);
/* Globals */
time_t now;                         // this are the seconds since Epoch (1970) - UTC
struct tm tm;                       // the structure tm holds time information in a more convenient way

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      256

#define GAMECUBE

#ifdef ESP8266
const int pin_ws2812    =  D1;
// joystick Pins
const int pin_gc        =  D5;
#else
const int pin_ws2812    =  6;
// joystick Pins
const int pin_joy_fire  =  1; 
const int pin_joy_up    =  2;
const int pin_joy_down  =  3;
const int pin_joy_right =  4;
const int pin_joy_left  =  5;

#endif

#ifdef GAMECUBE
static inline IRAM_ATTR uint32_t getCycleCount(void) {
  uint32_t ccount;
  __asm__ __volatile__("rsr %0,ccount":"=a" (ccount));
  return ccount;
}


void IRAM_ATTR myPinMode(uint8_t pin, uint8_t mode) {
    if (mode == OUTPUT) {
      GPF(pin) = GPFFS(GPFFS_GPIO(pin));//Set mode to GPIO
      GPC(pin) = (GPC(pin) & (0xF << GPCI)); //SOURCE(GPIO) | DRIVER(NORMAL) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
      GPES = (1 << pin); //Enable
    } else if (mode == INPUT) {
      GPF(pin) = GPFFS(GPFFS_GPIO(pin));//Set mode to GPIO
      GPEC = (1 << pin); //Disable
      GPC(pin) = (GPC(pin) & (0xF << GPCI)) | (1 << GPCD); //SOURCE(GPIO) | DRIVER(OPEN_DRAIN) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
    }
}  

void IRAM_ATTR joyEsp(uint32_t pin,  uint8_t *dataRead, uint32_t numBytes) {
    #define CYCLES_1US  (F_CPU / 1000000)
    #define CYCLES_2US  (F_CPU /  500000)
    #define CYCLES_3US  (F_CPU /  333333)
    #define CYCLES_4US  (F_CPU /  250000)
    uint8_t *p, *end, pix, mask, bit = 0;
    uint32_t t, period, c, startTime;
    uint32_t pinMask   = _BV(pin);
    uint8_t cmd[] = {0x40, 0x03, 0x02};
    p      =  cmd;
    end    =  p + sizeof(cmd);
    pix    = *p++;
    mask   = 0x80;
    period = CYCLES_4US;
    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pinMask); 
    myPinMode(pin, OUTPUT); 
    // send
    noInterrupts();
    while (1) {
        startTime = getCycleCount();                           // Save start time
        GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pinMask);        // Set high
        t = (pix & mask) ? CYCLES_1US : CYCLES_3US;            // Bit low duration
        while((getCycleCount() - startTime) < t);              // Wait low duration
        GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pinMask);        // Set High
        while ((getCycleCount() - startTime) < period);        // Wait for bit start
        if (!(mask >>= 1)) {                                   // Next bit/byte
            if (p >= end) break;
            pix  = *p++;
            mask = 0x80;
        }
    }
    t = CYCLES_1US;
    while ((getCycleCount() - startTime) < period);
    GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pinMask);
    startTime = getCycleCount();
    while ((getCycleCount() - startTime) < t);
    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pinMask);
    p = dataRead; 
    // read
    myPinMode(pin, INPUT);
    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pinMask);
    if (numBytes) {
        startTime = getCycleCount();
        uint8_t buf = 0U;
        while (((getCycleCount() - startTime) < period*4) && (GPIP(pin) == 1)) {} // Wait for startbit of answer
        if (GPIP(pin) == 1) {
            // not connected
        } else {
            while (1) {
                startTime = getCycleCount();
                while ((getCycleCount() - startTime) < CYCLES_2US-100);
                buf <<= 1;
                buf |= GPIP(pin); // read bit
                //GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, _BV(D2)); // DEBUG
                bit++;
                if (bit >= 8) {
                    *p++ = buf;
                    buf = 0; 
                    bit = 0;
                    numBytes--;
                    if (numBytes == 0) {
                        break; // finished
                    }
                }
                while ((GPIP(pin) == 0) && ((getCycleCount() - startTime) < CYCLES_3US+100)) {}  // wait for high
                //GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, _BV(D2)); // DEBUG
                while ((GPIP(pin) == 1) && ((getCycleCount() - startTime) < (CYCLES_3US+CYCLES_2US))) {} // wait start bit
            }
        }
    }
    interrupts(); // enable interrutp
    //GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, _BV(D2)); // DEBUG
}
#endif

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, pin_ws2812, NEO_GRB + NEO_KHZ800);

color_t colorMap[NUMPLANE+1] = {{0, 0, 0}, {3, 1, 0}, {20, 1, 0}, {140, 0, 0}};
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
bool color = false;
//#include "HexDump.h"
void show() {
  if (!color) {
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
  } else {
    color = false;
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
}

void set_color_frame(void) {
  color = true;
}

/**
 * @brief Set the color of a pixel in c
 * 
 * @param x x-coord 0-15
 * @param y y-coord 0-15
 * @param r red
 * @param g green
 * @param b blue
 */
void set_color_pixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
  if ((x < 16) && (y < 16)) {
    pixels.setPixelColor(mapPix[x][y], pixels.Color(r, g, b));
  }
}

uint8_t joyRead() {
  return 0;
  
}
extern "C" {
//=============================================================================

  void eeprom_write_byte(uint8_t *p, uint8_t value) {
    EEPROM.write((uint32_t) p, value);
    #ifdef ESP8266    
    EEPROM.commit();
    #endif
  }
  void eeprom_write_word(uint16_t *p, uint16_t value) {
    EEPROM.write((uint32_t) p, value);
    #ifdef ESP8266    
    EEPROM.commit();
    #endif
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
    long now = millis();
    uint8_t data[4] = {0, 0, 0, 0};
    if (((now - lastTimeShow) >= 20) || (ms >= 20)) {
      lastTimeShow = now;
      show();
      #ifdef GAMECUBE
        joyEsp(pin_gc, data, sizeof data);
        bool up    = ((data[1] & 0x08) != 0) || (data[3] > 0xa0);
        bool down  = ((data[1] & 0x04) != 0) || ((data[3] < 0x60) && (data[3] > 0));
        bool left  = ((data[1] & 0x01) != 0) || ((data[2] < 0x60) && (data[2] > 0));
        bool right = ((data[1] & 0x02) != 0) || (data[2] > 0xa0);
        bool fire  = (data[0] & 0x01) != 0;
        fakeport = (fire  ? 0x01 : 0) |
                  (left  ? 0x02 : 0) |
                  (right ? 0x04 : 0) |
                  (down  ? 0x08 : 0) |
                  (up    ? 0x10 : 0);
      #endif
      fakeport |= fakeportNet;
      fakeportNet = 0;
      yield();
    }
    long cur;
    #ifdef JOYSTICK_SUPPORT
      //static uint8_t fakeport_old;
      #ifdef GAMECUBE
        //if (fakeport_old != fakeport) {
            //GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, _BV(D0));
            //digitalWrite(D0, 1);
            //fakeport_old = fakeport;
            //Serial.print("fakeport = ");
            //Serial.printf("%02x  data = %02x %02x %02x %02x\n", fakeport, data[0], data[1], data[2], data[3]);
            //digitalWrite(D0, 0);
            //GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, _BV(D0));
        //}
      #endif
      if (waitForFire) {
        if (fakeport == 1) {  //if (JOYISFIRE) {
          start_game_menu();
        }
      }
    #endif
    do {
      cur = millis();
      if (lastTime > cur) {
        lastTime -= (1 << 31);
      }
      uint16_t timeVergangen = (cur - lastTime);
      if (timeVergangen < ms) {
        delay(ms - timeVergangen);
        now = cur;
      }
    } while ((cur - lastTime) < ms);
    lastTime = cur;
  }
}

void setup() {
  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);

  initMap();
  pixels.begin();
#ifdef ESP8266
  EEPROM.begin(512);
#endif
  Serial.begin(115200);
  //pinMode(D0, OUTPUT);
  //pinMode(D2, OUTPUT);
  configTime(MY_TZ, MY_NTP_SERVER); // --> Here is the IMPORTANT ONE LINER needed in your sketch!

  WiFi.begin (WLAN_SSID, WLAN_PASSWORD);
  // Wait for connection
  uint8_t timout = 20;
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
    if (--timout == 0) {
      char test[256];
      sprintf(test, "</# Timeout WLAN=" WLAN_SSID " not found !!! ");
      scrolltext(test);
      Serial.print ( "WLAN=" );
      Serial.print ( WLAN_SSID );
      Serial.println ( " not found" );
      return;
    }
  }
  Serial.println ( "" );
  Serial.print ( "Connected to " );
  Serial.println ( WLAN_SSID );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );
  
  if ( MDNS.begin ( "borg" ) ) {
    Serial.println ( "MDNS responder started" );
  }
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", R"(
  <!DOCTYPE html>
  <html><body>
  <h1>ESP borgware-2d</h1>
  <input type="text" id="in" size="40" onkeydown="keyEv(event) "><br>
  <a href="/update">FW-Update</a><br>
  <script>function keyEv(event){var x = event.which || event.keyCode;
  let xhr = new XMLHttpRequest(), req;
      switch(x) {
          case 32: req="f";break;
          case 37: case 65: req="l";break;
          case 39: case 68: req="r";break;
          case 38: case 87: req="u";break;
          case 40: case 83: req="d";break;
          default: return;
      }
      xhr.open("GET", req, true); xhr.send();
      document.getElementById("in").value = "";
  }
  </script></body></html>
  )");
        
  });
  server.on("/l", HTTP_GET, [](AsyncWebServerRequest *request) {
    fakeportNet |= 2;
    request->send ( 200, "text/plain", "" );
  });
  server.on("/r", HTTP_GET, [](AsyncWebServerRequest *request) {
    fakeportNet |= 4;
    request->send ( 200, "text/plain", "" );
  });
  server.on("/u", HTTP_GET, [](AsyncWebServerRequest *request) {
    fakeportNet |= 16;
    request->send ( 200, "text/plain", "" );
  });
  server.on("/d", HTTP_GET, [](AsyncWebServerRequest *request) {
    fakeportNet |= 8;
    request->send ( 200, "text/plain", "" );
  });
  server.on("/f", HTTP_GET, [](AsyncWebServerRequest *request) {
    fakeportNet |= 1;
    request->send ( 200, "text/plain", "" );
  });
  AsyncElegantOTA.begin(&server);
  server.begin();
  Serial.println("HTTP server started");
  char test[256];
  IPAddress ip = WiFi.localIP();
  sprintf(test, "</#   WLAN=" WLAN_SSID " ip= %d.%d.%d.%d ", ip[0], ip[1], ip[2], ip[3]);
  //scrolltext(test);
}

void loop() {
    display_loop();
}
