/**
 * Author: Michael Pate
 * Version: 2.0
 * Date: 8-21-2025
 */

#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>

// This was included in the Adafruit example sketch
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define LED_DATA 3
#define USR_BTN 4

// 4x LEDs, 800kHz GRB bitstream
Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, LED_DATA, NEO_GRB + NEO_KHZ800);

// EEPROM addresses to hold data for brightness and the current mode
#define MODE_ADDR 100
#define BRIGHT_ADDR 150 // stored as an index in brightOptions (one-indexed)
byte brightOptions[6] = {0, 25, 50, 100, 150, 200};
// These values will be saved every time they are changed, and loaded on startup
byte mode, brightness_i;
#define DEFAULT_MODE 0
#define DEFAULT_BRIGHT_I 3 // brightness of 50 for index 2

bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long pressStartTime = 0;
bool longPressHandled = false;
#define HOLD_TIME 600

double rainbowColorPosition = 0.0;
unsigned long lastSnapMillis = 0; // last time the color snapped in mode 2

void setup() {
  pinMode(USR_BTN, INPUT_PULLUP);
  pinMode(LED_DATA, OUTPUT);

  // Check if the EEPROM has been written, as in nonzero
  byte testMode = EEPROM.read(MODE_ADDR);
  if (testMode != 255)
  {
    mode = testMode;
  }
  else
  {
    mode = DEFAULT_MODE;
  }

  byte testBright_i = EEPROM.read(BRIGHT_ADDR);
  if (testBright_i != 255)
  {
    brightness_i = testBright_i;
  }
  else
  {
    brightness_i = DEFAULT_BRIGHT_I;
  }

  strip.begin();
  strip.setBrightness(brightOptions[brightness_i]);
  strip.show(); // Initialize all pixels to 'off'

  // for testing
  for (int i=0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, strip.Color(255,0,0));
    strip.show();
  }
}

void fillRed(int bright)
{
  strip.setBrightness(bright);
  for (int i=0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, strip.Color(255,0,0));
    strip.show();
  }
}

void loop() {
  // Handle button presses, holds, and such
  buttonState = digitalRead(USR_BTN);

  // Detect right when the button is pressed
  if (buttonState == LOW && lastButtonState == HIGH)
  {
    pressStartTime = millis();
    longPressHandled = false;
  }

  // Check if the button is still being held
  if (buttonState == LOW && !longPressHandled)
  {
    if (millis() - pressStartTime >= HOLD_TIME)
    {
      // long press action - change brightness and save to EEPROM
      if (++brightness_i > 5)
      {
        brightness_i = 0;
      }
      EEPROM.write(BRIGHT_ADDR, brightness_i);
      strip.setBrightness(brightOptions[brightness_i]);
      
      longPressHandled = true;
    }
  }

  // Button just released
  if (buttonState == HIGH && lastButtonState == LOW)
  {
    if (!longPressHandled)
    {
      // short press action - change mode and save to EEPROM
      if (++mode > 10)
      {
        mode = 0;
      }
      rainbowColorPosition = 0.0;
      EEPROM.write(MODE_ADDR, mode);
    }
  }
  lastButtonState = buttonState;

  uint32_t colorTarget;
  switch(mode)
  {
    case 0:
    {
      // rainbow mode, but kind of like a wheel
      if (rainbowColorPosition > 1024.0) rainbowColorPosition = 0.0;
      else rainbowColorPosition += 0.19;
      for(int i=0; i<strip.numPixels(); i++)
      {
        strip.setPixelColor(i, colorFrom1024(int(rainbowColorPosition + i * 200), strip));
      }
      strip.show();
      delay(1);
      break;
    }
    case 1:
    {
      // rainbow mode, slowly cycle through the colors
      if (rainbowColorPosition > 1024.0) rainbowColorPosition = 0.0;
      else rainbowColorPosition += 0.1;
      for(int i=0; i<strip.numPixels(); i++) 
      {
        strip.setPixelColor(i, colorFrom1024(int(rainbowColorPosition), strip));
      }
      strip.show();
      delay(1);
      break;
    }
    case 2:
    {
      // rainbow mode, slowly cycle through the colors a little faster
      if (rainbowColorPosition > 1024.0) rainbowColorPosition = 0.0;
      else rainbowColorPosition += 0.33;
      for(int i=0; i<strip.numPixels(); i++) 
      {
        strip.setPixelColor(i, colorFrom1024(int(rainbowColorPosition), strip));
      }
      strip.show();
      delay(1);
      break;
    }
    case 3:
    {
      // Still a rainbow but just snapping through the 6 primary colors
      if (rainbowColorPosition > 853.0) rainbowColorPosition = 0.0; // 853 because thats 1024-170, so red doesnt show twice
      unsigned long now_millis = millis();
      if (now_millis - lastSnapMillis > 2000)
      {
        rainbowColorPosition += 170;  // 1024 / 6 = 170
        lastSnapMillis = now_millis;
      }
      for(int i=0; i<strip.numPixels(); i++)
      {
        strip.setPixelColor(i, colorFrom1024(int(rainbowColorPosition), strip));
      }
      strip.show();
      delay(1);
      break;
    }
    case 4:
    {
      // Show white static
      strip.fill(strip.Color(255,255,255));
      strip.show();
      delay(1);
      break;
    }
    case 5:
    {
      // Show red static
      strip.fill(strip.Color(255,0,0));
      strip.show();
      delay(1);
      break;
    }
    case 6:
    {
      // Show green static
      strip.fill(strip.Color(0,255,0));
      strip.show();
      delay(1);
      break;
    }
    case 7:
    {
      // Show blue static
      strip.fill(strip.Color(0,0,255));
      strip.show();
      delay(1);
      break;
    }
    case 8:
    {
      // Show yellow static
      strip.fill(strip.Color(255,255,0));
      strip.show();
      delay(1);
      break;
    }
    case 9:
    {
      // Show cyan static
      strip.fill(strip.Color(0,255,255));
      strip.show();
      delay(1);
      break;
    }
    case 10:
    {
      // Show purple static
      strip.fill(strip.Color(255,0,255));
      strip.show();
      delay(1);
      break;
    }
    default:
    {
      // Default to just showing white
      strip.fill(strip.Color(255,255,255));
      strip.show();
      delay(1);
      break;
    }
  }
}

// Map a 0–1023 input into a NeoPixel color
uint32_t colorFrom1024(uint16_t pos, Adafruit_NeoPixel &strip) {
  // constrain to 0–1023
  pos = pos & 0x3FF; // wrap if needed

  // Scale 0–1023 into 0–767 (3 * 256)
  uint16_t scaled = (pos * 768L) / 1024;

  uint8_t r, g, b;

  if (scaled < 256) {
    // Red -> Green
    r = 255 - scaled;
    g = scaled;
    b = 0;
  } else if (scaled < 512) {
    // Green -> Blue
    scaled -= 256;
    r = 0;
    g = 255 - scaled;
    b = scaled;
  } else {
    // Blue -> Red
    scaled -= 512;
    r = scaled;
    g = 0;
    b = 255 - scaled;
  }

  // Use Adafruit's helper
  return strip.Color(r, g, b);
}
