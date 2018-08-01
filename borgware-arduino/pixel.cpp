#define PIXEL_C

#include "config.h"

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

color_t colorMap[NUMPLANE+1] = {{0, 0, 0}, {10, 0, 0}, {50, 0, 0}, {150, 0, 0}};
const unsigned char shl_table[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
uint8_t mapPix[16][16];

void initMap() {
  uint8_t i = 0;
  for (uint8_t x = 0; x < 16; x++) {
    for (uint8_t y = 0; y < 16; y++) {
      uint8_t y2 = ((x % 2) == 0 ? y : 15-y);
      mapPix[x][y2] = i++;
    }
  }
}

unsigned char pixmap[NUMPLANE][NUM_ROWS][LINEBYTES];

void pixel_init() {
  initMap();
  pixels.begin();
}

void clear_screen(unsigned char value) {
	unsigned char p, *pix,v = 0xff;
	unsigned int i;
	for (p=0;p<NUMPLANE;p++){
		pix=&pixmap[p][0][0];
		if(p==value)
			v=0;
		for(i=0;i<NUM_ROWS*LINEBYTES;i++)
			pix[i]=v;
	}
}

void show() {
  for (short x = 0; x < NUM_COLS; x++) {
    for (short y = 0; y < NUM_ROWS; y++) {
      short col = 0;
      for (short level = 0; level < NUMPLANE; level++) {
        if (pixmap[level][y % NUM_ROWS][x / 8] & (1 << x % 8)) {
          col = level + 1;
        }
      }
      color_t *c = &colorMap[col];
      pixels.setPixelColor(mapPix[x][y], pixels.Color(c->r, c->g, c->b)); 
    }
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
}

void b2d_wait(int ms) {
  long lastTime = millis(), curTime;
  show();
  do {
    curTime = millis();
  } while ((curTime - lastTime) >= ms);
}

void setpixel(pixel p, unsigned char value ){
	p.x %= NUM_COLS;
	if(value>NUMPLANE)
		value=NUMPLANE;
	unsigned char pos = (p.y%NUM_ROWS)*LINEBYTES + (p.x/8);
	unsigned char mask = shl_table[p.x%8];
	unsigned char plane;
	for(plane=0;plane<value;plane++)
		pixmap[plane][0][pos]|=mask;
	mask ^=0xff;
	for(;plane<NUMPLANE;plane++)
		pixmap[plane][0][pos]&=mask;
}


//shifts pixmap left. It is really shifted right, but because col0 is left in the Display it's left.
void shift_pixmap_l() {
	unsigned char plane, row, byte;
	
	for(plane=0; plane<NUMPLANE; plane++){
		for(row=0;row<NUM_ROWS; row++){
			for(byte=0;byte<(LINEBYTES-1);byte++){
				pixmap[plane][row][byte] >>= 1;
				pixmap[plane][row][byte] |= (pixmap[plane][row][byte+1] & 0x01) * 0x80;;
			}
				pixmap[plane][row][LINEBYTES-1] >>= 1;
		}
	}
}


unsigned char get_pixel(pixel p){

	if( (p.x >= NUM_COLS) || (p.y >= NUM_ROWS) ){
		return 0xff;
	}else{
		return 0!= (pixmap[0][p.y][p.x/8] & shl_table[p.x%8]);
	}
}

/**
 * An implementation of Bresenham's line drawing algorithm.
 * @param p1 first coordinate of the line
 * @param p2 second coordinate of the line
 * @param color brightness level of the line
 */
void line(pixel p1,
          pixel const p2,
          unsigned char const color)
{
	operand_t const dx = p1.x < p2.x ? p2.x - p1.x : p1.x - p2.x;
	operand_t const sx = p1.x < p2.x ? 1 : -1;
	operand_t const dy = p1.y < p2.y ? p2.y - p1.y : p1.y - p2.y;
	operand_t const sy = p1.y < p2.y ? 1 : -1;
	operand_t error = dx - dy;

	while(1)
	{
		setpixel(p1, color);
		if ((p1.x == p2.x) && (p1.y == p2.y))
			break;
		operand_t const error2 = 2 * error;
		if (error2 > -dy)
		{
			error -= dy;
			p1.x += sx;
		}
		if (error2 < dx)
		{
			error += dx;
			p1.y += sy;
		}
	}
}
