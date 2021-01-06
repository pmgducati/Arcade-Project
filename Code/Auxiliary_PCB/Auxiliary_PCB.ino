//Libraries
#include <DHT.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_LiquidCrystal.h>

//Pin Assignments
#define Temp_Upper 3  //Temp Sensor to Monitor Marquee LED Tempature
#define Temp_Lower 2  //Temp Sensor to Monitor All other Equipment
#define Fan_Upper 5   //Fans in Upper Section of Cabinet
#define Fan_Lower 6   //Fan in Lower Section of Cabinet 
#define Marquee 4     //Marquee LEDs 
#define Sw_Marquee 0  //Switch to Control Marquee Brightness
#define Sw_Fan 1      //Switch to Toggle the Fans On/Off
#define Sw_Future1 7  //Switch B - Possible Future Functionality Expansion
#define Sw_Future2 8  //Switch B - Possible Future Functionality Expansion
#define Pwr_LED 13    //Power LED

// LCD i2c Address
Adafruit_LiquidCrystal lcd(0);

//Temp Sensor Definition
#define DHTTYPE DHT22   // DHT 22  AM2302

//Arrays
//Fluorescent Flicker Array for Marquee 
int Flicker[] = {10, 20, 20, 240, 20, 40, 20, 100, 20, 20, 20, 260, 80, 20, 240, 60, 160, 20, 240, 20, 1000, 20, 20, 40, 100, 20, 2740, 340, 860, 20, 1400, 20, 60, 20};
// Brightness Scale for Marquee Flicker
int Flicker_High[] = {58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 115, 115, 115, 115, 172, 172, 230, 230, 230, 230, 230, 230, 230, 230, 230, 230, 230, 230, 230, 230, 230, 230, 230, 230};
// Brightness Low Scale for Marquee Flicker
int Flicker_Low[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 0, 0, 0, 0, 0, 0};
// Brightness Scale for Marquee Switch
int MQ_Bright[] = {230, 200, 172, 145, 115, 80, 58, 25, 0};

//Variable Assignments
unsigned long time;                 //Time for Millis
//Marquee Variables
int Marquee_Flicker_Status = 1;     //Trigger Flicker at Boot, Set to 0 to Disable
int Marquee_Sw_Status;              //Current Status of Marquee Switch
int Marquee_Sw_Last_Status;         //Last Status of Marquee Switch
int Marquee_Brightness = 0;         //Counter for Marquee Brightness Steps
int Marquee_EEProm = 0;             //EEPROM Address used to Store Last Marquee Brighness
//Temp Sensor Variables
int Temp_Last_Read = 0;             //Time (Millis) of last Temp Read
int Temp_Trigger = 80;              //Tempature to Trigger Fans On/Off
int Over_Temp_Trigger  = 85;        //Tempature to Trigger Warning
int Temp_Delay_Read = 2000;         //Delay Between Temp Reads
float Temp_Read_Upper;              //Upper Temp Sensor Reading
float Temp_Read_Lower;              //Lower Temp Sensor Reading
//Fan Switch Variables
int Fan_Sw_Toggle_Last_Status = 1;  //Last Status of Fan Switch
int Fan_Sw_Toggle_Last_State = 1;   //State to Disable Temp Sensors from Turning off Fans
int Fan_Sw_Toggle_Status;           //Current Status of Fan Switch
//EEPROM Variables
int EEPROM_Timer;                   //Time (Millis) of Last Marquee Change     
int EEPROM_Write_Delay = 15000;     //Delay before EEPROM Write to prevent to many writes to one address
int EEPROM_Current_Value = 0;       //Current EEPROM Value
int EEPROM_Confirm_Flash = 250;     //Milliseconds for Flash on EEPROM Write Confirm

//Temp Assignments
DHT TempUpper(Temp_Upper, DHTTYPE);
DHT TempLower(Temp_Lower, DHTTYPE);

void setup() {
  Serial.begin(9600);

 //Initialize GPIO
  pinMode(Fan_Upper, OUTPUT);
  pinMode(Fan_Lower, OUTPUT);
  pinMode(Marquee, OUTPUT);
  pinMode(Sw_Marquee, INPUT_PULLUP);
  pinMode(Sw_Fan, INPUT_PULLUP);
  pinMode(Sw_Future1, INPUT_PULLUP);
  pinMode(Sw_Future2, INPUT_PULLUP);
  pinMode(Pwr_LED, OUTPUT);
  digitalWrite(Pwr_LED, HIGH);

//Initialize Temp Assignments
  TempUpper.begin();
  TempLower.begin();

// Initialize the LCD 
  lcd.begin(16, 2);
  lcd.setBacklight(HIGH);
  lcd.print("    BOOTING");
}

void loop() {
//Marquee Fluorescent Flicker Emulation
  if (Marquee_Flicker_Status == 1){                                
    for(int i=0; i<33; i++) {
        lcd.setCursor(0, 0);
        lcd.print("Initialize  ");
        lcd.setCursor(0, 1);
        lcd.print("Marquee Flicker");
        analogWrite(Marquee, Flicker_High[i]);
        delay(Flicker[i]);
        ++i;
        analogWrite(Marquee, Flicker_Low[i]);
        delay(Flicker[i]);
        analogWrite(Marquee, Flicker_High[i]);        
        Marquee_Flicker_Status = 0;
     }

//Read EEPROM for last Marquee Brightness Value
  Marquee_Brightness = EEPROM.read(Marquee_EEProm);
  EEPROM_Current_Value = EEPROM.read(Marquee_EEProm);
  analogWrite(Marquee, MQ_Bright[Marquee_Brightness]); 
  }

//Start Reading Inputs
  time = millis();
  Marquee_Sw_Status = digitalRead(Sw_Marquee);
  Fan_Sw_Toggle_Status = digitalRead(Sw_Fan);
  if ((Temp_Last_Read + Temp_Delay_Read) < time) {
    Temp_Read_Upper = TempUpper.readTemperature(true);
    Temp_Read_Lower = TempLower.readTemperature(true);
    Temp_Last_Read = time;
  }

//Screen Temp & Fan Status Display
  if ((EEPROM_Timer + EEPROM_Write_Delay) < time && Marquee_Brightness != EEPROM_Current_Value) {
    lcd.setCursor(0, 0);
    lcd.print("EEPROM SAVE     ");
    lcd.setCursor(0, 1);
    lcd.print("FAILED!!!!      ");
  }
  else if (Temp_Read_Upper >= Over_Temp_Trigger) {
    lcd.setCursor(0, 0);
    lcd.print("WARNING - HOT!  ");
    lcd.setCursor(0, 1);
    lcd.print("U");
    lcd.print(Temp_Read_Upper);
    lcd.print("F ");
    lcd.print("L");
    lcd.print(Temp_Read_Lower);
    lcd.print("F ");
  }
  else {
    lcd.setCursor(0, 0);
    lcd.print("Upper:");
    lcd.print(Temp_Read_Upper);
    lcd.print("F");
    lcd.setCursor(0, 1);
    lcd.print("Lower:");
    lcd.print(Temp_Read_Lower);
    lcd.print("F");
    if (Temp_Read_Upper > Temp_Trigger or Fan_Sw_Toggle_Last_State == 0) {
      lcd.setCursor(12, 0);
      lcd.print(" (*)");
      lcd.setCursor(12, 1);
      lcd.print(" (*)");
    }
    else if (Temp_Read_Lower > Temp_Trigger && Temp_Read_Upper < Temp_Trigger) {
      lcd.setCursor(12, 1);
      lcd.print(" (*)");
    }
    else {//if (Temp_Read_Upper <Temp_Trigger && Temp_Read_Lower < Temp_Trigger && Fan_Sw_Toggle_Last_State == 1) {
      lcd.setCursor(12, 0);
      lcd.print(" (+)");
      lcd.setCursor(12, 1);
      lcd.print(" (+)");
    }
  }

//Tempature Relay Control
  if (Temp_Read_Upper > Temp_Trigger) {
    digitalWrite(Fan_Upper, HIGH);
    digitalWrite(Fan_Lower, HIGH);
  }
  else if (Temp_Read_Lower > Temp_Trigger && Temp_Read_Upper < Temp_Trigger) {
    digitalWrite(Fan_Lower, HIGH);
  }
  else if (Temp_Read_Upper <Temp_Trigger && Temp_Read_Lower < Temp_Trigger && Fan_Sw_Toggle_Last_State == 1) {
    digitalWrite(Fan_Lower, LOW);
    digitalWrite(Fan_Upper, LOW);
  }

//Toggle Switch Relay Control
  if (Fan_Sw_Toggle_Status != Fan_Sw_Toggle_Last_Status) {
    if (Fan_Sw_Toggle_Status == LOW && Fan_Sw_Toggle_Last_State == 1) {
      digitalWrite(Fan_Upper, HIGH);
      digitalWrite(Fan_Lower, HIGH);
      Fan_Sw_Toggle_Last_State = 0;
    }
    else if (Fan_Sw_Toggle_Status == LOW && Fan_Sw_Toggle_Last_State == 0) {
      digitalWrite(Fan_Upper, LOW);
      digitalWrite(Fan_Lower, LOW);
      Fan_Sw_Toggle_Last_State = 1;
    }
  delay(500);
  }
  Fan_Sw_Toggle_Last_Status = Fan_Sw_Toggle_Status;

//Toggle Switch Marquee Brightness Control
  if (Marquee_Sw_Status != Marquee_Sw_Last_Status) {
    if (Marquee_Sw_Status == LOW) {
      ++Marquee_Brightness;
      if (Marquee_Brightness == 9){
        Marquee_Brightness=0;
      }
      analogWrite(Marquee, MQ_Bright[Marquee_Brightness]);
      EEPROM_Timer = time;
      delay(400);
    }
  Marquee_Sw_Last_Status = Marquee_Sw_Status;
  } 
 
//Write Brightness Value to EEPROM  
  if ((EEPROM_Timer + EEPROM_Write_Delay) < time && Marquee_Brightness != EEPROM_Current_Value) {
    EEPROM.write(Marquee_EEProm, Marquee_Brightness);
    EEPROM_Current_Value = EEPROM.read(Marquee_EEProm);
    if (Marquee_Brightness != EEPROM_Current_Value) {
      digitalWrite(Marquee, LOW);
      delay(EEPROM_Confirm_Flash);
      digitalWrite(Marquee, HIGH);
      delay(EEPROM_Confirm_Flash);
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("EEPROM SAVE");
    lcd.setCursor(0, 1);
    lcd.print("SUCCESSFUL!!!!");    
    digitalWrite(Pwr_LED, LOW);
    delay(EEPROM_Confirm_Flash);
    digitalWrite(Pwr_LED, HIGH);
    delay(EEPROM_Confirm_Flash);
    digitalWrite(Pwr_LED, LOW);
    delay(EEPROM_Confirm_Flash);
    digitalWrite(Pwr_LED, HIGH);
    delay(EEPROM_Confirm_Flash);
    digitalWrite(Pwr_LED, LOW);
    delay(EEPROM_Confirm_Flash);
    digitalWrite(Pwr_LED, HIGH);
    lcd.clear();
  }
}
