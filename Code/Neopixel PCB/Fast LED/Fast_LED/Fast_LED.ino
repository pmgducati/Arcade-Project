#include <FastLED.h>

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

//Pin Assignments
#define P1_A 19      //Player 1 Button A
#define P1_B 1       //Player 1 Button B
#define P1_C 2       //Player 1 Button C
#define P1_X 3       //Player 1 Button X
#define P1_Y 4       //Player 1 Button Y 
#define P1_Z 5       //Player 1 Button Z
#define P1_ST 6      //Player 1 Button START
#define P1_SL 7      //Player 1 Button SELECT
#define P2_A  10     //Player 2 Button A
#define P2_B  11     //Player 2 Button B
#define P2_C  12     //Player 2 Button C
#define P2_X  13     //Player 2 Button X 
#define P2_Y  14     //Player 2 Button Y
#define P2_Z  15     //Player 2 Button Z
#define P2_ST  16    //Player 2 Button START
#define P2_SL  17    //Player 2 Button SELECT
#define OPT1  8      //Option 1 (QUIT)
#define OPT2  9      //Option 2 (SAVE)
#define OPT3  18     //Option 3 (LOAD)
#define DATA_PIN  22 //Neopixel Data Pin

//FastLED Parameters
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    19
CRGB leds[NUM_LEDS];
#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

//Arrays
//LED Order 
int LED_Order[] = {19, 1, 2, 5, 4, 3, 6, 7, 8, 9, 18, 16, 17, 13, 14, 15, 12, 11, 10};
//Player 1 Buttons
int Player_1[] = {19, 1, 2, 5, 4, 3, 6, 7};
//Player 2 Buttons
int Player_2[] = {16, 17, 13, 14, 15, 12, 11, 10};
//Option (Middle) Buttons
int Opt_Buttons[] = {8, 9, 18};
//P1 Start/Select, Option, and P2 Start/Select Buttons
int Top_Row [] = {6, 7, 8, 9, 18, 16, 17};
//Player 1 ACBXYZ Buttons
int P1_3X2[] = {19, 1, 2, 5, 4, 3};
//Player 2 ACBXYZ Buttons
int P2_3X2[] = {13, 14, 15, 12, 11, 10};
//Player 1 & 2 Start/Select Buttons
int STSL[] = {6, 7, 16, 17};

//Variables
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void setup() {
  delay(3000); // 3 second delay for recovery
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}

void loop()
{
  // Call the current pattern function once, updating the 'leds' array

  sinelon();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  Player_1[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
