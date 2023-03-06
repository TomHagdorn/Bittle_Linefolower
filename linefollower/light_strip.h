#include <Adafruit_NeoPixel.h>

#define LED_PIN    15
#define LED_COUNT  5

unsigned long last_colorchange = 0;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
void strip_setup() {
  strip.begin();
  
  for (int i = 0; i < 5; i++) {
    strip.setPixelColor(i, 255, 255, 255); // set each LED to white
  }
  
  strip.show(); // display the updated colors
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}




void cycle_led_strip() {
  // Cycle through colors
  for(int c = 0; c < 256; c++) {
    for(int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i + c) & 255));
    }
    if (millis() - last_colorchange >= 10000UL)
    {
      strip.show();
      last_colorchange = millis();
    }
  }
}
#
