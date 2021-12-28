
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

void setup() {
  //Serial Setup
  Serial.begin(9600);
  //Input Setup
  pinMode(P1_A, INPUT_PULLUP);     //Player 1 Button A
  pinMode(P1_B, INPUT_PULLUP);     //Player 1 Button B
  pinMode(P1_C, INPUT_PULLUP);     //Player 1 Button C
  pinMode(P1_X, INPUT_PULLUP);     //Player 1 Button X
  pinMode(P1_Y, INPUT_PULLUP);     //Player 1 Button Y
  pinMode(P1_Z, INPUT_PULLUP);     //Player 1 Button Z
  pinMode(P1_ST, INPUT_PULLUP);    //Player 1 Button START
  pinMode(P1_SL, INPUT_PULLUP);    //Player 1 Button SELECT
  pinMode(P2_A, INPUT_PULLUP);     //Player 2 Button A
  pinMode(P2_B, INPUT_PULLUP);     //Player 2 Button B
  pinMode(P2_C, INPUT_PULLUP);     //Player 2 Button C
  pinMode(P2_X, INPUT_PULLUP);     //Player 2 Button X
  pinMode(P2_Y, INPUT_PULLUP);     //Player 2 Button Y
  pinMode(P2_Z, INPUT_PULLUP);     //Player 2 Button Z
  pinMode(P2_ST, INPUT_PULLUP);    //Player 2 Button START
  pinMode(P2_SL, INPUT_PULLUP);    //Player 2 Button SELECT
  pinMode(OPT1, INPUT_PULLUP);     //Option 1 (QUIT)
  pinMode(OPT2, INPUT_PULLUP);     //Option 2 (SAVE)
  pinMode(OPT3, INPUT_PULLUP);     //Option 3 (LOAD)
}

//Array Setup

void loop() {
Serial.print(digitalRead(P1_A));
Serial.print(digitalRead(P1_B));
Serial.print(digitalRead(P1_C));
Serial.print(digitalRead(P1_X));
Serial.print(digitalRead(P1_Y));
Serial.print(digitalRead(P1_Z));
Serial.print(digitalRead(P1_SL));
Serial.print(digitalRead(P1_ST));
Serial.print(digitalRead(P2_A));
Serial.print(digitalRead(P2_B));
Serial.print(digitalRead(P2_C));
Serial.print(digitalRead(P2_X));
Serial.print(digitalRead(P2_Y));
Serial.print(digitalRead(P2_Y));
Serial.print(digitalRead(P2_ST));
Serial.print(digitalRead(P1_SL));
Serial.print(digitalRead(OPT1));
Serial.print(digitalRead(OPT2));
Serial.println(digitalRead(OPT3));

}
