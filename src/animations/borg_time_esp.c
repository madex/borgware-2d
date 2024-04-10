/*
 * Description:	Request time strings from a can-master
 * 				and show them in an animation
 * Author: 		hansi
 */

#include <stdio.h>
#include <stdlib.h>
#include "../compat/pgmspace.h"
#include "../util.h"
#include "../scrolltext/scrolltext.h"
#include <time.h>

extern time_t now; 
extern tm tm;

#define DRAWN_NUM_CLOCK
#ifdef DRAWN_NUM_CLOCK

const color_t pal_green[] = { 
	{.r=0,  .g=0,  .b=0},   // 0
	{.r=30, .g=130,.b=20}   // 1
};

const color_t pal_blue[] = { 
	{.r=0,  .g=0,  .b=0},   // 0
	{.r=30, .g=30, .b=120}, // 1
};

const color_t pal_orange[] = {
	{.r=0,  .g=0,   .b=0},  // 0
	{.r=130,.g=90,  .b=0},  // 1	
	{.r=30, .g=130, .b=20}  // 2 green
};

typedef struct {
	uint8_t num, len_pal;
	const color_t *pal;
	uint8_t data[8][8];
} num_t;

#define PAL(palette) .len_pal = sizeof(palette)/sizeof(palette[0]), .pal = palette

const num_t num[] = {	
	{ PAL(pal_green),
	  .data = { // 0 martin
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 1, 0, 0, 1, 0, 0},
		{0, 0, 1, 0, 0, 1, 0, 0},
		{0, 0, 1, 0, 0, 1, 0, 0},
		{0, 0, 1, 0, 0, 1, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
	  },
	},
	{ PAL(pal_blue),
	  .data = { // 1 moritz
		{0, 0, 0, 0, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 1, 0, 1, 0, 0, 0},
		{0, 0, 0, 0, 1, 0, 0, 0},
		{0, 0, 0, 0, 1, 0, 0, 0},
		{0, 0, 0, 0, 1, 0, 0, 0},
		{0, 0, 0, 0, 1, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
	  }, 
	},
		{ PAL(pal_blue),
		.data = { // 2 moritz
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 0, 0},
		{0, 1, 0, 0, 0, 1, 0, 0},
		{0, 0, 0, 0, 1, 0, 0, 0},
		{0, 0, 0, 1, 0, 0, 0, 0},
		{0, 0, 1, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 1, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		}
	},
	{ PAL(pal_blue),
		.data = { // 3 moritz
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 0, 0},
		{0, 0, 0, 0, 0, 0, 1, 0},
		{0, 0, 0, 0, 0, 0, 1, 0},
		{0, 0, 0, 1, 1, 1, 1, 0},
		{0, 0, 0, 0, 0, 0, 1, 0},
		{0, 0, 0, 0, 0, 0, 1, 0},
		{0, 1, 1, 1, 1, 1, 0, 0},
		},
	},
	{ PAL(pal_orange),
		.data = { // 4 nele
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 1, 0, 1, 0, 0, 0},
		{0, 1, 0, 0, 1, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 0},
		{0, 0, 0, 0, 1, 0, 0, 0},
		{0, 0, 0, 0, 1, 0, 0, 0},
		{0, 0, 0, 0, 1, 0, 0, 0},
		},
	},
	{ PAL(pal_orange),
		.data = { // 5 nele
		{0, 0, 1, 1, 1, 1, 1, 1},
		{0, 0, 1, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 0, 0, 0, 0, 0, 1, 0},
		{0, 0, 0, 0, 0, 0, 1, 0},
		{0, 0, 0, 0, 0, 0, 1, 0},
		{0, 1, 1, 1, 1, 1, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		},
	},
	{ PAL(pal_orange),
		.data = { // 6 moritz
		{0, 0, 1, 1, 1, 1, 1, 0},
		{0, 0, 1, 0, 0, 0, 0, 0},
		{0, 0, 1, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 0, 1, 0, 0, 1, 0, 0},
		{0, 0, 1, 0, 0, 1, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		},
	},
	{ PAL(pal_green),
		.data = { // 7 moritz
		{0, 1, 1, 1, 1, 1, 1, 0},
		{0, 0, 0, 0, 0, 0, 1, 0},
		{0, 0, 0, 0, 0, 1, 0, 0},
		{0, 0, 1, 1, 1, 1, 1, 0},
		{0, 0, 0, 1, 0, 0, 0, 0},
		{0, 0, 1, 0, 0, 0, 0, 0},
		{0, 1, 0, 0, 0, 0, 0, 0},
		{1, 0, 0, 0, 0, 0, 0, 0},
		},
	},
	{ PAL(pal_orange),
		.data = { // 8 moritz
		{0, 1, 1, 1, 1, 0, 0, 0},
		{0, 1, 0, 0, 1, 0, 0, 0},
		{0, 1, 0, 0, 1, 0, 0, 0},
		{0, 1, 1, 1, 1, 0, 0, 0},
		{0, 1, 0, 0, 1, 0, 0, 0},
		{0, 1, 0, 0, 1, 0, 0, 0},
		{0, 1, 0, 0, 1, 0, 0, 0},
		{0, 1, 1, 1, 1, 0, 0, 0},
		},
	},
	{ PAL(pal_orange),
		.data = { // 9 moritz
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 0, 1, 0, 0, 1, 0, 0},
		{0, 0, 2, 0, 0, 1, 0, 0},
		{0, 0, 2, 1, 1, 1, 0, 0},
		{0, 0, 0, 0, 0, 2, 0, 0},
		{0, 0, 0, 0, 0, 2, 0, 0},
		{0, 0, 0, 0, 0, 2, 0, 0},
		{0, 0, 2, 2, 2, 2, 0, 0},
		},
	}
};

static void drawNum(uint8_t off_x, uint8_t off_y, uint8_t number) {
	if (number < (sizeof(num) / sizeof(num[0]))) {
		num_t *n = (num_t *) &num[number];
		for (int x = 0; x < 8; x++) {
			for (int y = 0; y < 8; y++) {
				color_t *c = (color_t*) &n->pal[n->data[x][y]];
				set_color_pixel(off_y + 7 - y, off_x +  x, c->r, c->g, c->b);
			}	
		}
	}
}
#endif

//display the time
void time_anim(void)
{
    time(&now);                       // read the current time
    localtime_r(&now, &tm);           // update the structure tm with the current time
	#ifdef DRAWN_NUM_CLOCK
		drawNum(0, 8, tm.tm_hour / 10);
		drawNum(0, 0, tm.tm_hour % 10);
		drawNum(8, 8, tm.tm_min / 10);
		drawNum(8, 0, tm.tm_min % 10);

		set_color_frame();
		b2d_wait(15000);
	#else
		char timestring[100];
		//convert the time to a string
		//sprintf(timestring, ">+:p42d50/#%02i#<;+p42d50/# %02i#x49y8b255p42d50#:", tm.tm_hour, tm.tm_min);
		sprintf(timestring, "</#    Es ist %02i:%02i    ", tm.tm_hour, tm.tm_min);

		//show the time
		scrolltext(timestring);
	#endif
}
