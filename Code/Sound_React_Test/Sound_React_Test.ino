#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <FastLED.h>

CHSV color1 = CHSV(32, 255, 255);
CHSV color2 = CHSV(0, 0, 0);

AudioInputAnalog         adc1(A9);  //A9 is on ADC0
AudioAnalyzeFFT256       fft256_1;
AudioConnection          patchCord1(adc1, fft256_1);

int FFTdisplayValueMax16[16]; //max vals for normalization over time
uint8_t FFTdisplayValue16[16]; //max vals for normalization over time
uint8_t FFTdisplayValue8[8]; //max vals for normalization over time
uint8_t FFTdisplayValue5[5]; //max vals for normalization over time
uint8_t FFTdisplayValue1;

const uint8_t kMatrixWidth = 16;
const uint8_t kMatrixHeight = 5;

// Pixel layout
//
//      0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
//   +------------------------------------------------
// 0 |  .  . 12 11  .  . 10  9  8  .  .  7  6  .  .  .
// 1 |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .
// 2 |  .  . 13 14 15  .  .  .  .  .  .  3  4  5  .  .
// 3 |  .  . 18 17 16  .  .  .  .  .  .  2  1  0  .  .
// 4 |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds[ NUM_LEDS ];

// This function will return the right 'led index number' for
// a given set of X and Y coordinates on your RGB Shades.
// This code, plus the supporting 80-byte table is much smaller
// and much faster than trying to calculate the pixel ID with code.
#define LAST_VISIBLE_LED 67
uint8_t XY( uint8_t x, uint8_t y)
{
  // any out of bounds address maps to the first hidden pixel
  if ( (x >= kMatrixWidth) || (y >= kMatrixHeight) ) {
    return (LAST_VISIBLE_LED + 1);
  }

  const uint8_t ShadesTable[] = {
    68, 46, 47, 12, 11, 40, 41, 10,  9,  8, 39,  7,  6, 48, 32, 69,
    29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 56, 55, 54, 33, 34,
    30, 31, 13, 14, 15, 35, 36, 70, 71, 37, 38,  3,  4,  5, 42, 43,
    57, 53, 18, 17, 16, 52, 51, 72, 73, 50, 49,  2,  1,  0, 45, 44,
    74, 58, 59, 60, 61, 62, 75, 76, 77, 78, 63, 64, 65, 66, 67, 79
  };

  uint8_t i = (y * kMatrixWidth) + x;
  uint8_t j = ShadesTable[i];
  return j;
}

void setup() {
  Serial.begin(115200);

  //audio library setup
  AudioMemory(3);
  fft256_1.windowFunction(AudioWindowHanning256);
  fft256_1.averageTogether(4);
  FastLED.addLeds<WS2811, 22, GRB>(leds, LAST_VISIBLE_LED + 1);
  FastLED.setBrightness(48);

}


CHSV map_hsv(uint8_t input, uint8_t in_min, uint8_t in_max, CHSV* out_starting, CHSV* out_ending) {


  if (input <= in_min) return CHSV(*out_starting);
  if (input >= in_max) return CHSV(*out_ending);

  //calculate shortest path between colors
  int16_t shortest_path = out_ending->h; //no rollover
  if ((((int16_t)out_ending->h) + 256) - ((int16_t)out_starting->h) <= 127) {
    shortest_path += 256;  //rollover
  }
  else if ((int16_t)(out_starting->h) - (((int16_t)out_ending->h) - 255) <= 127) {
    shortest_path -= 256; //rollunder
  }


  return CHSV(
           ((input - in_min) * (shortest_path - out_starting->h + 1) / (in_max - in_min + 1) + out_starting->h), \
           (input - in_min) * (out_ending->s - out_starting->s + 1) / (in_max - in_min + 1) + out_starting->s, \
           (input - in_min) * (out_ending->v - out_starting->v + 1) / (in_max - in_min + 1) + out_starting->v);
}
void calcfftcolor(CHSV * temp_color, uint8_t input) {

  //make the tip of the color be color 2
  *temp_color = (input > 240) ? map_hsv(input, 240, 255, &color2, &color2) : color2;

  //ignore brightness, max it.
  temp_color->v = input;

  return;
}

void loop() {

  if (fft256_1.available()) {

    for (uint8_t i = 0; i < 16; i++) {
      int16_t n = 1000 * fft256_1.read((i * 2), (i * 2) + 2);

      //de-emphasize lower frequencies (for in clubs)
      switch (i) {
        case 0:  n = max(n - 3, 0); break;
        case 1:  n = max(n - 4, 0);  break;
        case 2:  n = max(n - 7, 0);  break;
        case 3:  n = max(n - 15, 0);  break;
        default: n = max(n - 30, 0);   break;
      }

      //falloff controll
      FFTdisplayValueMax16[i] = max(max(FFTdisplayValueMax16[i] * .98, n), 4);
      FFTdisplayValue16[i] = constrain(map(n, 0, FFTdisplayValueMax16[i], 0, 255), 0, 255);

      // downsample 16 samples to 8
      if (i & 0x01) {
        FFTdisplayValue8[i >> 1] = (FFTdisplayValue16[i] + FFTdisplayValue16[i - 1]) >> 1;
      }
    }

    // downsample 8 samples to 1
    FFTdisplayValue1 = 0;
    for (uint8_t i = 0; i < 4; i++) {
      //Serial.print(FFTdisplayValue8[i]);
      //Serial.print( ' ');
      FFTdisplayValue1 += FFTdisplayValue8[i] / 4;
    }

    //downsample 8 to 5
    FFTdisplayValue5[0] = FFTdisplayValue8[1] ;
    FFTdisplayValue5[1] = FFTdisplayValue8[2];
    FFTdisplayValue5[2] = FFTdisplayValue8[3] ;
    FFTdisplayValue5[3] = FFTdisplayValue8[4] ;
    FFTdisplayValue5[4] = FFTdisplayValue8[6] ;

    if (global_mode == 0) {

      for (uint8_t y = 5; y > 0; y--) {
        for (uint8_t x = 0; x < 16; x++) {
          leds[XY(x, y)] = leds[XY(x, y - 1)];
        }
      }

      for (uint8_t i = 0; i < 16; i++) {
        //make the tip of the color be color 2
        CHSV temp_color;
        calcfftcolor(&temp_color, FFTdisplayValue16[i]);
        leds[XY(i, 0)] = temp_color;
      }

    }
    for (uint8_t y = 0; y < 5; y++) {
      for (uint8_t x = 0; x < 16; x++) {
        leds[XY(x, y)] = leds[XY(x, y - 1)];
      }
    }

    for (uint8_t i = 0; i < 16; i++) {
      //make the tip of the color be color 2
      CHSV temp_color;
      calcfftcolor(&temp_color, FFTdisplayValue16[i]);
      leds[XY(i, 0)] = temp_color;
    }
  }
  FastLED.show();
}
