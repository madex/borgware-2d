#include "config.h"
extern "C" {
    #include "pixel.h"
    #include "animations/blackhole.h"
    #include "animations/blackhole.c"
}

void setup() {
  pixel_init();
}

void loop() {
  blackhole();
}
