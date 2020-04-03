//Libraries
#include "DHT.h"
#include <EEPROM.h>

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
#define Pwr_LED 13  //Power LED


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
int MQ_Bright[] = {230, 172, 115, 58, 0};

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
int Temp_Delay_Read = 2000;         //Delay Between Temp Reads
float Temp_Read_Upper;              //Upper Temp Sensor Reading
float Temp_Read_Lower;              //Lower Temp Sensor Reading
//Fan Switch Variables
int Fan_Sw_Toggle_Last_Status = 1;  //Last Status of Fan Switch
int Fan_Sw_Toggle_Last_State = 1;   //State to Disable Temp Sensors from Turning off Fans
int Fan_Sw_Toggle_Status;           //Current Status of Fan Switch
//EEPROM Variables
int EEPROM_Timer;                   //Time (Millis) of Last Marquee Change     
int EEPROM_Write_Delay = 15000;      //Delay before EEPROM Write to prevent to many writes to one address
int EEPROM_Current_Value = 0;       //Current EEPROM Value
int EEPROM_Confirm_Flash = 250;     //Milliseconds for Flash on EEPROM Write Confirm

//Temp Assignments
DHT TempUpper(Temp_Upper, DHTTYPE);
DHT TempLower(Temp_Lower, DHTTYPE);

void setup() {
  Serial.begin(9600);
  TempUpper.begin();
  TempLower.begin();
  pinMode(Fan_Upper, OUTPUT);
  pinMode(Fan_Lower, OUTPUT);
  pinMode(Marquee, OUTPUT);
  pinMode(Sw_Marquee, INPUT_PULLUP);
  pinMode(Sw_Fan, INPUT_PULLUP);
  pinMode(Sw_Future1, INPUT_PULLUP);
  pinMode(Sw_Future2, INPUT_PULLUP);
  pinMode(Pwr_LED, OUTPUT);
  digitalWrite(Pwr_LED, HIGH);
}

void loop() {
  if (Marquee_Flicker_Status == 1){                                //Marquee Fluorescent Flicker Emulation
    for(int i=0; i<33; i++) {
        analogWrite(Marquee, Flicker_High[i]);
        delay(Flicker[i]);
        ++i;
        analogWrite(Marquee, Flicker_Low[i]);
        delay(Flicker[i]);
        analogWrite(Marquee, Flicker_High[i]);        
//        Serial.print("Position: ");
//        Serial.print(i);
//        Serial.print("   Value: ");
//        Serial.print(Flicker[i]);
//        Serial.print("   brightness: ");
//        Serial.println(Flicker_High[i]);
        Marquee_Flicker_Status = 0;
     }
  Marquee_Brightness = EEPROM.read(Marquee_EEProm);              //Read EEPROM for last Marquee Brightness Value
  EEPROM_Current_Value = EEPROM.read(Marquee_EEProm);
//  Serial.println("EEPROM Read");
//  Serial.println(Marquee_Brightness);
  analogWrite(Marquee, MQ_Bright[Marquee_Brightness]); 
  }
  time = millis();                                                //Start/Reads
  Marquee_Sw_Status = digitalRead(Sw_Marquee);
  Fan_Sw_Toggle_Status = digitalRead(Sw_Fan);
  if ((Temp_Last_Read + Temp_Delay_Read) < time) {
    Temp_Read_Upper = TempUpper.readTemperature(true);
    Temp_Read_Lower = TempLower.readTemperature(true);
    Temp_Last_Read = time;
//    Serial.print("Upper Sensor: ");
//    Serial.print(Temp_Read_Upper);
//    Serial.print(F("°F"));
//    Serial.print("     Lower Sensor: ");
//    Serial.print(Temp_Read_Lower);
//    Serial.println(F("°F"));
 }

  if (Temp_Read_Upper > Temp_Trigger) {                              //Temp Related Realy Control
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

  if (Fan_Sw_Toggle_Status != Fan_Sw_Toggle_Last_Status) {             //Fan Toggle Switch
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
 
  if (Marquee_Sw_Status != Marquee_Sw_Last_Status) {                       //Marquee Toggle Switch
    if (Marquee_Sw_Status == LOW) {
      ++Marquee_Brightness;
      if (Marquee_Brightness == 5){
        Marquee_Brightness=0;
      }
      analogWrite(Marquee, MQ_Bright[Marquee_Brightness]);
      EEPROM_Timer = time;
//      Serial.print("Marquee_Sw_Status: "); 
//      Serial.print(Marquee_Sw_Status);
//      Serial.print("   MV: ");
//      Serial.println(MQ_Bright[Marquee_Brightness]);
//      Serial.println("EEPROM Write!!");
//      Serial.println(Marquee_Brightness);  
//      Serial.print("   Marquee_Brightness: ");
//      Serial.println(Marquee_Brightness);   
      delay(400);
    }
  Marquee_Sw_Last_Status = Marquee_Sw_Status;
  } 
  if ((EEPROM_Timer + EEPROM_Write_Delay) < time && Marquee_Brightness != EEPROM_Current_Value) {
    EEPROM.write(Marquee_EEProm, Marquee_Brightness);                      //Write Brightness Value to EEPROM
    EEPROM_Current_Value = EEPROM.read(Marquee_EEProm);
    if (Marquee_Brightness != EEPROM_Current_Value) {
      digitalWrite(Marquee, LOW);
      delay(EEPROM_Confirm_Flash);
      digitalWrite(Marquee, HIGH);
      delay(EEPROM_Confirm_Flash);
    }
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
//    Serial.print("   EEPROM_Current_Value: ");
//    Serial.print(EEPROM_Current_Value);
//    Serial.println("WRITE!");
  }
}
