//Included Libraries
#include "WiFi.h"

//Pin Assignments
#define PowerLED 21
#define PowerButton 16
#define PowerRelay 17
#define WifiLED 22

//Variables
int ButtonStatus = 0;
int ButtonHoldStatus = 0;
int PowerState = 0;
int WifiStatus = 0;
int WifiLEDStatus = 0;
int WifiInitialize = 0;
unsigned long WifiDelay = 0;
unsigned long CurrentTime = 0;
unsigned long LastPress = 0;
unsigned long previousTime = 0;
const long timeoutTime = 2000;

//Wifi Credentals
const char* ssid = "XXSSIDXX";
const char* password =  "XXPASSXX";

//TCP Server Setup
WiFiServer server(82);

// Variable to store the HTTP requests
String RelayState = "off";
String header;

void setup() {
  // initialize the digital pin as an output.
  Serial.begin(115200);
  //  Server.begin();

  pinMode(PowerButton, INPUT);
  pinMode(PowerLED, OUTPUT);
  pinMode(PowerRelay, OUTPUT);
  pinMode(WifiLED, OUTPUT);
}

void loop() {
  CurrentTime = millis();
  ButtonStatus = digitalRead(PowerButton);
  if (ButtonStatus == 1 && CurrentTime - LastPress >= 30000) {
    ButtonPress();
  }
  if (WiFi.status() != 3) {
    WifiManagement();
  }
  if (WiFi.status() == 3 && WifiStatus == 0) {
    WifiConnection();
  }
  //  if (client) {
  HTTPRequest();
  //  }
}

void WifiManagement() {
  //Initilize and Connect to the Wifi Network
  if (WifiInitialize == 0) {
    WiFi.begin(ssid, password);
    WifiInitialize = 1;
    WifiStatus = 0;
    Serial.print("Connecting to ");
    Serial.print(ssid);
  }
  if (CurrentTime - WifiDelay >= 500) {
    if (WifiLEDStatus == 1) {
      digitalWrite(WifiLED, LOW);
      WifiLEDStatus = 0;
      WifiDelay = CurrentTime;
    }
    else {
      Serial.print(".");
      digitalWrite(WifiLED, HIGH);
      WifiLEDStatus = 1;
      WifiDelay = CurrentTime;
    }
  }
}

void WifiConnection() {
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(WifiLED, HIGH);
  server.begin();
  WifiInitialize = 0;
  WifiStatus = 1;
}


void ButtonPress() {
  if (PowerState == 0) {
    digitalWrite(PowerLED, HIGH);
    digitalWrite(PowerRelay, HIGH);
    RelayState = "on";
    PowerState = 1;

  }
  else {
    digitalWrite(PowerLED, LOW);
    digitalWrite(PowerRelay, LOW);
    RelayState = "off";
    PowerState = 0;
  }
  LastPress = CurrentTime;
}

void HTTPRequest() {
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    previousTime = CurrentTime;
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && CurrentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      CurrentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            if (header.indexOf("GET /ArcadePWR/toggle/activate") >= 0) {
              ButtonPress();
            }
            client.println("<p>GPIO 26 - State " + RelayState + "</p>");
            client.println();
            break;
          }
          else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
  }
}
