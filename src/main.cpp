#include <Arduino.h>


#define BLINK_LED 13
#define POWER_ON_LED 12
#define POWER_ACTION_LED 11
#define POWER_RELAY 9
#define OPTION_BUTTON 8
#define CURRENT_READ A1   // A0
#define ANNOYING_BUZZER 10

const uint32_t g_waitFor = 200;           // general delay
const uint32_t g_samplePeriod = 900;      // check every xxx millseconds
const uint32_t g_powerOffDelay = 3000;    // when power goes off on the monitored power socket
const int16_t g_allowance = 5;            // amount added to the threshold when being setup

bool g_powerOn = false;
bool g_powerActive = false;
char g_pout[200] = {};  // used for output to the serial port

// Sensitivity of various Hall-Effect Current Sensors
// https://www.sparkfun.com/datasheets/BreakoutBoards/0712.pdf
// https://cdn.sparkfun.com/assets/d/0/0/5/e/ACS723-Datasheet.pdf
// ACS712-05 185mV/A  - 5A
// ACS712-20 100mV/A  - 20A
// ACS732-05 400mV/A  - 5A
// ACS732-20 200mV/A  - 20A

int16_t g_adcReading = 0;
int16_t g_threshold = 0; 
int16_t g_tempThreshold = 0;
bool g_optionAction = false;

// used to perform a soft reset, in case you need to set the threshold again
// as an alternative to switching off and back on
void(* resetFunc) (void) = 0;


// this is not being used, but might be useful in th future
int16_t getAverageReading() {

  uint32_t tNow = millis();
  int16_t samples = 0;
  int16_t samplesCount = 0;

  while ((millis() - tNow) < g_samplePeriod) {
    g_adcReading = analogRead(CURRENT_READ);
    samples = samples + g_adcReading;
    samplesCount++;
    delay(3);
  }

  return samples / samplesCount;
}

// plays a tone for the option given, to help pick the correct option
bool playOptionTone(uint8_t opt, bool buzzOnce) {

  int toneStart = 3000;
  int tonePlus = 250;
  uint32_t toneLen = 120;

  if (opt == 0) {
    return false;
  }

  if (buzzOnce) {
    return buzzOnce;  // returns true
  }

  for (uint8_t i = 0; i < opt; i++) {
    tone(ANNOYING_BUZZER,toneStart,toneLen);
    delay(toneLen + 50);
    toneStart = toneStart + tonePlus;
    Serial.print(i);
    Serial.print(" ");
  }

  Serial.println( " end");
  return true;
}


// check to see if the button is being pressed and of so, for how long
// to select an option depending on how long the button has been pressed
void buttonPressCheck() {

  uint8_t optionButton = 0;
  uint32_t optionBtnPressTime = 0;
  uint32_t optionButTimer = millis();
  bool buzzOnce = false;

  if (digitalRead(OPTION_BUTTON) == HIGH) {
    digitalWrite(BLINK_LED, HIGH);
    Serial.println("button pressed");
    // waits until the button is released. Provides an option based on 
    // the duration of pressing
    do
    {
      delay(3);
      optionBtnPressTime = abs(millis() - optionButTimer);

      if (optionBtnPressTime >= 100 && optionBtnPressTime <= 3999) {
        optionButton = 1;
        buzzOnce = playOptionTone(optionButton, buzzOnce);
      }
      else if (optionBtnPressTime > 4100 && optionBtnPressTime <= 5999) {
        optionButton = 2;
        buzzOnce = playOptionTone(optionButton, buzzOnce);
      }
      else if (optionBtnPressTime >= 6100 && optionBtnPressTime <= 7999) {
        optionButton = 3;
        buzzOnce = playOptionTone(optionButton, buzzOnce);          
      }
      else {
        buzzOnce = false;
      }

    }
    while (digitalRead(OPTION_BUTTON) == HIGH);
    digitalWrite(BLINK_LED, LOW);
  }

  // carry out the selected option
  switch (optionButton)
  {
    // toggle swiched socket on all the time
    case (1): 
      g_optionAction = !(g_optionAction);   // toggle the boolean
      if (g_optionAction) {
        g_tempThreshold = g_threshold;
        g_threshold = -1;  // set threshold below anything the adc will read
        g_powerOn = true;
      }
      else {
        g_threshold = g_tempThreshold;
      }      
      break;
    
    // toggle swiched socket off all the time
    case (2): 
      g_optionAction = !(g_optionAction);
      if (g_optionAction) {
        g_tempThreshold = g_threshold;
        g_threshold = 9999; // set threshold above anything the adc will read
        g_powerOn = false;
      }
      else
      {
        g_threshold = g_tempThreshold;
      }
      break;

    // soft reset the microcontroller
    case(3):
      digitalWrite(POWER_ON_LED, LOW);
      digitalWrite(POWER_RELAY, LOW);
      digitalWrite(BLINK_LED, HIGH);
      delay(1500);
      resetFunc();
      delay(1000);
      break;

    case (0):
    default:
      break;
  }

}

// finds the highest reding on the ADC pin for a given period of time
int16_t getMaxReading()
{
  uint32_t tNow = millis();
  int16_t maxSample = 0;

  while ((millis() - tNow) < g_samplePeriod)
  {
    g_adcReading = analogRead(CURRENT_READ);
    if (g_adcReading > maxSample) {
      maxSample = g_adcReading;
    }
    buttonPressCheck();
    delay(3);
  }

  return maxSample;
}

// sets the state of teh switched socket. Keeps reasserting that state until the power
void triggerRelay() {

  uint32_t tNow = millis();
  if (g_powerActive) {
    digitalWrite(POWER_ACTION_LED, HIGH);
  }

  if (!g_powerOn) {
    if (g_powerActive) {    
      tone(ANNOYING_BUZZER,3000,250);
      // power off delay begins
      // check to see if the power comes back on during the powerOffDelay
      while ((millis() - tNow) < g_powerOffDelay)
      {
        digitalWrite(BLINK_LED, HIGH);
        g_adcReading = getMaxReading();
        if (g_adcReading > g_threshold) { 
          // power resumed
          g_powerOn = true;
          digitalWrite(POWER_ACTION_LED, LOW);
          delay(g_waitFor);
          return;
        }
        digitalWrite(BLINK_LED, LOW);
        delay(g_waitFor);
      }

      // power definatley off
      g_powerActive = false;
    }

    // turn thw swiched socked off, or reassert that it is off
    digitalWrite(POWER_ON_LED, LOW);
    digitalWrite(POWER_RELAY, LOW);
    delay(10);
    digitalWrite(POWER_ACTION_LED, LOW);
    return;
  }

  // turn the swiched socket on, or reassert that it is on
  digitalWrite(POWER_ON_LED, HIGH);
  digitalWrite(POWER_RELAY, HIGH);
  delay(100);
  digitalWrite(POWER_ACTION_LED, LOW);
  g_powerActive = true;
}

void setup() {
  Serial.begin(9600);
  Serial.println("ready");

  pinMode(BLINK_LED, OUTPUT);
  pinMode(POWER_ON_LED, OUTPUT);
  pinMode(POWER_RELAY, OUTPUT);
  pinMode(POWER_ACTION_LED, OUTPUT);
  pinMode(OPTION_BUTTON, INPUT);
  pinMode(ANNOYING_BUZZER, OUTPUT);

  // make sure the swiched socket is off
  g_powerOn = false;
  triggerRelay();   
  delay(500);

  // setup the threshold by getting a reading from the adc and adding an allowance
  // Can be fooled if there is a load already on the power socket
  g_adcReading = getMaxReading();
  g_threshold = g_adcReading + g_allowance;  

  // play a little melody
  int toneMelody[5] = { 1600, 1900, 2000, 2100, 1800 };
  for (int i = 0; i < 4; i++) {
    tone(ANNOYING_BUZZER,toneMelody[i]-380,80);
    delay(85);
  }
  delay(500);  
  
}

void loop() {

  digitalWrite(BLINK_LED, HIGH);
  g_adcReading = getMaxReading();

  if (g_adcReading > g_threshold) {
    g_powerOn = true;
  }
  else { 
    g_powerOn = false;
  }
  
  triggerRelay();
  digitalWrite(BLINK_LED,LOW);
  delay(g_waitFor);
}