// Berreizeta Soldering station for A1322
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
// LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
LiquidCrystal_I2C lcd(0x27,20,4);
#define tempSensor A1 // A1
#define knob A3 // A3
#define iron 10 // D10
#define LED 7 // D5
//-------------------------------------------------------
int
Temp,
minTemp = 25, //Minimum aquired iron tip temp during testing (°C)
maxTemp = 500, //Maximum aquired iron tip temp during testing (°C)
minADC = 25, //Minimum aquired ADC value during minTemp testing
maxADC = 740, //Maximum aquired ADC value during minTemp testing
maxPWM = 255, //Maximum PWM Power
avgCounts = 20, //Number of avg samples
lcdInterval = 80, //LCD refresh rate (miliseconds)
pwm = 0, //System Variable
tempRAW = 0, //System Variable
knobRAW = 0, //System Variable
counter = 0, //System Variable
setTemp = 0, //System Variable
setTempAVG = 0, //System Variable
currentTempAVG = 0, //System Variable
previousMillis = 0; //System Variable
//------------------------
float
currentTemp = 0.0, //System Variable
store = 0.0, //System Variable
knobStore = 0.0; //System Variable
unsigned long currentMillis;
float X = 5.0 / 1023; // analog reading in volts based on 5 volt max
int i = 0;
int u = 0;
int value = 0;
int err = 0;
float temp = 0;
int samples = 1;
int units = 0;
int command = 0; // This is the command char, in ascii form, sent from the serial port
//--------------------------------------------------------------------------
void setup(){
Serial.begin(9600);
lcd.backlight();
lcd.init();
//lcd.begin(16,2);
lcd.clear();
digitalWrite(LED,HIGH);
pinMode(tempSensor,INPUT); //Set Temp Sensor pin as INPUT
pinMode(knob,INPUT); //Set Potentiometer Knob as INPUT
pinMode(iron,OUTPUT); //Set MOSFET PWM pin as OUTPUT
pinMode(LED,OUTPUT); //Set LED Status pin as OUTPUT
lcd.setCursor(0,1);lcd.print("PRESET T: ");
lcd.setCursor(0,0);lcd.print("ACTUAL T:");
lcd.setCursor(14,0);
lcd.print((char)223);
lcd.print("C");
//lcd.setCursor(14,1);
//lcd.print((char)223);
//lcd.print("C");
} // end setup
//---------------------------------------------------------------------
double Temp_Out(double Vin){ // calculates deg C from voltage for K thermocouple)
double Sum;
double a0 = 0;
double a1 = 2.5132785E-2;
double a2 = 6.0883423E-8;
double a3 = 5.5358209E-13;
double a4 = 9.3720918E-18;
Sum = a1 * Vin + a2* pow(Vin,2)+ a3* pow(Vin,3)+ a4* pow(Vin,4);
return Sum * 10000; //
}
//------------------------------------------------------------------------
void Average10() { // average 10 readings
float Ave;
int AveCt ;
AveCt = 0;
Ave = 0;
do {
Ave = Ave + analogRead(A1) ;
AveCt = AveCt + 1;
delay(1);
} while (AveCt < 10 ) ;
temp = (Ave / AveCt) / 408 ; // Convert to 5 volt reading to degrees F /204
temp = Temp_Out(temp);
}
//-----------------------------------------------------------------------
void loop(){
value = 0;
Average10(); // take averaged sample
knobRAW = analogRead(knob); //Get analog value of Potentiometer
setTemp = map(analogRead(knob), 0, 1023, minTemp, maxTemp); //Scale pot analog value into temp unit
//--------Get Average of Temp Sensor and Knob--------//
if (counter < avgCounts){ //Sum up temp and knob data samples
store = store + temp;
// store = store + currentTemp;
knobStore = knobStore + setTemp;
counter++;
}
else
{
currentTempAVG = (store / avgCounts) - 1; //Get temp mean (average)
setTempAVG = (knobStore / avgCounts); //Get knob - set temp mean (average)
knobStore = 0; //Reset storage variable
store = 0; //Reset storage variable
counter = 0; //Reset storage variable
}
//--------PWM Soldering Iron Power Control--------//
if (analogRead(knob) <= 10){ //Turn off iron when knob as at its lowest (iron shutdown)
digitalWrite(LED,LOW);
pwm = 0;
}
else if (temp <= setTemp - 2){ //Turn on iron when iron temp is lower than preset temp avec hystérésis
digitalWrite(LED,HIGH);
pwm = maxPWM;
}
else if (temp >= setTemp + 2){ //Turn off iron when iron temp is higher than preset temp avec hystérésis
digitalWrite(LED,LOW);
pwm = 0;
}
analogWrite(iron,pwm); //Apply the aquired PWM value from the three cases above
//--------Display Data------------------------------------------------
currentMillis = millis(); //Use and aquire millis function instead of using delay
if (currentMillis - previousMillis >= lcdInterval){ //LCD will only display new data ever n milisec intervals
previousMillis = currentMillis;

   if (analogRead(knob)== 0){
      lcd.setCursor(10,1);lcd.print("OFF ");
    Serial.println("PRESET: OFF");
   }
   else {
    if (setTempAVG < 100 ){
       lcd.setCursor(12,1);lcd.print(" ");
   }
      lcd.setCursor(10,1);
      lcd.print(setTempAVG);
    Serial.print("PRESET: ");
    Serial.println(setTempAVG);
   }
   if (temp < minTemp){
      lcd.setCursor(10,0);
      lcd.print("COOL");
   Serial.println("TEMP : COOL");
   }
  else
  {
   if (temp < 100 ){
    lcd.setCursor(12,0);
    lcd.print(" ");
  }
    lcd.setCursor(10,0);
    lcd.print(int(temp));
  Serial.print("TEMP : ");
  Serial.println(int(temp));
  Serial.println();
}
}
} // end loop
//----------------------------------------------------------------------
