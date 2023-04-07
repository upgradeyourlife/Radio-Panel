#include <BitsAndDroidsFlightConnector.h>
BitsAndDroidsFlightConnector* connector = new BitsAndDroidsFlightConnector;
#include <LiquidCrystal_I2C.h>
#include <TM1637Display.h>
#include <Encoder.h>

#define SQdisplayCLK 10
#define SQdisplayDIO 11

#define SwapComButton 12 //swap butona 6te trqba da mine na Analog pin

#define SQ_Rotary1CLK 6
#define SQ_Rotary1DTA 5

LiquidCrystal_I2C LCD_COM(0x27,16,2);

//TM1637
TM1637Display SQdisplay = TM1637Display(SQdisplayCLK, SQdisplayDIO);
// Create an array that turns all segments ON
const uint8_t allON[] = {0xff, 0xff, 0xff, 0xff};

//--------------------------------
const int rotCom1Btn=4;
const int SHORT_PRESS_TIME = 300;
const int LONG_PRESS_TIME = 800;
const int PinDT=3;
const int PinCLK=2;
int SendShortPress = false;
int SendLongPress = false;

//Rotary for Transponder first 2 digits 
// const int TCASleftDT=5;
// const int TCASleftCLK=6;

//rotary encoder for sq that uses Encoder.h 
Encoder SQ_Rotary1(SQ_Rotary1CLK, SQ_Rotary1DTA);

// Variables will change ( for long and short rotary button push):
int lastState = LOW;  // the previous state from the input pin
int currentState;     // the current reading from the input pin
unsigned long pressedTime  = 0;
unsigned long releasedTime = 0;

long TimeOfLasDebounce = 0;
int DelayofDebounce = 0.01;

int PreviousCLK;
int PreviousDATA;

int PreviousTCASleftCLK;
int PreviousTCASleftDATA;
//--------------------------------

String Com1ActiveFreq, Com1StdbyFreq, oldCom1ActiveFreq, oldCom1StdbyFreq, Nav1ActiveFreq, oldNav1ActiveFreq, Nav1StbyFreq, oldNav1StbyFreq, TransponderCode, oldTransponderCode = "";
String zero = "0";

//int displaycounter=0;
bool skip = false;
bool skip2 = false;
int cmdInc, cmdDec, SQ_left_inc, SQ_left_dec = 0;
int lastSwapComButtonState;
bool readySwap = false;
long oldMhzPos, oldKhzPos = -1;
long newMhzPos, newKhzPos, newPos, oldPos, newPos2, oldPos2;
long ATCcodeLeftpart, ATCcodeRightpart;
long oldATCcodeLeftpart = -1;
// long LeftPart1, LeftPart2;
short int LeftPart1, LeftPart2;
int LeftPart2_int, LeftPart1_int;
String ThreedigitSQ, TwodigitSQ, OnedigitSQ, ModedSQ;
int passFlag = 0;

byte SelectedFreqDot[] =
{
0b00000,
0b00000,
0b00000,
0b00000,
0b00000,
0b00000,
0b00100,
0b00000
};

void setup() {
  Serial.begin(115200);
  //Serial.setTimeout(15);

  //attachInterrupt(digitalPinToInterrupt(2), check_rotary, CHANGE);

  PreviousCLK=digitalRead(PinCLK);
  PreviousDATA=digitalRead(PinDT);

 // Rotary Button config
  pinMode(rotCom1Btn, INPUT);
  pinMode(rotCom1Btn, INPUT_PULLUP);
  digitalWrite(rotCom1Btn, HIGH);

 //COM1 Swap Button
  pinMode(SwapComButton, INPUT);
  pinMode(SwapComButton, INPUT_PULLUP);  // enable the internal pull-up resistor for SWAP

  LCD_COM.init();
  LCD_COM.clear();
  LCD_COM.backlight();

  //TM 1637 LED Display  
  SQdisplay.clear();
  SQdisplay.setBrightness(1);
}

void check_rotary() {

  if ((PreviousCLK == 0) && (PreviousDATA == 1)) {
    if ((digitalRead(PinCLK) == 1) && (digitalRead(PinDT) == 0)) { 
      Serial.println(cmdInc);
      //Serial.println(displaycounter);
    }

  if ((digitalRead(PinCLK) == 1) && (digitalRead(PinDT) == 1)) {
      Serial.println(cmdDec);
      //Serial.println(displaycounter);
      }
  	}

  if ((PreviousCLK == 1) && (PreviousDATA == 0)) {
    if ((digitalRead(PinCLK) == 0) && (digitalRead(PinDT) == 1)) {
      Serial.println(cmdInc);
      //Serial.println(displaycounter);
    }

  if ((digitalRead(PinCLK) == 0) && (digitalRead(PinDT) == 0)){
      Serial.println(cmdDec);
      //Serial.println(displaycounter);
   }
  } 

  if ((PreviousCLK == 1) && (PreviousDATA ==1)) {
    if ((digitalRead(PinCLK) == 0) && (digitalRead(PinDT) == 1)) {
      Serial.println(cmdInc);
      //Serial.println(displaycounter);
    }

  if ((digitalRead(PinCLK) == 0) && (digitalRead(PinDT) == 0)){
      Serial.println(cmdDec);
      //Serial.println(displaycounter);
   }
  } 

  if ((PreviousCLK == 0) && (PreviousDATA == 0)) {
    if ((digitalRead(PinCLK) == 1) && (digitalRead(PinDT) == 0)) {
      Serial.println(cmdInc);
      //Serial.println(displaycounter);
    }

  if ((digitalRead(PinCLK) == 0) && (digitalRead(PinDT) == 1)){
      Serial.println(cmdDec);
      //Serial.println(displaycounter);
    }
  } 

}

void LCD_Control() {

  LCD_COM.createChar(0, SelectedFreqDot);
  LCD_COM.setCursor(1,0);
  LCD_COM.print("COM 1");

 // Active COM Freq
  if(!oldCom1ActiveFreq.equals(Com1ActiveFreq))
  { 
      String stringMHz = Com1ActiveFreq.substring(0,3);
      String stringKHz = Com1ActiveFreq.substring(3,6);
      LCD_COM.setCursor(0,1);
      LCD_COM.print(stringMHz + "." + stringKHz);
      oldCom1ActiveFreq = Com1ActiveFreq;  
  }

 // Standby COM Freq
  if(!Com1StdbyFreq.equals(oldCom1StdbyFreq))
  {
      String stringMHz = Com1StdbyFreq.substring(0,3);
      String stringKHz = Com1StdbyFreq.substring(3,6);
      LCD_COM.setCursor(9,1);
      LCD_COM.print(stringMHz + "." + stringKHz);
      oldCom1StdbyFreq = Com1StdbyFreq;
  }

}

void Transponder_Panel() {

 //-----Initial Transponder Screen refresh
      if (passFlag == 1) {
        delay(50);
        Serial.println(sendCommands::sendPMDG_73_ATC_Code_Left_Outer_Inc);
        delay(50);
        Serial.println(sendCommands::sendPMDG_73_ATC_Code_Left_Outer_Dec);
      passFlag++;
      }

  // if (TransponderCode != oldTransponderCode) {
  //   SQdisplay.showNumberDec(TransponderCode.toInt() );
  //   oldTransponderCode = TransponderCode;
  // }

    if (TransponderCode != oldTransponderCode) {
    // proverqva sq koda dali ima e 4, 3, 2 ili 1 cifra i ako e taka zna4i drugite ot pred za "0" koqto ne q izpisva i q dobavqme
        if (TransponderCode.length() == 3) {
        ModedSQ = zero + TransponderCode;
        //Serial.println("noviq kod: " + ThreedigitSQ);     
        delay(100);
        SQdisplay.showNumberDec(ModedSQ.toInt(), true);    
       }

      if (TransponderCode.length() == 2) {
        ModedSQ = zero + zero + TransponderCode;
        //Serial.println("noviq kod: " + TwodigitSQ);
        SQdisplay.showNumberDec(ModedSQ.toInt(), true);
      }

      if (TransponderCode.length() == 1) {
        ModedSQ = zero + zero + zero + TransponderCode;
        //Serial.println("noviq kod: " + OnedigitSQ);
        SQdisplay.showNumberDec(ModedSQ.toInt(), true);
       }

      if (TransponderCode.length() == 4) {
        // OnedigitSQ = zero + zero + zero + TransponderCode;
        // Serial.println("noviq kod: " + OnedigitSQ);
        ModedSQ = TransponderCode;
        SQdisplay.showNumberDec(TransponderCode.toInt());
       }

        String LeftPart1 = ModedSQ.substring(0,1);
        String LeftPart2 = ModedSQ.substring(1,2);

                //Serial.println(ModedSQ);
                // Serial.println("Parvoto 4islo: " + LeftPart1);
                // delay(50);
                //Serial.println("Vtoroto 4islo: " + LeftPart2);  

        LeftPart1_int = LeftPart1.toInt();
        LeftPart2_int = LeftPart2.toInt();

        Serial.println(LeftPart2_int);

    oldTransponderCode = TransponderCode;
  }

  newPos2 = SQ_Rotary1.read();
    if(newPos2 != oldPos2)
    {
      skip2 = !skip2;

        if(newPos2 < oldPos2) {
          if(!skip2) {
            if (LeftPart2_int >= 0 && LeftPart2_int <= 6 ) {             
              //Serial.println(sendCommands::sendPMDG_73_ATC_Code_Left_Inner_Inc); 
              Serial.println(sendCommands::sendXpndr100Inc);
            }  
            if (LeftPart2_int == 7) {
              //Serial.println(sendCommands::sendPMDG_73_ATC_Code_Left_Outer_Inc);
              Serial.println(sendCommands::sendXpndr1000Inc);
              delay(20);
              Serial.println(sendCommands::sendXpndr100Inc);
            }
          }
        }       

        if(newPos2 > oldPos2) {
          if(!skip2) {
            if (LeftPart2_int >= 1 && LeftPart2_int <= 7 ) {             
              Serial.println(sendCommands::sendXpndr100Dec);
            }
            if (LeftPart2_int == 0) {
              Serial.println(sendCommands::sendXpndr100Dec);
              delay(20);
              Serial.println(sendCommands::sendXpndr1000Dec);
            }
          }
        }

    oldPos2 = newPos2;

    }
      

}

void COM1_Panel() {

 //-----Initial Screen refresh
          if (passFlag == 0) {
          Serial.println(sendCommands::sendSwapCom1);
          delay(30);
          Serial.println(sendCommands::sendSwapCom1);
          passFlag++;
          }

 // Rotary control
  if(!readySwap)
  {
  // MHz
    LCD_COM.setCursor(14,0);
    LCD_COM.print(" ");
    newPos = newMhzPos;
    oldPos = oldMhzPos;
    cmdInc = sendCommands::sendCom1WholeInc;
    cmdDec = sendCommands::sendCom1WholeDec;
    LCD_COM.setCursor(10,0);
    LCD_COM.write(0);
  }
  else
  {
  // KHz
    LCD_COM.setCursor(10,0);
    LCD_COM.print(" ");
    newPos = newKhzPos;
    oldPos = oldKhzPos;
    cmdInc = sendCommands::sendCom1FractInc;
    cmdDec = sendCommands::sendCom1FractDecr;
    LCD_COM.setCursor(14,0);
    LCD_COM.write(0);
  } 

  if(!readySwap) 
    oldMhzPos = oldPos;
  else
    oldKhzPos = oldPos;

  //Rotary button
  if(SendShortPress == true)
  {
    delay(100);
    readySwap = !readySwap;
    SendShortPress = false;
  }

 //Rotary button
  // if(digitalRead(rotCom1Btn) == LOW)
  // {
  //   delay(300);
  //   readySwap = !readySwap;
  // }

 //----- COM1 swap Button at D12------
 int SwapComButtonState = digitalRead(SwapComButton);
  if (lastSwapComButtonState != SwapComButtonState) {
     // delay(50);
    if (SwapComButtonState == LOW)
      Serial.println(sendCommands::sendSwapCom1);
  lastSwapComButtonState = SwapComButtonState;
    if (readySwap)
    readySwap = !readySwap;
  }

}

void loop() {

 // Rotary COM1 debounce check:
  if ((millis() - TimeOfLasDebounce) > DelayofDebounce){

    check_rotary();

    PreviousCLK=digitalRead(PinCLK);
    PreviousDATA=digitalRead(PinDT);

    TimeOfLasDebounce=millis();
  }


 // Read the state of the rotary button if short or long pressed:
  currentState = digitalRead(rotCom1Btn);

  if(lastState == HIGH && currentState == LOW)        // button is pressed
    pressedTime = millis();
  else if(lastState == LOW && currentState == HIGH) { // button is released
    releasedTime = millis();
    
    long pressDuration = releasedTime - pressedTime;

    if( pressDuration > LONG_PRESS_TIME )
      SendLongPress = true;
    else
      SendShortPress = true;
  }

  // save the the last state
  lastState = currentState;

  // Load values from Sim (COM 1)
  connector->dataHandling();
  Com1ActiveFreq = connector->getActiveCom1();
  Com1StdbyFreq  = connector->getStandbyCom1();
  Nav1ActiveFreq = connector->getActiveNav1();
  Nav1StbyFreq = connector->getStandbyNav1();
  TransponderCode = connector->getTransponderCode1();

  COM1_Panel();  
  //NAV1_Panel();
  Transponder_Panel();
  LCD_Control();

   
}
