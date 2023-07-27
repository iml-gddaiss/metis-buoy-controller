//Can't tell which version it is
//Update to have a auto calibration of the amp meter
//But started from a older version

#include <EEPROM.h>
#include "pins_arduino.h"

byte EepromAlreadyConfigure = 199;         // (1 = OK, 255 = new unit)
byte EepromBatterytoMainController = 200;  // (1 = Battery #1, 2 = Battery #2)
byte EepromManualtoMainController = 201;   //(0 = Autorun, 1 = Override)
byte EepromBatterytoWinch = 202;           //(1 = Battery #1, 2 = Battery #2)
byte EepromManualtoWinch = 203;            // (0 = Autorun, 1 = Override)
byte EepromBatterytoTurbine = 204;         //(1 = Battery #1, 2 = Battery #2)
byte EepromManualtoTurbine = 205;          // (0 = Autorun, 1 = Override)
byte EepromTurbineBrakeActivated = 206;    //(0 = Released, 1 = Brake On)
byte EepromTurbineRelayState = 207;        //(1 = Digital 11, 2 = Digital 12)

byte EepromTurbineScale = 100;
byte EepromTurbineOffset = 110;
byte EepromWinchScale = 120;
byte EepromWinchOffset = 130;
byte EepromSolarScale = 140;
byte EepromSolarOffset = 150;
byte EepromMainScale = 160;
byte EepromMainOffset = 170;
byte EepromIsCalibrated = 180;




//Relay State
int BatterytoMainController = 1;
int BatterytoTurbine = 1;
int BatterytoWinch = 1;
int TurbineRelayState = 1;

int TurbineState = 0;

//Manual Control
boolean ManualtoMainController = 0;
boolean ManualtoWinch = 0;
boolean ManualtoTurbine = 0;
boolean TurbineBrakeActivated = 0;

//Starting
boolean bootUp = 1;



//Analog input

const int AnalogWaterDetection = A7;
const int AnalogTurbine = A1;
const int AnalogMain = A2;
const int AnalogWinch = A3;
const int AnalogSolar = A4;

int WaterDetectionPercent = 0;


//Digital (0-5V) output

//int SerialMainRx = 0;
//int SerialMainTx = 1;
int Batt_1toMainController = 2;
//int SerialSolarTx = 3;
int Batt_2toMainController = 4;
int IfTurbineIsBraked = 5;
//int SerialSolarRx = 6;
int Batt_2toTurbine = 7;
int Batt_1toTurbine = 8;
int Batt_1toWinch = 10;
int Batt_2toWinch = 11;
int TurbineRelayState1 = 9;
int TurbineRelayState2 = A6;
int SerialTxRx = 13;

int AccPower = A5;
int ResetPin = 12;
int SerialSolar = A0;



//Variables

//Data entries variables
const byte numChars = 32;
char receivedChars[numChars];  // an array to store the received data
boolean newData = false;

//Timed sequence
unsigned long nextAverageData = 0; //to compare with millis()
unsigned long humidityTimer = 0; //to compare with millis()
boolean closeRelay = false;
unsigned long currentMillis = 0;
int timeBetweenAutoCollect = 30; //sec
unsigned long WinchDelayBatt1 = 0;
unsigned long WinchDelayBatt2 = 0;
unsigned long TurbineDelayBatt1 = 0;
unsigned long TurbineDelayBatt2 = 0;


//Configurable Menu
boolean needsEntry = false;
boolean allowsReset = false;
boolean allowsCalib = false;
boolean manualControl = false;
byte manualControlStep = 0;
byte calibrationStep = 0;
boolean autoControl = false;
byte autoControlStep = 0;
//string password = "Yes";

//Calibration variable
float zeroAnalogTurbine = 0;
float zeroAnalogWinch = 0;
float zeroAnalogSolar = 0;
float zeroAnalogMain = 0;

float twoAmpAnalogTurbine = 0;
float twoAmpAnalogWinch = 0;
float twoAmpAnalogSolar = 0;
float twoAmpAnalogMain = 0;


//new if Power Data from PV Solar
boolean newVBatt1 = 0;
boolean newABatt1 = 0;
boolean newVBatt2 = 0;
boolean newABatt2 = 0;
boolean newVSolar = 0;
boolean newASolar = 0;
boolean newAMain = 0;

//String to send to Solar Regulator for data
byte Solar[] = { 0xC0, 0xF0, 0x01, 0x00, 0x00, 0xE4, 0x62, 0xC0, 0xFF };
byte Batt1[] = { 0xC0, 0xF0, 0x02, 0x02, 0x00, 0x84, 0x93, 0xC0, 0xFF };
byte Batt2[] = { 0xC0, 0xF0, 0x02, 0x82, 0x00, 0x44, 0xF2, 0xC0, 0xFF };


//Voltage, Amperage and Relay
String WinchState = "not working";
float VBatt1 = 0;
float ABatt1 = 0;
float VBatt2 = 0;
float ABatt2 = 0;
float VSolar = 0;
float ASolar = 0;
float AWinch = 0;
float ATurbine = 0;
float AMain = 0;

//To send DV, averaged data
float VBatt1a = 0;
float ABatt1a = 0;
float VBatt2a = 0;
float ABatt2a = 0;
float VSolara = 0;
float ASolara = 0;
float AWincha = 0;
float ATurbinea = 0;
float AMaina = 0;


//Tab lenght can be bigger than lenghtOfTab, and must be a constant so the lenghtOfTab really define the average lenght
byte lenghtOfTab = 60;
int averageTab = 0;
boolean longAverage = 0;
float VBatt1Tab[60];
float ABatt1Tab[60];
float VBatt2Tab[60];
float ABatt2Tab[60];
float VSolarTab[60];
float ASolarTab[60];
float ATurbineTab[60];
float AMainTab[60];
float AWinchTab[60];


//Watch, Warning, Alarm and Error

int HumidityFlag = 0; //00 = NoError, 01 = NotPresent, 02 = Over40%, 03 = Over75%, 04 = Over90%



//Scale and Offset for Current Measurement
float TurbineScale = 0;
float TurbineOffset = 0;
float WinchScale = 0;
float WinchOffset = 0;
float SolarScale = 0;
float SolarOffset = 0;
float MainScale = 0;
float MainOffset = 0;
int navg = 15;

///Démarrage
///
///
///
///

void setup() {
  //réattribuer les pins
  pinMode(Batt_1toMainController, OUTPUT);
  pinMode(Batt_2toMainController, OUTPUT);
  pinMode(SerialTxRx, OUTPUT);
  pinMode(Batt_1toTurbine, OUTPUT);
  pinMode(Batt_2toTurbine, OUTPUT);
  pinMode(Batt_1toWinch, OUTPUT);
  pinMode(Batt_2toWinch, OUTPUT);
  pinMode(TurbineRelayState1, OUTPUT);
  pinMode(TurbineRelayState2, OUTPUT);
  pinMode(AccPower, OUTPUT);
  pinMode(IfTurbineIsBraked, INPUT);
  pinMode(ResetPin, OUTPUT);

  pinMode(SerialSolar, OUTPUT);




  Serial1.begin(19200);
  digitalWrite(SerialTxRx, HIGH);
  Serial1.println();
  Serial1.println();
  Serial1.println("Powering Arduino");
  Serial1.flush();

  //Update relay state and Man/Auto mode. Eeprom will be 255 if never uploaded with this code
  if (EEPROM.read(EepromAlreadyConfigure) == 255) {
    EEPROM.update(EepromBatterytoMainController, BatterytoMainController);
    EEPROM.update(EepromManualtoMainController, ManualtoMainController);
    EEPROM.update(EepromBatterytoWinch, BatterytoWinch);
    EEPROM.update(EepromManualtoWinch, ManualtoWinch);
    EEPROM.update(EepromBatterytoTurbine, BatterytoTurbine);
    EEPROM.update(EepromManualtoTurbine, ManualtoTurbine);
    EEPROM.update(EepromTurbineBrakeActivated, TurbineBrakeActivated);
    EEPROM.update(EepromTurbineRelayState, TurbineRelayState);
    EEPROM.update(EepromAlreadyConfigure, 1);
    Serial1.println("System set to Default configuration");
    Serial1.flush();
  } else {
    BatterytoMainController = EEPROM.read(EepromBatterytoMainController);
    ManualtoMainController = EEPROM.read(EepromManualtoMainController);
    BatterytoWinch = EEPROM.read(EepromBatterytoWinch);
    ManualtoWinch = EEPROM.read(EepromManualtoWinch);
    BatterytoTurbine = EEPROM.read(EepromBatterytoTurbine);
    ManualtoTurbine = EEPROM.read(EepromManualtoTurbine);
    TurbineBrakeActivated = EEPROM.read(EepromTurbineBrakeActivated);
    TurbineRelayState = EEPROM.read(EepromTurbineRelayState);
    Serial1.println("Previous configuration loaded");
    Serial1.flush();
  }

  if (EEPROM.read(EepromIsCalibrated) == 255){
    Serial1.println("The PCB needs to be calibrated");
  }
  else{
    EEPROM.get(EepromTurbineScale,TurbineScale);
    EEPROM.get(EepromTurbineOffset,TurbineOffset);
    EEPROM.get(EepromWinchScale,WinchScale);
    EEPROM.get(EepromWinchOffset,WinchOffset);
    EEPROM.get(EepromSolarScale,SolarScale);
    EEPROM.get(EepromSolarOffset,SolarOffset);
    EEPROM.get(EepromMainScale,MainScale);
    EEPROM.get(EepromMainOffset,MainOffset);
  }

  //if the Power Manager is set to work on manual on which parameter
  if (ManualtoMainController == true || ManualtoWinch == true || ManualtoTurbine == true) {
    Serial1.println();
    Serial1.println("!!!! Warning !!!!");
    Serial1.println();
    if (ManualtoMainController == true) { Serial1.println((String) "The Main Controller is only powered by Battery #" + BatterytoMainController); }
    if (ManualtoWinch == true) { Serial1.println((String) "The Winch in only powered by Battery #" + BatterytoWinch); }
    if (ManualtoTurbine == true) { Serial1.println((String) "The Turbine in only charging Battery #" + BatterytoTurbine); }
    Serial1.println();
    Serial1.println("End of Warning");
    Serial1.println();
    Serial1.flush();
  } else {
    Serial1.println("Power Manager is in Automated Control");
    Serial1.flush();
  }

  //Collecting data before starting to see if it's need to switch some relays
  autoCollectData();
  bootUp = 0;
  Serial1.println("   <Arduino is ready>");
  Serial1.print(">");
  Serial1.flush();
  digitalWrite(SerialTxRx, LOW);
}


///
///
///
///
/// Fin du démarrage

void batt_1toMain(){
  digitalWrite(Batt_1toMainController, HIGH);
  delay(60);
  digitalWrite(Batt_1toMainController, LOW);
  EEPROM.update(EepromBatterytoMainController, 1);
  BatterytoMainController = 1;
}
void batt_2toMain(){
  digitalWrite(Batt_2toMainController, HIGH);
  delay(60);
  digitalWrite(Batt_2toMainController, LOW);
  EEPROM.update(EepromBatterytoMainController, 2);
  BatterytoMainController = 2;
}
void batt_1toWinch(){
  digitalWrite(Batt_1toWinch, HIGH);
  delay(100);
  digitalWrite(Batt_1toWinch, LOW);
  EEPROM.update(EepromBatterytoWinch, 1);
  BatterytoWinch = 1;
}
void batt_2toWinch(){
  digitalWrite(Batt_2toWinch, HIGH);
  delay(100);
  digitalWrite(Batt_2toWinch, LOW);
  EEPROM.update(EepromBatterytoWinch, 2);
  BatterytoWinch = 2;
}
void batt_1toTurbine(){
  digitalWrite(Batt_1toTurbine, HIGH);
  delay(60);
  digitalWrite(Batt_1toTurbine, LOW);
  EEPROM.update(EepromBatterytoTurbine, 1);
  BatterytoTurbine = 1;
}
void batt_2toTurbine(){
  digitalWrite(Batt_2toTurbine, HIGH);
  delay(60);
  digitalWrite(Batt_2toTurbine, LOW);
  EEPROM.update(EepromBatterytoTurbine, 2);
  BatterytoTurbine = 2;
}

void collectRelativeHumidity(){
  digitalWrite(AccPower, HIGH);
  delay(20);
  WaterDetectionPercent = ((analogRead(AnalogWaterDetection)) - 1023) * -1;
  WaterDetectionPercent = WaterDetectionPercent / 8;
  if (WaterDetectionPercent > 100){WaterDetectionPercent = 100;}
  digitalWrite(AccPower, LOW);
  
  //Pour savoir si le sensor donne 0% pendant 24h (savoir s'il fonctionne)
  if (HumidityFlag == 0 && humidityTimer == 0){
    humidityTimer = millis();
  }
  else if (HumidityFlag == 0 && humidityTimer != 0){
    currentMillis = millis();
    if (currentMillis - humidityTimer > (1000 * 60 * 60 * 24)){
      HumidityFlag = 1;
      humidityTimer = 0;
    }
  }
  else if (WaterDetectionPercent != 0 && (humidityTimer != 0 || HumidityFlag == 1)){
    HumidityFlag = 0;
    humidityTimer = 0;
  }
  
  //donc si WaterDetectionPercent = 0, peut avoir Flag=1
  if (WaterDetectionPercent > 0 && WaterDetectionPercent < 40){HumidityFlag = 0;}
  if (WaterDetectionPercent >= 40 && WaterDetectionPercent < 75){HumidityFlag = 2;}
  if (WaterDetectionPercent >= 75 && WaterDetectionPercent < 90){HumidityFlag = 3;}
  if (WaterDetectionPercent >= 90){HumidityFlag = 4;}
  Serial1.println("");
  Serial1.flush();
}

void headerPowerData(){
  Serial1.println();
  Serial1.println("Collecting voltage and amperage informations");
  Serial1.flush();
}

void collectPowerData() {
  Serial3.begin(9600);
  digitalWrite(AccPower, HIGH);
  delay(20);
  //Collecting information from Solar
  for (int t=0; t<3; t++){
    byte PowerResult[25];
    int i = 0;
    digitalWrite(SerialSolar, HIGH);
    Serial3.write(Solar, sizeof(Solar));
    Serial3.flush();
    digitalWrite(SerialSolar, LOW);
    delay(40);

    while(Serial3.available()){
      int incomingByte = Serial3.read();
      PowerResult[i] = incomingByte;
      //Serial1.println(PowerResult[i],HEX);
      if (newVSolar == 0 && PowerResult[i] == 0x41 && PowerResult[i-5] == 0x0D && PowerResult[i-7] == 0x01){
        VSolar = ((PowerResult[i-1] * 0.625) + 81) / 10;
        if (VSolar > 17){
          VSolar = ((PowerResult[i-1] * 1.25) + 0.4) / 10;
        }
        //Serial1.println((String) "VSolar : " + VSolar);
        newVSolar = 1;
      }
      if (newASolar == 0 && (PowerResult[i] == 0x3E || PowerResult[i] == 0x3F || PowerResult[i] == 0x40 || PowerResult[i] == 0x41 || PowerResult[i] == 0x42)  && PowerResult[i-9] == 0x0D && PowerResult[i-11] == 0X01){
        //Serial1.println(PowerResult[i-1]+((PowerResult[i]-62)*255));
        ASolar = 0.0000000478*pow(PowerResult[i-1]+((PowerResult[i]-62)*255),3)-0.000027414*pow(PowerResult[i-1]+((PowerResult[i]-62)*255),2)+0.0056868293*(PowerResult[i-1]+((PowerResult[i]-62)*255))+0.45752933;
        
        if (ASolar < 1){
          ASolar = 0;
          float sum = 0;
          int k = 0;
          for(int t = 0; t < navg; t++){
            //Serial1.println((String)"ASolar from PVSolar : " + ASolar);
            ASolar = analogRead(AnalogSolar);
            if ((-1*SolarOffset/SolarScale)-2 < ASolar && ASolar < (-1*SolarOffset/SolarScale)+2){
              k++;
            }
            sum += ASolar;
              //Serial1.println((String)"ASolar from ACS712 = "+ ASolar);
              //Serial1.flush();
          }
          if (k < (navg/3)){
            ASolar = (SolarScale * (sum / navg)) + SolarOffset;
            newASolar = 1;
          }
          else {
            ASolar = 0;
          }
        }
        else {
          newASolar = 1;
        }
      }
      i++;
    }
    Serial1.print(".");
    if(newVSolar == 1 && newASolar == 1){break;}
  }

  //Collecting information from Battery 1
  for (int t=0; t<3; t++){
    byte PowerResult[25];
    int i = 0;
    digitalWrite(SerialSolar, HIGH);
    Serial3.write(Batt1, sizeof(Batt1));
    Serial3.flush();
    digitalWrite(SerialSolar, LOW);
    delay(40);
    while(Serial3.available()){
      int incomingByte = Serial3.read();
      PowerResult[i] = incomingByte;
      if (PowerResult[i] == 0x41 && PowerResult[i-7] == 0x0F && PowerResult[i-8] == 0x02){
        VBatt1 = ((PowerResult[i-1] * 0.625) + 81) / 10;
        newVBatt1 = 1;
      }
      if ((PowerResult[i] == 0x3E || PowerResult[i] == 0x3F || PowerResult[i] == 0x40 || PowerResult[i] == 0x41 || PowerResult[i] == 0x42)  && PowerResult[i-11] == 0x0F && PowerResult[i-12] == 0X02){
        ABatt1 = 0.1391 * exp((PowerResult[i-1]+((PowerResult[i]-62)*255)) * 0.0053);
        if (ABatt1 < 0.8){
          ABatt1 = (ASolar * 1.2903) - 0.1723;
          if(ABatt1 < 0){ABatt1 = 0;}
        }
        //Serial1.println((String)"ABatt1 : " + ABatt1);
        newABatt1 = 1;
      }
      i++;
    }
    Serial1.print(".");
    if (newVBatt1 == 1 && newABatt1 == 1){break;}
  }

  //Collecting information from Battery 2
  for (int t=0; t<3; t++){
    byte PowerResult[25];
    int i = 0;
    digitalWrite(SerialSolar, HIGH);
    Serial3.write(Batt2, sizeof(Batt2));
    Serial3.flush();
    digitalWrite(SerialSolar, LOW);
    delay(40);
    while(Serial3.available()){
      int incomingByte = Serial3.read();
      PowerResult[i] = incomingByte;
      //Serial1.println(PowerResult[i],HEX);
      if (PowerResult[i] == 0x41 && PowerResult[i-7] == 0x0F && PowerResult[i-8] == 0x82){
        //Serial1.println(PowerResult[i-1],HEX);
        //Serial1.println(PowerResult[i-1]);
        VBatt2 = ((PowerResult[i-1] * 0.625) + 81) / 10;
        //Serial1.println((String) "VBatt2 : " + VBatt2);
        newVBatt2 = 1;
      }
      if ((PowerResult[i] == 0x3E || PowerResult[i] == 0x3F || PowerResult[i] == 0x40 || PowerResult[i] == 0x41 || PowerResult[i] == 0x42)  && PowerResult[i-11] == 0x0F && PowerResult[i-12] == 0x82){
        ABatt2 = 0.1391 * exp((PowerResult[i-1]+((PowerResult[i]-62)*255)) * 0.0053);
        if (ABatt2 < 0.8){
          ABatt2 = (ASolar * 1.2903) - 0.1723;
          if(ABatt2 < 0){ABatt2 = 0;}
        }
        newABatt2 = 1;
      }
      i++;
    }
    Serial1.print(".");
    if (newVBatt2 == 1 && newABatt2 == 1){break;}
  }

  //Put values to 0 if no new values
  if (newVBatt1 == 1){newVBatt1 = 0;} else {VBatt1 = 0; newVBatt1 = 0;}
  if (newABatt1 == 1){newABatt1 = 0;} else {ABatt1 = 0; newABatt1 = 0;}
  if (newVBatt2 == 1){newVBatt2 = 0;} else {VBatt2 = 0; newVBatt2 = 0;}
  if (newABatt2 == 1){newABatt2 = 0;} else {ABatt2 = 0; newABatt2 = 0;}
  if (newVSolar == 1){newVSolar = 0;} else {VSolar = 0; newVSolar = 0;}
  if (newASolar == 1){newASolar = 0;} else {ASolar = 0; newASolar = 0;}
  

  if(VBatt1 != 0 && VBatt2 == 0){
    BatterytoMainController = 1;
    EEPROM.update(EepromManualtoMainController, 0);
    EEPROM.update(EepromBatterytoMainController, 1);
    if (EEPROM.read(EepromManualtoWinch) == 0 && BatterytoWinch == 2){batt_1toWinch();}
    if (EEPROM.read(EepromManualtoTurbine) == 0 && BatterytoTurbine == 2){batt_1toTurbine();}
  }

  if(VBatt1 == 0 && VBatt2 != 0){
    BatterytoMainController = 2;
    EEPROM.update(EepromManualtoMainController, 0);
    EEPROM.update(EepromBatterytoMainController, 2);
    if (EEPROM.read(EepromManualtoWinch) == 0 && BatterytoWinch == 1){batt_2toWinch();}
    if (EEPROM.read(EepromManualtoTurbine) == 0 && BatterytoTurbine == 1){batt_2toTurbine();}
  }

  //Power sends to the Winch
  for (int t=0; t<1; t++){
    float sum = 0;
    int k = 0;
    for (int j = 0; j < navg; j++){
      AWinch = analogRead(AnalogWinch);
      if ((-1*WinchOffset/WinchScale) - 2 < AWinch && AWinch < (-1*WinchOffset/WinchScale) + 2){ //Replace 0 values with noise 0A value
        k++;
        }
      //Serial1.println(AWinch);
      sum += AWinch;
    }
    //Serial1.println(sum / navg);
    //Serial1.println(sum);
    Serial1.print(".");
    if (k < (navg/3)){AWinch = WinchScale * (sum / navg) + WinchOffset;}
    else {AWinch = 0;}
  
  }

  //Collecting information from Turbine, doing the loop 1 time
  for (int t = 0; t<1; t++){
    int k = 0;
    float sum = 0;

    for (int j = 0; j < navg; j++){
      ATurbine = analogRead(AnalogTurbine);
      //Serial1.println(ATurbine);
      if ((-1*TurbineOffset/TurbineScale) - 2 < ATurbine && ATurbine < (-1*TurbineOffset*TurbineScale) + 2){ //Replace 0 values with noise around 0A
        k++;
      }
      //Serial1.println(ATurbine);
      //Serial1.flush();
      sum += ATurbine;
      //Serial1.println(sum);
    }
    //Serial1.println(sum / navg);
    Serial1.print(".");
    //Si le tier des valeurs (navg/3) sont autour de 0A, dire que l'éolienne donne 0A
    if (k < (navg/3)) {
      //Serial1.println((String)"ATurbine = "+ sum / navg);
      ATurbine = (TurbineScale * (sum / navg)) + TurbineOffset;
      } //si 1/3 des valeurs sont 0, écrire 0
    else{ATurbine = 0;}
    //Serial1.println((String)"ATurbine = "+ ATurbine);
    //Serial1.flush();

    /*if (ATurbine <= -0.1){
      //If the EEPROM is written as 1, first it tries to power State2 to put the brake 
      if (EEPROM.read(EepromTurbineRelayState) == 1){
        digitalWrite(TurbineRelayState2, HIGH);
        delay(60);
        digitalWrite(TurbineRelayState2, LOW);
        //Check if the turbine is braked. If not, then tries to power State 1 to put the brake
        TurbineState = digitalRead(IfTurbineIsBraked);
        if (TurbineState != HIGH){
          digitalWrite(TurbineRelayState1, HIGH);
          delay(60);
          digitalWrite(TurbineRelayState1, LOW);
          TurbineRelayState = 1;
          EEPROM.write(EepromTurbineRelayState, 1);
        }
        //If the turbine is braked after powering State2, continue
        else {
          TurbineRelayState = 2;
          EEPROM.write(EepromTurbineRelayState, 2);
        }
      }

      else if (EEPROM.read(EepromTurbineRelayState) == 2){
        digitalWrite(TurbineRelayState1, HIGH);
        delay(60);
        digitalWrite(TurbineRelayState1, LOW);
        TurbineState = digitalRead(IfTurbineIsBraked);
        if (TurbineState != HIGH){
          digitalWrite(TurbineRelayState2, HIGH);
          delay(60);
          digitalWrite(TurbineRelayState2, LOW);
          TurbineRelayState = 2;
          EEPROM.write(EepromTurbineRelayState, 2);
        }
        else {
          TurbineRelayState = 1;
          EEPROM.write(EepromTurbineRelayState, 1);
        }
      }

      TurbineBrakeActivated = 1;
      if (EEPROM.read(EepromTurbineBrakeActivated) == 0){
        EEPROM.write(EepromTurbineBrakeActivated, 1);
      }
    }*/
  }

  //Power drawn by Power Manager and Main Controller
  for (int t = 0; t<1; t++){
    int k = 0;
    float sum = 0;

    for (int j = 0; j < navg; j++){
      AMain = analogRead(AnalogMain);
      if ((-1*MainOffset/MainScale) - 2 < AMain && AMain < (-1*MainOffset/MainScale) + 2){
        k++;
      }
      //Serial1.println(AMain);
      //Serial1.flush();
      sum += AMain;
      //Serial1.println(sum);
    }
    //Serial1.println(sum/navg);
    if (k < (navg/3)){AMain = (MainScale * (sum / navg) + MainOffset) - 0.08;} //0.08 is the Power Manager draws when collecting information
    else {AMain = 0;}
    //Serial1.println((String)"AMain = "+ AMain);
      //Serial1.flush();
  }
  Serial3.end();

  digitalWrite(AccPower, LOW);

  TurbineState = digitalRead(IfTurbineIsBraked);
  if (TurbineState == HIGH){
    TurbineBrakeActivated = 1;
    EEPROM.update(TurbineBrakeActivated, 1);
  }

  else {
    TurbineBrakeActivated = 0;
    EEPROM.update(TurbineBrakeActivated, 0);
  }
  
  currentMillis = millis();
  //Conditons for the Turbine if the turbine is in auto mode and no braked
  if(ManualtoTurbine != 1 || TurbineBrakeActivated == 1){
    //Switch to battery 1 if battery 2 isn't giving current, the turbine is on batt2 and batt1 is present
    if(ABatt2 >= 0.5 && BatterytoTurbine == 2 && VBatt1 > 8){
      batt_1toTurbine();
    }
    //Switch to battery 2 if battery 1 isn't giving current, the turbine is on batt1 and batt2 is present
    if(ABatt1 >= 0.5 && BatterytoTurbine == 1 && VBatt2 > 8){
      batt_2toTurbine();
    }
    //Switch if there's no solar power and batt 2 is more than 0.3V higher than batt for 2H
    if(ASolar == 0 && BatterytoTurbine == 2 && VBatt2 - VBatt1 >= 0.3 && VBatt1 > 8){
      TurbineDelayBatt2 = 0;
      if(TurbineDelayBatt1 == 0){millis();}
      if(currentMillis - TurbineDelayBatt1 >= 1000*60*60*2){
        batt_1toTurbine();
        TurbineDelayBatt1 = 0;
      }
    }
    else if(ASolar == 0 && BatterytoTurbine == 1 && VBatt1 - VBatt2 >= 0.3 && VBatt2 > 8){
      TurbineDelayBatt1 = 0;
      if(TurbineDelayBatt2 == 0){millis();}
      if(currentMillis - TurbineDelayBatt2 >= 1000*60*60*2){
        batt_2toTurbine();
        TurbineDelayBatt2 = 0;
      }
    }
    else{
      TurbineDelayBatt1 = 0;
      TurbineDelayBatt2 = 0;
    }
  }

  //Conditions for the Winch
  if(ManualtoWinch != 1){
    if(ASolar == 0 && ATurbine == 0 && AWinch == 0 && VBatt2 - VBatt1 >= 0.3 && VBatt2 > 8){
      WinchDelayBatt1 = 0;
      if(WinchDelayBatt2 == 0){WinchDelayBatt2 = millis();}
      if(currentMillis - WinchDelayBatt2 >= 1000*60*15){
        batt_2toWinch();
      }
      WinchDelayBatt2 = 0;
    }

    else if(ASolar == 0 && ATurbine == 0 && AWinch == 0 && VBatt1 - VBatt2 >= 0.3 && VBatt1 > 8){
      WinchDelayBatt2 = 0;
      if(WinchDelayBatt1 == 0){WinchDelayBatt1 = millis();}
      if(currentMillis - WinchDelayBatt1 >= 1000*60*15){
        batt_1toWinch();
      }
      WinchDelayBatt1 = 0;
    }
    
    else{
      WinchDelayBatt1 = 0;
      WinchDelayBatt2 = 0;
    }

    if(AWinch == 0 && VBatt2 - VBatt1 >= 0.7 && VBatt1 < 12.2){
      batt_2toWinch();
    }

    if(AWinch == 0 && VBatt1 - VBatt2 >= 0.7 && VBatt2 < 12.2){
      batt_1toWinch();
    }
  }


  //Conditions for the Power Manager
  if(ManualtoMainController != 1){
    if(BatterytoWinch == 1 && AMain < 0.6 && VBatt2 - VBatt1 >= 0.3){
      batt_2toMain();
    }
    if(BatterytoWinch == 2 && AMain < 0.6 && VBatt1 - VBatt2 >= 0.3){
      batt_1toMain();
    }
  }
}


void showPowerData() {
  Serial1.println();
  Serial1.println();
  Serial1.print("Solar = ");Serial1.print(VSolar,1);Serial1.print("V, producing ");Serial1.print(ASolar,1);Serial1.println("A");
  Serial1.print("Batt1 = ");Serial1.print(VBatt1,1);Serial1.print("V, charging ");Serial1.print(ABatt1,1);Serial1.println("A from Solar");
  Serial1.print("Batt2 = ");Serial1.print(VBatt2,1);Serial1.print("V, charging ");Serial1.print(ABatt2,1);Serial1.println("A from Solar");
  if(TurbineBrakeActivated == 1){
    Serial1.println("The Turbine's Break is Activated");
  }
  else {
  Serial1.println((String) "Batt #" + BatterytoTurbine + " is being charged by the Turbine at " + ATurbine + "Ah.");
  }

  Serial1.println();
  Serial1.println((String) "Batt #" + BatterytoMainController + " is powering " + AMain + "A to the Buoy");

  
  Serial1.println((String) "Batt #" + BatterytoWinch + " is powering " + AWinch + "A to the Winch");
  
  Serial1.println();
  Serial1.flush();
}


void loop() {
  recvWithEndMarker();
  autoCollectData();
  showNewData();
}

float averageData(float x[], float y){
  x[averageTab] = y;
  float sumTab = 0;
  if (longAverage == 0){
      //Serial1.println("I'm here");
      for (int a = 0; a <= averageTab; a++){
        //Serial1.println(x[a]);
        sumTab += x[a];
        }
        y = sumTab / (averageTab + 1);
        //Serial1.println(y);
    }
    else {
      //Serial1.println("I'm there");
      for (int a = 0; a < lenghtOfTab; a++){
        //Serial1.println(x[a]);
        sumTab += x[a];
      }
      y = sumTab / (lenghtOfTab);
      //Serial1.println(y);
    }
  return y;
}

void autoCollectData(){
  currentMillis = millis();
  if(currentMillis >= nextAverageData){
    if(bootUp == 0){
      digitalWrite(SerialTxRx, HIGH);
      Serial1.println("");
      Serial1.print("Wait");
    }
    collectPowerData();
    collectRelativeHumidity();
    
    VBatt1a = averageData(VBatt1Tab, VBatt1);
    ABatt1a = averageData(ABatt1Tab, ABatt1);
    VBatt2a = averageData(VBatt2Tab, VBatt2);
    ABatt2a = averageData(ABatt2Tab, ABatt2);    
    VSolara = averageData(VSolarTab, VSolar);    
    ASolara = averageData(ASolarTab, ASolar);    
    ATurbinea = averageData(ATurbineTab, ATurbine);
    AMaina = averageData(AMainTab, AMain);
    AWincha = averageData(AWinchTab, AWinch);
    
    if (averageTab == lenghtOfTab - 1){
      averageTab = 0;
      longAverage = 1;
    }
    else{averageTab++;}
    
    nextAverageData = currentMillis + (timeBetweenAutoCollect*1000);

    if(bootUp == 0){
      Serial1.println();
      Serial1.print(">");
      Serial1.flush();
      digitalWrite(SerialTxRx, LOW);
    }

  }
}


void recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\r';
  char rc;

  while (Serial1.available() > 0 && newData == false) {

    rc = Serial1.read();
    if (rc == '\0'){}
    else if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      digitalWrite(SerialTxRx, HIGH);
      delay(20);
      Serial1.print(rc);
      Serial1.flush();
      digitalWrite(SerialTxRx, LOW);
      delay(20);
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    } else {
      receivedChars[ndx] = '\0';  // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void showNewData() {
  if (newData == true) {
    delay(60);
    digitalWrite(SerialTxRx, HIGH);
    Serial1.println();
    if (needsEntry == false) {
    
      //Automatic Control
      if (strcmp(receivedChars, "AC") == 0 || strcmp(receivedChars, "ac") == 0) {
        Serial1.println();
        Serial1.println("Do you want the Power Manager to go in Automated Control ? Y/N");
        Serial1.flush();
        needsEntry = true;
        autoControl = true;
      }
      //Manual Control
      else if (strcmp(receivedChars, "MC") == 0 || strcmp(receivedChars, "mc") == 0) {
        Serial1.println();
        Serial1.println("Do you want to manually choose batteries functions ? Y/N ");
        Serial1.flush();
        needsEntry = true;
        manualControl = true;
      }
      //Turbine Break
      else if (strcmp(receivedChars, "TB") == 0 || strcmp(receivedChars, "tb") == 0) {
        TurbineState = digitalRead(IfTurbineIsBraked);
        if (TurbineState == LOW){
          if (TurbineRelayState == 1){
            digitalWrite(TurbineRelayState2, HIGH);
            delay(60);
            digitalWrite(TurbineRelayState2, LOW);
            TurbineState = digitalRead(IfTurbineIsBraked);
            if (TurbineState != HIGH){
              digitalWrite(TurbineRelayState1, HIGH);
              delay(60);
              digitalWrite(TurbineRelayState1, LOW);
              TurbineRelayState = 1;
              EEPROM.write(EepromTurbineRelayState, 1);
            }
            else {
              TurbineRelayState = 2;
              EEPROM.write(EepromTurbineRelayState, 2);
            }
          }
          else if (TurbineRelayState == 2){
            digitalWrite(TurbineRelayState1, HIGH);
            delay(60);
            digitalWrite(TurbineRelayState1, LOW);
            TurbineState = digitalRead(IfTurbineIsBraked);
            if (TurbineState != HIGH){
              digitalWrite(TurbineRelayState2, HIGH);
              delay(60);
              digitalWrite(TurbineRelayState2, LOW);
              TurbineRelayState = 2;
              EEPROM.write(EepromTurbineRelayState, 2);
            }
            else {
              TurbineRelayState = 1;
              EEPROM.write(EepromTurbineRelayState, 1);
            }
          }
          
          TurbineBrakeActivated = 1;
          if (EEPROM.read(EepromTurbineBrakeActivated) == 0){
            EEPROM.write(EepromTurbineBrakeActivated, 1);          
          }
          Serial1.println("Turbine Brake Activated");
        }
        else{
          Serial1.println("Turbine Brake was already ON");
        }
      }
      //Turbine Release
      else if (strcmp(receivedChars, "TR") == 0 || strcmp(receivedChars, "tr") == 0) {
        TurbineState = digitalRead(IfTurbineIsBraked);
        if (TurbineState == HIGH){
          if (TurbineRelayState == 1){
            digitalWrite(TurbineRelayState2, HIGH);
            delay(60);
            digitalWrite(TurbineRelayState2, LOW);
            TurbineState = digitalRead(IfTurbineIsBraked);
            if (TurbineState != LOW){
              digitalWrite(TurbineRelayState1, HIGH);
              delay(60);
              digitalWrite(TurbineRelayState1, LOW);
              TurbineRelayState = 1;
              EEPROM.write(EepromTurbineRelayState, 1);
            }
            else {
              TurbineRelayState = 2;
              EEPROM.write(EepromTurbineRelayState, 2);
            }
          }
          else if (TurbineRelayState == 2){
            digitalWrite(TurbineRelayState1, HIGH);
            delay(60);
            digitalWrite(TurbineRelayState1, LOW);
            TurbineState = digitalRead(IfTurbineIsBraked);
            if (TurbineState != LOW){
              digitalWrite(TurbineRelayState2, HIGH);
              delay(60);
              digitalWrite(TurbineRelayState2, LOW);
              TurbineRelayState = 2;
              EEPROM.write(EepromTurbineRelayState, 2);
            }
            else {
              TurbineRelayState = 1;
              EEPROM.write(EepromTurbineRelayState, 1);
            }
          }
          TurbineBrakeActivated = 0;
          if (EEPROM.read(EepromTurbineBrakeActivated) == 1){
            EEPROM.write(EepromTurbineBrakeActivated, 0);          
          }
          Serial1.println("Turbine Brake Released");
        }
        else {
          Serial1.println("Turbine Brake was already OFF");
        }
      }

      //Show Power Data
      else if (strcmp(receivedChars, "DS") == 0 || strcmp(receivedChars, "ds") == 0) {
        headerPowerData();
        collectPowerData();
        showPowerData();
        collectRelativeHumidity();
        Serial1.println((String) "Relative Humidity : " + WaterDetectionPercent + "%");
        Serial1.println();
        Serial1.flush();
      }

      else if (strcmp(receivedChars, "DV") == 0 || strcmp(receivedChars, "dv") == 0){
        Serial1.print((String)VBatt1a + "," + ABatt1a + "," + VBatt2a + "," + ABatt2a + ",");
        Serial1.print((String)VSolara + "," + ASolara + "," + AMaina + "," + ATurbinea + "," + AWincha + "," + WaterDetectionPercent + ",");
        Serial1.println((String)ManualtoMainController + BatterytoMainController + ManualtoWinch + BatterytoWinch + ManualtoTurbine + BatterytoTurbine + TurbineBrakeActivated + HumidityFlag);
        Serial1.println();
        Serial1.flush();
      }

      else if (strcmp(receivedChars, "CalibAll") == 0 || strcmp(receivedChars, "CALIBALL") == 0 || strcmp(receivedChars, "caliball") == 0){
        Serial1.println();
        Serial1.println("Make sure there's no power going or coming from :");
        Serial1.println("The Winch");
        Serial1.println("The Turbine");
        Serial1.println("The Solar");
        Serial1.println("The Main");
        Serial1.println("Enter the password to start the calibration,");
        Serial1.flush();
        needsEntry = true;
        allowsCalib = true;
      }


      // Help Menu
      else if (strcmp(receivedChars, "H") == 0 || strcmp(receivedChars, "h") == 0 || strcmp(receivedChars, "?") == 0) {
        Serial1.println();
        Serial1.println();
        Serial1.println("### This is the Power Manager's Menu ###");
        Serial1.println();
        Serial1.println("Management Options :");
        Serial1.println("AC to return to Automated Control");        
        Serial1.println("MC to Manually Control batteries' function");
        Serial1.println("ReSet to Restart the device");
        Serial1.println();
        Serial1.println("Turbine Options :");
        Serial1.println("TB to brake the turbine");
        Serial1.println("TR to release the turbine's brake");
        Serial1.println("");
        Serial1.println("Other Options :");
        Serial1.println("CalibAll to Calibrate all the Amp Meter");
        Serial1.println("DC to Display Calibration");
        Serial1.println("DS to Display Status");
        Serial1.println("DV to Display Average Data");
        Serial1.println("H for Help");
        Serial1.println("W for Water detection sensor");
        Serial1.println("End");
        Serial1.println();
      }


      // Water Detect
      else if (strcmp(receivedChars, "W") == 0 || strcmp(receivedChars, "w") == 0) {
        collectRelativeHumidity();
        Serial1.println((String) "Relative Humidity : " + WaterDetectionPercent + "%");
        Serial1.println("");
      }

      // Reset
      else if (strcmp(receivedChars, "ReSeT") == 0){
        Serial1.println();
        Serial1.println("Do you want to reset the Power Manager ?  Y/N");
        Serial1.flush();
        needsEntry = true;
        allowsReset = true;
      }


      //Others (Null value or anything else not in the menu)
      else if (strcmp(receivedChars, "") == 0) {
      } else {
        Serial1.println("!Invalid Command, press h for help");
        Serial1.flush();
      }
      newData = false;
      Serial1.print(">");
      Serial1.flush();
      delay(40);
      digitalWrite(SerialTxRx, LOW);
    }


    //Submenu
    else {

      if (allowsReset == true) {
        if (strcmp(receivedChars, "y") == 0 || strcmp(receivedChars, "Y") == 0) {
          Serial1.println("Reset confirmed");
          Serial1.flush();
          needsEntry = false;
          allowsReset = false;
          digitalWrite(ResetPin, HIGH);
          delay(3000);
        } else {
          Serial1.println("Reset aborded");
          Serial1.flush();
          needsEntry = false;
          allowsReset = false;
        }
      }

      //Submenu of Automated Control
      else if (autoControl == true) {
        if (autoControlStep == 0 && (strcmp(receivedChars, "y") == 0 || strcmp(receivedChars, "Y") == 0)) {
          headerPowerData();
          collectPowerData();
          showPowerData();
          Serial1.println("Do you really want to put the Power Manager to Automated Control ? Y/N");
          Serial1.flush();
          autoControlStep = 1;
        } else if (autoControlStep == 1 && (strcmp(receivedChars, "y") == 0 || strcmp(receivedChars, "Y") == 0)) {
          ManualtoMainController = 0;
          if (EEPROM.read(EepromManualtoMainController) != 0){
            EEPROM.write(EepromManualtoMainController, 0);
          }
          ManualtoWinch = 0;
          if(EEPROM.read(EepromManualtoWinch) != 0){
            EEPROM.write(EepromManualtoWinch, ManualtoWinch);
          }
          ManualtoTurbine = 0;
          if (EEPROM.read(EepromManualtoTurbine) != 0){
            EEPROM.write(EepromManualtoTurbine, 0);
          }
          Serial1.println("The Power Manager is now set to Automated Control");
          Serial1.println();
          Serial1.flush();
          needsEntry = false;
          autoControl = false;
          autoControlStep = 0;
        }

        else {
          Serial1.println("Automated Control aborded");
          Serial1.flush();
          needsEntry = false;
          autoControl = false;
          autoControlStep = 0;
        }
      }

      //Submenu of Manual Control
      else if (manualControl == true) {
        if (manualControlStep == 0 && (strcmp(receivedChars, "y") == 0 || strcmp(receivedChars, "Y") == 0)) {
          headerPowerData();
          collectPowerData();
          showPowerData();
          Serial1.println("Choose one of the following :");
          Serial1.println("(1) Only use one battery pack for everything");
          Serial1.println("(2) Only use one battery pack for the Main Controller");
          Serial1.println("(3) Only use one battery pack for the Winch");
          Serial1.println("(4) Only charge one battery pack with the Turbine");
          Serial1.println();
          Serial1.flush();
          manualControlStep = 1;
        }

        //Submenu of Manual Control, choice (1)

        else if (manualControlStep == 1 && strcmp(receivedChars, "1") == 0) {
          Serial1.println();
          Serial1.println("Which battery must be used for everything ? 1 or 2");
          Serial1.flush();
          manualControlStep = 11;
        }

        else if (manualControlStep == 11 && strcmp(receivedChars, "1") == 0) {
          Serial1.println();
          Serial1.println("Do you really want to switch everything to Battery #1 ? Y/N");
          Serial1.flush();
          manualControlStep = 111;
        }

        else if (manualControlStep == 111 && (strcmp(receivedChars, "y") == 0 || strcmp(receivedChars, "Y") == 0)) {
          Serial1.println();

          batt_1toMain();
          if (EEPROM.read(EepromManualtoMainController) != 1){
            EEPROM.write(EepromManualtoMainController, 1);
          }

          batt_1toWinch();
          if (EEPROM.read(EepromManualtoWinch) != 1){
            EEPROM.write(EepromManualtoWinch, 1);
          }

          batt_1toTurbine();
          if (EEPROM.read(EepromManualtoTurbine) != 1){
            EEPROM.write(EepromManualtoTurbine, 1);
          }    

          Serial1.println("All powers switchs to Battery #1");
          Serial1.println();
          Serial1.flush();
          needsEntry = false;
          manualControl = false;
          manualControlStep = 0;
        }

        else if (manualControlStep == 11 && strcmp(receivedChars, "2") == 0) {
          Serial1.println();
          Serial1.println("Do you really want to switch everything to Battery #2 ? Y/N");
          Serial1.flush();
          manualControlStep = 112;
        }

        else if (manualControlStep == 112 && (strcmp(receivedChars, "y") == 0 || strcmp(receivedChars, "Y") == 0)) {
          Serial1.println();

          batt_2toMain();
          if (EEPROM.read(EepromManualtoMainController) != 1){
            EEPROM.write(EepromManualtoMainController, 1);
          }

          batt_2toWinch();
          if (EEPROM.read(EepromManualtoWinch) != 1){
            EEPROM.write(EepromManualtoWinch, 1);
          }

          batt_2toTurbine();
          if (EEPROM.read(EepromManualtoTurbine) != 1){
            EEPROM.write(EepromManualtoTurbine, 1);
          }    
          Serial1.println("All powers switchs to Battery #2");
          Serial1.println();
          Serial1.flush();
          needsEntry = false;
          manualControl = false;
          manualControlStep = 0;
        }

        //Submenu for Manual control, choice (2)

        else if (manualControlStep == 1 && strcmp(receivedChars, "2") == 0) {
          Serial1.println();
          Serial1.println("Which battery must power the Main Controller ? 1 or 2");
          Serial1.flush();
          manualControlStep = 12;

        } else if (manualControlStep == 12 && strcmp(receivedChars, "1") == 0) {
          Serial1.println();
          Serial1.println("Do you really want to power the Main Controller with Battery #1 ? Y/N");
          Serial1.flush();
          manualControlStep = 121;
        }

        else if (manualControlStep == 121 && (strcmp(receivedChars, "y") == 0 || strcmp(receivedChars, "Y") == 0)) {
          Serial1.println();
          batt_1toMain();
          if (EEPROM.read(EepromManualtoMainController) != 1){
            EEPROM.write(EepromManualtoMainController, 1);
          }
          Serial1.println("The Main Controller is now powered by Battery #1");
          Serial1.println();
          Serial1.flush();
          needsEntry = false;
          manualControl = false;
          manualControlStep = 0;
        }

        else if (manualControlStep == 12 && strcmp(receivedChars, "2") == 0) {
          Serial1.println();
          Serial1.println("Do you really want to power the Main Controller with Battery #2 ? Y/N");
          Serial1.flush();
          manualControlStep = 122;
        }

        else if (manualControlStep == 122 && (strcmp(receivedChars, "y") == 0 || strcmp(receivedChars, "Y") == 0)) {
          Serial1.println();
          batt_2toMain();
          if (EEPROM.read(EepromManualtoMainController) != 1){
            EEPROM.write(EepromManualtoMainController, 1);
          }
          Serial1.println("The Main Controller is now powered by Battery #2");
          Serial1.println();
          Serial1.flush();
          needsEntry = false;
          manualControl = false;
          manualControlStep = 0;
        }

        //Submenu for Manual control, choice (3)

        else if (manualControlStep == 1 && strcmp(receivedChars, "3") == 0) {
          Serial1.println();
          Serial1.println("Which battery must power the Winch ? 1 or 2");
          Serial1.flush();
          manualControlStep = 13;

        } else if (manualControlStep == 13 && strcmp(receivedChars, "1") == 0) {
          Serial1.println();
          Serial1.println("Do you really want to power the Winch with Battery #1 ? Y/N");
          Serial1.flush();
          manualControlStep = 131;
        }

        else if (manualControlStep == 131 && (strcmp(receivedChars, "y") == 0 || strcmp(receivedChars, "Y") == 0)) {
          Serial1.println();
          batt_1toWinch();
          if (EEPROM.read(EepromManualtoWinch) != 1){
            EEPROM.write(EepromManualtoWinch, 1);
          }
          Serial1.println("The Winch is now powered by Battery #1");
          Serial1.println();
          Serial1.flush();
          needsEntry = false;
          manualControl = false;
          manualControlStep = 0;
        }

        else if (manualControlStep == 13 && strcmp(receivedChars, "2") == 0) {
          Serial1.println();
          Serial1.println("Do you really want to power the Winch with Battery #2 ? Y/N");
          Serial1.flush();
          manualControlStep = 132;
        }

        else if (manualControlStep == 132 && (strcmp(receivedChars, "y") == 0 || strcmp(receivedChars, "Y") == 0)) {
          Serial1.println();
          batt_2toWinch();
          if (EEPROM.read(EepromManualtoWinch) != 1){
            EEPROM.write(EepromManualtoWinch, 1);
          }
          Serial1.println("The Winch is now powered by Battery #2");
          Serial1.println();
          Serial1.flush();
          needsEntry = false;
          manualControl = false;
          manualControlStep = 0;
        }

        //Submenu for Manual control, choice (4)

        else if (manualControlStep == 1 && strcmp(receivedChars, "4") == 0) {
          Serial1.println();
          Serial1.println("Which battery must be charged by the Turbine ? 1 or 2");
          Serial1.flush();
          manualControlStep = 14;

        } else if (manualControlStep == 14 && strcmp(receivedChars, "1") == 0) {
          Serial1.println();
          Serial1.println("Do you really want the Turbine to only charge Battery #1 ? Y/N");
          Serial1.flush();
          manualControlStep = 141;
        }

        else if (manualControlStep == 141 && (strcmp(receivedChars, "y") == 0 || strcmp(receivedChars, "Y") == 0)) {
          Serial1.println();
          batt_1toTurbine();
          if (EEPROM.read(EepromManualtoTurbine) != 1){
            EEPROM.write(EepromManualtoTurbine, 1);
          }
          Serial1.println("The Turbine is now only charging Battery #1");
          Serial1.println();
          Serial1.flush();
          needsEntry = false;
          manualControl = false;
          manualControlStep = 0;
        }

        else if (manualControlStep == 14 && strcmp(receivedChars, "2") == 0) {
          Serial1.println();
          Serial1.println("Do you really want the Turbine to only charge Battery #2 ? Y/N");
          ;
          Serial1.flush();
          manualControlStep = 142;
        }

        else if (manualControlStep == 142 && (strcmp(receivedChars, "y") == 0 || strcmp(receivedChars, "Y") == 0)) {
          Serial1.println();
          batt_2toTurbine();
          if (EEPROM.read(EepromManualtoTurbine) != 1){
            EEPROM.write(EepromManualtoTurbine, 1);
          }
          Serial1.println("The Turbine is now only charging Battery #2");
          Serial1.println();
          Serial1.flush();
          needsEntry = false;
          manualControl = false;
          manualControlStep = 0;
        }

        else {
          Serial1.println("Manual control aborded");
          Serial1.println();
          Serial1.flush();
          needsEntry = false;
          manualControl = false;
          manualControlStep = 0;
        }
      }

      //Submenu of CalibAll
      else if (allowsCalib == true){
        if (calibrationStep == 0 && strcmp(receivedChars, "Yes") == 0) {
          Serial1.println("Calib Started");
          Serial1.flush();
          digitalWrite(AccPower, HIGH);
          delay(20);
          zeroAnalogTurbine = 0;
          zeroAnalogWinch = 0;
          zeroAnalogSolar = 0;
          zeroAnalogMain = 0;
          twoAmpAnalogTurbine = 0;
          twoAmpAnalogWinch = 0;
          twoAmpAnalogSolar = 0;
          twoAmpAnalogMain = 0;

          for (int j = 0; j < 101; j++){
            zeroAnalogTurbine = zeroAnalogTurbine + analogRead(AnalogTurbine);
            zeroAnalogWinch = zeroAnalogWinch + analogRead(AnalogWinch);
            zeroAnalogSolar = zeroAnalogSolar + analogRead(AnalogSolar);
            zeroAnalogMain = zeroAnalogMain + analogRead(AnalogMain);
          }
          zeroAnalogTurbine = zeroAnalogTurbine / 100;
          zeroAnalogWinch = zeroAnalogWinch / 100;
          zeroAnalogSolar = zeroAnalogSolar / 100;
          zeroAnalogMain = zeroAnalogMain / 100;

          Serial1.println((String)"The zero value for the Turbine is : " + zeroAnalogTurbine);
          Serial1.println((String)"The zero value for the Solar is : " + zeroAnalogSolar);          
          Serial1.println((String)"The zero value for the Winch is : " + zeroAnalogWinch);
          Serial1.println((String)"The zero value for the Main is : " + zeroAnalogMain);
          Serial1.println();
          Serial1.println("Output 2A on the Turbine bulkhead and press Enter");
          Serial1.println();
          Serial1.flush();
          calibrationStep = 1;
        }

        else if (calibrationStep == 1 && strcmp(receivedChars, "") == 0) {
          for (int j = 0; j < 100; j++){
            twoAmpAnalogTurbine = twoAmpAnalogTurbine + analogRead(AnalogTurbine);
          }
          twoAmpAnalogTurbine = twoAmpAnalogTurbine / 100;
          Serial1.println((String)"The 2A value for the Turbine is : " + twoAmpAnalogTurbine);
          Serial1.println();
          Serial1.println("Output 2A on the Solar bulkhead and press Enter");
          Serial1.println();
          Serial1.flush();
          calibrationStep = 2;     
        }

        else if (calibrationStep == 2 && strcmp(receivedChars, "") == 0) {
          for (int j = 0; j < 100; j++){
            twoAmpAnalogSolar = twoAmpAnalogSolar + analogRead(AnalogSolar);
          }
          twoAmpAnalogSolar = twoAmpAnalogSolar / 100;
          Serial1.println((String)"The 2A value for the Solar is : " + twoAmpAnalogSolar);
          Serial1.println();
          Serial1.println("Now Draw 2A on the Winch bulkhead and press Enter");
          Serial1.println();
          Serial1.flush();
          calibrationStep = 3;     
        }

        else if (calibrationStep == 3 && strcmp(receivedChars, "") == 0) {
          for (int j = 0; j < 100; j++){
            twoAmpAnalogWinch = twoAmpAnalogWinch + analogRead(AnalogWinch);
          }
          twoAmpAnalogWinch = twoAmpAnalogWinch / 100;
          Serial1.println((String)"The 2A value for the Winch is : " + twoAmpAnalogWinch);
          Serial1.println();
          Serial1.println("Draw 2A on the Main bulkhead and press Enter");
          Serial1.println();
          Serial1.flush();
          calibrationStep = 4;     
        }

        else if (calibrationStep == 4 && strcmp(receivedChars, "") == 0) {
          for (int j = 0; j < 100; j++){
            twoAmpAnalogMain = twoAmpAnalogMain + analogRead(AnalogMain);
          }
          twoAmpAnalogMain = twoAmpAnalogMain / 100;
          Serial1.println((String)"The 2A value for the Main is : " + twoAmpAnalogMain);
          Serial1.println();

          TurbineScale = 2 / (twoAmpAnalogTurbine - zeroAnalogTurbine);
          TurbineOffset = 2 - (TurbineScale * twoAmpAnalogTurbine);

          WinchScale = 2 / (twoAmpAnalogWinch - zeroAnalogWinch);
          WinchOffset = 2 - (WinchScale * twoAmpAnalogWinch);

          SolarScale = 2 / (twoAmpAnalogSolar - zeroAnalogSolar);
          SolarOffset = 2 - (SolarScale * twoAmpAnalogSolar);

          MainScale = 2 / (twoAmpAnalogMain - zeroAnalogMain);
          MainOffset = 2 - (MainScale * twoAmpAnalogMain);

          Serial1.println((String)"The Turbine equation is : y = " + (TurbineScale, 5) + "x + " + (TurbineOffset, 5));
          Serial1.println((String)"The Solar equation is : y = " + (SolarScale, 5) + "x + " + (SolarOffset, 5));
          Serial1.println((String)"The Winch equation is : y = " + (WinchScale, 5) + "x + " + (WinchOffset, 5));
          Serial1.println((String)"The Main equation is : y = " + (MainScale, 5) + "x + " + (MainOffset , 5));
          Serial1.println();
          Serial1.println("The calibration is now complete");
          Serial1.println("Do you want to save these equations ? Y/N");
          Serial1.println();
          Serial1.flush();
          calibrationStep = 5;
        }

        else if (calibrationStep == 5 && strcmp(receivedChars, "Y") == 0) {
          EEPROM.put(EepromTurbineScale,(TurbineScale, 5));
          EEPROM.put(EepromTurbineOffset,(TurbineOffset, 5));
          EEPROM.put(EepromWinchScale,(WinchScale, 5));
          EEPROM.put(EepromWinchOffset,(WinchOffset, 5));
          EEPROM.put(EepromSolarScale,(SolarScale, 5));
          EEPROM.put(EepromSolarOffset,(SolarOffset, 5));
          EEPROM.put(EepromMainScale,(MainScale, 5));
          EEPROM.put(EepromMainOffset,(MainOffset, 5));
          Serial1.println("The above equations are now saved in the EEPROM");
          EEPROM.update(EepromIsCalibrated,1);
          Serial1.println("The Arduino will now restart to apply the new values");
          allowsCalib = false;
          calibrationStep = 0;
          needsEntry = false;
          digitalWrite(AccPower, LOW);
          digitalWrite(ResetPin, HIGH);
          delay(3000);
        }

        else {
          Serial1.println("Calib aborded");
          Serial1.flush();
          digitalWrite(AccPower, LOW);
          needsEntry = false;
          allowsCalib = false;
          calibrationStep = 0;
        }
      }

      newData = false;
      Serial1.print(">");
      Serial1.flush();
      digitalWrite(SerialTxRx, LOW);
    }
  }
}