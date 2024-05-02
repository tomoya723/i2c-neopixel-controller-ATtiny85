#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

////////////////////////////////////////////////////////////////////////////////
/// Configuration
////////////////////////////////////////////////////////////////////////////////

// i2c addressing is a free-for-all, so feel free to change this if you have
// another device that conflicts
#define I2C_ADDRESS 0x06

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        3 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 8 // Popular NeoPixel ring size

////////////////////////////////////////////////////////////////////////////////
/// Defaults
////////////////////////////////////////////////////////////////////////////////

// sets the maximum brightness of the pixels; 40 is a reasonable default for
// direct viewing without searing your retinas
byte brightness = 40;

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// we keep the pixel state in memory so it can be manipulated very quickly in
// the command handlers. The updatePixels() function iterates over this and
// sends commands to the LEDs
byte pixelState[NUMPIXELS][3];

#define DELAYVAL 500 // Time (in milliseconds) to pause between pixels

void setup() {
  Wire.begin(I2C_ADDRESS);                // join i2c bus with address #0x06
  Wire.onReceive(receiveEvent); // register event
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(brightness);
  pixels.clear(); // Set all pixel colors to 'off'
  pixels.show();
  blackout();
}

void loop() {
//   pixels.clear(); // Set all pixel colors to 'off'

  // The first NeoPixel in a strand is #0, second is 1, all the way up
  // to the count of pixels minus one.
//  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...

    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    // Here we're using a moderately bright green color:
//    pixels.setPixelColor(i, pixels.Color(0, 150, 0));

//    pixels.show();   // Send the updated pixel colors to the hardware.

//    delay(DELAYVAL); // Pause before next pass through loop
//    }
}

// iterates over the state array and sets everything to zero
void blackout() {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixelState[i][0] = 0;
    pixelState[i][1] = 0;
    pixelState[i][2] = 0;
  }
}
// dumps the i2c buffer as hex bytes
void dumpMessage() {
  Serial.print("[ ");

  while (Wire.available()) {
    char buf[4];
    sprintf(buf, "0x%02X ", Wire.read());
    Serial.print(buf);
  }

  Serial.println("]");
}

// fetches the color of a pixel from the state array and returns it as a
// uint32_t that can be passed directly to pixels.setPixelColor()
uint32_t pixelColor(int pixel) {
  byte r = pixelState[pixel][0];
  byte g = pixelState[pixel][1];
  byte b = pixelState[pixel][2];

  return pixels.Color(r, g, b);
}

// copies the pixelState array to the actual LEDs. Also sets brightness.
void updatePixels() {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixelColor(i));
  }

  pixels.setBrightness(brightness);
  pixels.show();
}

////////////////////////////////////////////////////////////////////////////////
/// Command Handler
////////////////////////////////////////////////////////////////////////////////

// this callback is fired whenever we receive an i2c message
//
// the command "parser" is deliberately very simple... it works well enough for
// me for now but I'm happy to make it better if it doesn't work for you. File
// an issue or PR!
//
// Commands:
//
// * 0x00 - turn off all the LEDs
// * 0x01 - set the maximum brightness. Takes one arg.
// * 0x02 - set a specific LED to the given color. takes four args: addr, red,
//          green, blue
// * 0x03 - enable color correction
// * 0x04 - disable color correction
void receiveEvent(int numBytes) {
  int command = Wire.read();

  switch (command) {
  case 0x00:
    Serial.println("blackout");
    blackout();
    break;

  case 0x01:
    brightness = Wire.read();
    break;

  case 0x02:
    // do a little bit of sanity checking... if we got a weird number of bytes
    // it's better to just bail than to guess what the user meant
    if (numBytes != 5) {

      if(numBytes - 1) {
        dumpMessage();
      } else {
        // dumpMessage() does a println so we only need it if we didn't end up
        // calling dumpMessage()
      }

    } else {
      int led = Wire.read();
      int r   = Wire.read();
      int g   = Wire.read();
      int b   = Wire.read();

      pixelState[led][0] = r;
      pixelState[led][1] = g;
      pixelState[led][2] = b;
    }
    break;

  default:

    if(Wire.available()) {
      dumpMessage();
    } else {
      // as above... dumpMessage() does a println so we only need it if we
      // didn't end up calling dumpMessage()
    }
  }

  updatePixels();

  // dump the rest of the buffer
  Wire.flush();
}