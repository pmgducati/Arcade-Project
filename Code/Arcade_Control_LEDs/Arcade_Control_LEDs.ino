#include <FastLED.h>
#include <EEPROM.h>

//Pin Assignments
#define P1_A 1      //Player 1 Button A
#define P1_B 2       //Player 1 Button B
#define P1_C 3       //Player 1 Button C
#define P1_X 4       //Player 1 Button X
#define P1_Y 5       //Player 1 Button Y 
#define P1_Z 6       //Player 1 Button Z
#define SW1  A6      //Toggle Switch 1y
#define SW2  A7      //Toggle Switch 2
#define DATA_PIN  22 //Neopixel Data Pin

//FastLED Parameters
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    6
#define FRAMES_PER_SECOND  120
int BRIGHTNESS = 96;

//Define Canvases and Arrays
CRGB Output_Array[NUM_LEDS]; //physical space
CRGB canvas1[16][16]; //virtual canvas
CRGB canvas2[16][16]; //virtual canvas
CRGB combined_canvas[16][16];  // this is optional

//Arrays
int Boot_X[] = {8, 9, 7, 4, 11, 3, 12, 2, 11, 2, 11, 3, 12, 3, 12, 4, 13, 4, 13, 2, 3, 4, 11, 12, 13, 2, 3, 4, 11, 12, 13, 3, 12, 4, 11, 9, 7, 8, 0, 7, 8, 9, 7, 8, 9, 7, 8, 9, 7, 8, 9, 7, 8, 9, 7, 8, 9, 7, 8, 9, 7, 8, 9, 7, 8, 9, 7, 8, 9, 0, 0};
int Boot_Y[] = {9, 9, 9, 9, 9, 9, 9, 7, 7, 6, 6, 7, 7, 6, 6, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 9, 9, 9, 9, 9, 9, 9, 0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0, 0};
int Boot_Step[] = {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 6, 6, 2, 2, 2, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 0};


//Variables

//Switch Variables
int SW1state = 0;             //Current State of Switch 1
int SW2state = 0;             //Current State of Switch 2
int SW1debounce = 0;          //SW1 Debouce trigger
int SW2debounce = 0;          //SW2 Debouce trigger
int RainbowTrigger = 0;       //Triggers Rainbow effect
int WashType = 0;             //Determines wether the wash is Rainbow or Confetti
int gHuereset = 0;            //resets Hue for Static layouts
int decayflag = 0;            //sets the decay function
unsigned long Cycle_Time;     //Time for Millis

//Boot Animation Variables
int Array_Pos = 0;
int Array_Pos_Start = 0;
int Boot_Step_Pos = 0;
int Boot_Start = 0;
int Fade_out_toggle = 0;
int Fade_out_rate = 1;
int Fade_out_tail = 1;
int Array_Pos_Delay = 0;
int Boot_Bright = 255;
int Boot_Hue = 0;
int Frame_Rate = 200;
unsigned long start_Time = 0;

//Attractmode placeholder variables
int AM_gHue = 0;
int AM_Brightness = 0;
int AM_Mode = 0;
int AM_RainbowTrigger = 0;
int AM_WashType = 0;
unsigned long AM_Timer;               //Time (Millis) of Button Press
unsigned long AM_Delay = 15000;           //Length of time to wait to start Attract Mode

//EEPROM Variables
unsigned long EEPROM_Timer;               //Time (Millis) of Last Marquee Change
unsigned long EEPROM_Write_Delay = 15000; //Delay before EEPROM Write to prevent to many writes to one address
int EEPROMCheck = 0;             //Current EEPROM Value
int EEPROM_Confirm_Flash = 250;           //Milliseconds for Flash on EEPROM Write Confirm
int Color_EEProm = 0;                     //EEPROM Address used to Store Last Color
int Brightness_EEProm = 1;                //EEPROM Address used to Store Last Brighness
int Mode_EEProm = 2;                      //EEPROM Address used to Store Last Mode
int Wash_EEProm = 3;                      //EEPROM Address used to Store Last Color Wash Type
int RB_EEProm = 3;                      //EEPROM Address used to Store Last Color Wash Toggle
int EEPROMstart = 0;

void setup() {
  //Serial Setup
  Serial.begin(9600);
  //FastLED Setup
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(Output_Array, NUM_LEDS).setCorrection(TypicalLEDStrip); //LED Setup
  FastLED.setBrightness(BRIGHTNESS); // set master brightness control
  //Input Setup
  pinMode(P1_A, INPUT);     //Player 1 Button A
  pinMode(P1_B, INPUT);     //Player 1 Button B
  pinMode(P1_C, INPUT);     //Player 1 Button C
  pinMode(P1_X, INPUT);     //Player 1 Button X
  pinMode(P1_Y, INPUT);     //Player 1 Button Y
  pinMode(P1_Z, INPUT);     //Player 1 Button Z
  pinMode(OPT1, INPUT);     //Option 1 (QUIT)
  pinMode(OPT2, INPUT);     //Option 2 (SAVE)
  pinMode(OPT3, INPUT);     //Option 3 (LOAD)
}

//Array Setup
typedef void (*SimplePatternList[])(); // List of patterns to cycle through.  Each is defined as a separate function below.
SimplePatternList gPatterns = { sinelon, doublesinelon, rainbow, juggle, confetti, soildstatic, layoutstatic, demo };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop() {
  if (Boot_Start == 0) {
    Boot_Animation();    //Boot Animation
    EEPROMread();    //Read EEPROM for last known values
    Boot_Start = 1;
  }

  if ((EEPROM_Timer + EEPROM_Write_Delay) < Cycle_Time && EEPROMCheck != 0) {
    EEPROMwrite();
  }
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  //Read All Inputs
  Cycle_Time = millis();
  SW1state = analogRead(SW1);
  SW2state = analogRead(SW2);

  //Switch 1 Functionality - Controls Current Pattern and Brightness of LEDs
  if (SW1state < 700) {
    SW1debounce = 0;
  }
  if (SW1state > 700 && SW1state < 800 && SW1debounce == 0) {
    nextPattern();
    SW1debounce = 1;
    EEPROMCheck = 1;
    EEPROM_Timer = Cycle_Time;
    gHuereset = 0;
  }
  if (SW1state > 900) {
    bright();
    EEPROMCheck = 1;
    EEPROM_Timer = Cycle_Time;
  }

  //Switch 2 Functionality - Controls hue and Color Wash of LEDs
  if (SW2state < 700) {
    SW2debounce = 0;
  }
  if (SW2state > 700 && SW2state < 800 && SW2debounce == 0) {
    gHue = gHue + 28;
    RainbowTrigger = 1;
    SW2debounce = 1;
    EEPROMCheck = 1;
    EEPROM_Timer = Cycle_Time;
  }
  if (SW2state > 900 && SW2debounce == 0) {
    RainbowTrigger = 0;
    if (WashType == 1) {
      WashType = 0;
    }
    else {
      WashType = 1;
    }
    SW2debounce = 1;
    EEPROMCheck = 1;
    EEPROM_Timer = Cycle_Time;
  }
  if (RainbowTrigger == 0) {
    if (WashType == 0) {
      EVERY_N_MILLISECONDS( 20 ) {
        gHue++;  // slowly cycle the "base color" through the rainbow
      }
    }
    else {
      EVERY_N_MILLISECONDS( 48 ) {
        gHue += 25;
      }
    }
  }

  //FastLED Execution
  FastLED.show();  // send the 'leds' array out to the actual LED strip
  FastLED.delay(1000 / FRAMES_PER_SECOND); // insert a delay to keep the framerate modest
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void EEPROMread() {
  gHue = EEPROM.read(Color_EEProm);
  BRIGHTNESS = EEPROM.read(Brightness_EEProm);
  gCurrentPatternNumber = EEPROM.read(Mode_EEProm);
  RainbowTrigger = EEPROM.read(RB_EEProm);
  WashType = EEPROM.read(Wash_EEProm);
}

void EEPROMwrite() {
  //Write Brightness Value to EEPROM
  if (gHue != EEPROM.read(Color_EEProm)) {
    EEPROM.write(Color_EEProm, gHue);
    Serial.println("Color Written");
  }
  if (BRIGHTNESS != EEPROM.read(Brightness_EEProm)) {
    EEPROM.write(Brightness_EEProm, BRIGHTNESS);
    Serial.println("Brightness Written");
  }
  if (gCurrentPatternNumber != EEPROM.read(Mode_EEProm)) {
    EEPROM.write(Mode_EEProm, gCurrentPatternNumber);
    Serial.println("Mode Written");
  }
  if (RainbowTrigger != EEPROM.read(RB_EEProm)) {
    EEPROM.write(RB_EEProm, RainbowTrigger);
    Serial.println("Rainbow Written");
  }
  if (WashType != EEPROM.read(Wash_EEProm)) {
    EEPROM.write(Wash_EEProm, WashType);
    Serial.println("Wash Type Written");
  }
  EEPROMCheck = 0;
  delay(2000);
}

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void demo()
{
  // add one to the current pattern number, and wrap around at the end
  EVERY_N_MILLISECONDS( 15000 ) {
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
  }
}

void bright()
{
  // add one to the current pattern number, and wrap around at the end
  BRIGHTNESS = (BRIGHTNESS - 1);
  if (BRIGHTNESS < 1) {
    BRIGHTNESS = 255;
  }
  FastLED.setBrightness(BRIGHTNESS);
}

//void attractmode(){
// AM_gHue = gHue;
// AM_Brightness = BRIGHTNESS;
// AM_Mode = gCurrentPatternNumber;
// AM_RainbowTrigger = RainbowTrigger;
// AM_WashType = WashType;
// Brightness = 200;
// RainbowTrigger = 0;
// WashType = 1;
// EVERY_N_SECONDS( 13 ) { nextPattern(); } // change patterns periodically
//}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( Output_Array, NUM_LEDS, gHue, 7);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( Output_Array, NUM_LEDS, 8);
  int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
  Output_Array[pos] += CHSV( gHue, 255, 192);
}

void doublesinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( Output_Array, NUM_LEDS, 20);
  int pos1 = beatsin16( 13, 0, NUM_LEDS - 1 );
  int pos2 = beatsin16( 13, 0, NUM_LEDS - 1 );
  pos2 = pos2 - (pos2 * 2) + NUM_LEDS - 1;
  Output_Array[pos1] += CHSV( gHue, 255, 192);
  Output_Array[pos2] += CHSV( gHue - 97, 255, 192);
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( Output_Array, NUM_LEDS, 20);
  for ( int i = 0; i < 8; i++) {
    Output_Array[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(gHue, 200, 255);
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( Output_Array, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  Output_Array[pos] += CHSV( gHue + random8(64), 200, 255);
}

void layoutstatic() {
  if (gHuereset == 0) {
    gHue = 0;
    gHuereset = 1;
    RainbowTrigger = 1;
  }
  canvas1[13][6] = CHSV(gHue + 128, 255, 255);
  canvas1[12][6] = CHSV(gHue + 112, 255, 255);
  canvas1[11][6] = CHSV(gHue + 96, 255, 255);
  canvas1[11][7] = CHSV(gHue + 96, 255, 255);
  canvas1[12][7] = CHSV(gHue + 112, 255, 255);
  canvas1[13][7] = CHSV(gHue + 128, 255, 255);
  canvas1[12][9] = CHSV(gHue + 64, 255, 255);
  canvas1[11][9] = CRGB(gHue + 255, 255, 255);
  canvas1[9][9] = CHSV(gHue + 160, 255, 255);
  canvas1[8][9] = CHSV(gHue + 160, 255, 255);
  canvas1[7][9] = CHSV(gHue + 160, 255, 255);
  canvas1[4][9] = CHSV(gHue + 64, 255, 255);
  canvas1[3][9] = CRGB(gHue + 255, 255, 255);
  canvas1[2][7] = CHSV(gHue + 0, 255, 255);
  canvas1[3][7] = CHSV(gHue + 16, 255, 255);
  canvas1[4][7] = CHSV(gHue + 32, 255, 255);
  canvas1[4][6] = CHSV(gHue + 32, 255, 255);
  canvas1[3][6] = CHSV(gHue + 16, 255, 255);
  canvas1[2][6] = CHSV(gHue + 0, 255, 255);
  canvasOutput(5);
}

void soildstatic()
{
  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 16; y++) {
      canvas1[x][y] = CHSV(gHue, 255, 255);
    }
  }
  canvasOutput(5);
}


void canvasOutput(int blend_rate) {
  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 16; y++) {
      combined_canvas[x][y] = canvas1[x][y] + canvas2[x][y];
    }
  }

  blend_copy(&Output_Array[0], &combined_canvas[13][6], blend_rate);
  blend_copy(&Output_Array[1], &combined_canvas[12][6], blend_rate);
  blend_copy(&Output_Array[2], &combined_canvas[11][6], blend_rate);
  blend_copy(&Output_Array[3], &combined_canvas[11][7], blend_rate);
  blend_copy(&Output_Array[4], &combined_canvas[12][7], blend_rate);
  blend_copy(&Output_Array[5], &combined_canvas[13][7], blend_rate);
  blend_copy(&Output_Array[6], &combined_canvas[12][9], blend_rate);
  blend_copy(&Output_Array[7], &combined_canvas[11][9], blend_rate);
  blend_copy(&Output_Array[8], &combined_canvas[9][9], blend_rate);
  blend_copy(&Output_Array[9], &combined_canvas[8][9], blend_rate);
  blend_copy(&Output_Array[10], &combined_canvas[7][9], blend_rate);
  blend_copy(&Output_Array[11], &combined_canvas[4][9], blend_rate);
  blend_copy(&Output_Array[12], &combined_canvas[3][9], blend_rate);
  blend_copy(&Output_Array[13], &combined_canvas[2][7], blend_rate);
  blend_copy(&Output_Array[14], &combined_canvas[3][7], blend_rate);
  blend_copy(&Output_Array[15], &combined_canvas[4][7], blend_rate);
  blend_copy(&Output_Array[16], &combined_canvas[4][6], blend_rate);
  blend_copy(&Output_Array[17], &combined_canvas[3][6], blend_rate);
  blend_copy(&Output_Array[18], &combined_canvas[2][6], blend_rate);
}

void blend_copy(CRGB * destination , CRGB * source_crgb , uint8_t blur_rate ) {
  if (destination->r < source_crgb->r) destination->r = min(qadd8(destination->r, blur_rate), source_crgb->r);
  else if (destination->r > source_crgb->r) destination->r = max(qsub8(destination->r, blur_rate), source_crgb->r);
  if (destination->g < source_crgb->g) destination->g = min(qadd8(destination->g, blur_rate), source_crgb->g);
  else if (destination->g > source_crgb->g) destination->g = max(qsub8(destination->g, blur_rate), source_crgb->g);
  if (destination->b < source_crgb->b) destination->b = min(qadd8(destination->b, blur_rate), source_crgb->b);
  else if (destination->b > source_crgb->b) destination->b = max(qsub8(destination->b, blur_rate), source_crgb->b);
}

void Boot_Animation() {
  int blend_setting;
  for (int z = 0; z < 12; z++) {
    if (z == 2 || z == 4 || z == 6 || z == 8 || z == 10 || z == 12 || z == 14 || z == 16 || z == 18 || z == 20) {
      Frame_Rate = 1000;
      Boot_Bright = 255;
      Fade_out_toggle = 1;
      blend_setting = 2;
      Fade_out_rate = 1;
    }
    else {
      Frame_Rate = 1000;
      Boot_Bright = 0;
      Fade_out_toggle = 1;
      blend_setting = 2;
      Fade_out_rate = 1;
    }
    uint32_t framestarttime = millis();
    while (millis() - framestarttime < Frame_Rate) {
      canvas1[8][9] = CHSV(0, 0, Boot_Bright);

      if (Fade_out_toggle == 1) {
        static uint32_t decay2_timer = 0;
        if (millis() - decay2_timer > 1) {
          decay2_timer = millis();
          fadeToBlackBy( (CRGB*) canvas1, 16 * 16, Fade_out_rate);
        }
      }
      static uint32_t render_timer = 0;
      if (millis() - render_timer > 10) {
        canvasOutput(blend_setting);
        FastLED.show();  // send the 'leds' array out to the actual LED strip
        render_timer = millis();
      }
    }
  }
  Boot_Step_Pos = 0;
  Array_Pos_Start = 0;
  Array_Pos = 0;
  for (int z = 0; z < 29; z++) {
    if (z < 9) {
      Frame_Rate = 400;
      Boot_Hue = 144;
      Boot_Bright = 255;
      Fade_out_toggle = 1;
      blend_setting = 1;
      Fade_out_rate = 1;
      Fade_out_tail = 5;
    }
    if (z == 9) {
      Frame_Rate = 2000;
      Boot_Hue = 144;
      Boot_Bright = 255;
      Fade_out_toggle = 1;
      blend_setting = 1;
      Fade_out_rate = 1;
      Fade_out_tail = 5;
    }
    if (z == 10) {
      Frame_Rate = 600;
      Boot_Hue = 32;
      Boot_Bright = 255;
      Fade_out_toggle = 0;
      blend_setting = 2;
      Fade_out_rate = 1;
      Fade_out_tail = 1;
    }
    if (z > 11  && z < 16) {
      Frame_Rate = 400;
      Boot_Hue = 32;
      Boot_Bright = 255;
      Fade_out_toggle = 0;
      blend_setting = 10;
      Fade_out_rate = 1;
      Fade_out_tail = 1;
    }
    if (z == 16) {
      Frame_Rate = 800;
      Boot_Hue = 0;
      Boot_Bright = 0;
      Fade_out_toggle = 1;
      blend_setting = 10;
      Fade_out_rate = 1;
      Fade_out_tail = 1;
      for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
          canvas1[x][y] = CRGB(0, 0, 0);
        }
      }
    }
    if (z == 17 || z == 19 || z == 21 || z == 23 || z == 25) {
      Frame_Rate = 200;
      Boot_Hue = 160;
      Boot_Bright = 255;
      Fade_out_toggle = 1;
      blend_setting = 20;
      Fade_out_rate = 1;
      Fade_out_tail = 1;
    }
    if (z == 18 || z == 20 || z == 22 || z == 24 || z == 26) {
      Frame_Rate = 1200;
      Boot_Hue = 0;
      Boot_Bright = 0;
      Fade_out_toggle = 1;
      blend_setting = 5;
      Fade_out_rate = 1;
      Fade_out_tail = 1;
    }
    if (z == 27) {
      Frame_Rate = 2000;
      Boot_Hue = 0;
      Boot_Bright = 255;
      Fade_out_toggle = 0;
      blend_setting = 10;
      Fade_out_rate = 1;
      Fade_out_tail = 1;
      for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
          canvas1[x][y] = CHSV(Boot_Hue, 255, 255);
        }
      }
    }
    if (z == 28) {
      Frame_Rate = 2000;
      Boot_Hue = 0;
      Boot_Bright = 0;
      Fade_out_toggle = 1;
      blend_setting = 10;
      Fade_out_rate = 1;
      Fade_out_tail = 1;
      for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
          canvas1[x][y] = CHSV(96, 255, 0);
        }
      }
    }
    uint32_t framestarttime = millis();
    while (millis() - framestarttime < Frame_Rate) {
      Array_Pos = Array_Pos_Start;
      for (int x = 0; x < Boot_Step[Boot_Step_Pos]; x++) {
        canvas1[Boot_X[Array_Pos]][Boot_Y[Array_Pos]] = CHSV(Boot_Hue, 255, Boot_Bright);
        Array_Pos++;
      }

      if (Fade_out_toggle == 1) {
        static uint32_t decay2_timer = 0;
        if (millis() - decay2_timer > Fade_out_tail) {
          decay2_timer = millis();
          fadeToBlackBy( (CRGB*) canvas1, 16 * 16, Fade_out_rate);
        }
      }
      static uint32_t render_timer = 0;
      if (millis() - render_timer > 10) {
        canvasOutput(blend_setting);
        FastLED.show();  // send the 'leds' array out to the actual LED strip
        render_timer = millis();
      }
    }
    Array_Pos_Start += Boot_Step[Boot_Step_Pos];
    Boot_Step_Pos++;
  }
}
