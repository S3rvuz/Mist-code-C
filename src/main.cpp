#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <MCCAB_Lib.h>

#define LED_PIN 5
#define LED_PIN2 8
#define POTI1_PIN A6
#define POTI2_PIN A7
#define V_REF 3.3
#define STEPS 1024.0
#define VOLT_POS 0
#define PAUSE 30
#define BUZZER_PIN 9
#define PERIOD 1000
#define DURATION_TONE 1000
#define PAUSE_TONE 1000
#define TONE1 0
#define PAUSE_TONE1 1
#define INTERVAL_PAUSE 500

#define LED_PIN3 2

#define POTI_OHM 4300.0


uint8_t thisState = TONE1;
uint32_t counter;
uint32_t deltaTime;
uint32_t deltaPeriod;
uint32_t tonePeriod = 0;
uint32_t toneTimeout = 0;

KeySwitch KeyOne(S1_DEFAULT_PIN, ACTIVE_HIGH);
KeySwitch KeyTwo(S2_DEFAULT_PIN, ACTIVE_HIGH);
KeySwitch KeyThree(S3_DEFAULT_PIN, ACTIVE_HIGH);
KeySwitch KeyFour(S4_DEFAULT_PIN, ACTIVE_HIGH);
KeySwitch KeyFive(S5_DEFAULT_PIN, ACTIVE_HIGH);
KeySwitch KeySix(S6_DEFAULT_PIN, ACTIVE_HIGH);

uint8_t screen = 0; 

LED LedA(LED_PIN2);
Sound Horn;
//struct tone_t tones[] = {{ 1000, 5, 5}, { 587, 5, 5}, { 0, 0, 0}};


bool enabled;

void alarmSound() {
  static bool high = false;

  if (high) {
    tone(BUZZER_PIN, 2000);
  } else {
    tone(BUZZER_PIN, 1000);
  }

  high = !high;
}
void switchedOn() {
  //LedA.blink(F_1_5_HZ);
  enabled = true;

}
void switchedOff() {
  //LedA.off();
  noTone(BUZZER_PIN);
  enabled = false;
}
void Alarmbeleuchtung() {

  for(int i = 0; i <= 12; i++) {
    digitalWrite(i, HIGH);
    pinMode(i, OUTPUT);
    delay(10);

    if (i == 12) {
      for(int j = 12; j >= 0; j--) {
        digitalWrite(j, LOW);
        delay(10);
      }
    }

  }
}

LiquidCrystal_I2C lcd(0x27, 16, 2);

  float displayVoltage(uint16_t adcValue, uint8_t row) {
  float voltageFloat;
  voltageFloat = adcValue * V_REF / STEPS;
  lcd.setCursor(VOLT_POS, row);
  lcd.print("U = " + String(voltageFloat, 3) + " Vo");
  return voltageFloat;
} 


float displayResistance(uint16_t adcValue, uint8_t row) {
  
  float resistanceFloat;
  
  resistanceFloat = (adcValue / 1023.0) * POTI_OHM;

  if (resistanceFloat > POTI_OHM) {
  resistanceFloat = POTI_OHM;
}
  lcd.setCursor(VOLT_POS, row);
  lcd.print("R = " + String(resistanceFloat / 1000.0, 3) + " KOhm");
  return resistanceFloat;
}

float displayCurrent(float voltage, float resistance, uint8_t row) {
  float currentFloat;
  if (resistance > 0) {
    currentFloat = voltage / resistance;
  } else {
    currentFloat = 0; 
  }
  lcd.setCursor(VOLT_POS, row);
  lcd.print("I = " + String(currentFloat * 1000.0, 3) + " mA ");
  return currentFloat;
}

void Alarm(float u1, float u2) {
  
  if (u1 > 2.5 || u2 > 2.5) {
  digitalWrite(LED_PIN, HIGH);
  switchedOn();
  
} else {
  digitalWrite(LED_PIN, LOW);
  switchedOff();
}
  }


  void Menu() {
     
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Menu:");
    lcd.setCursor(0, 1);
    lcd.print("S1=L S2=R S3=I");
  
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  analogReference(DEFAULT);
  lcd.init();
  lcd.backlight();

  Menu();
}


void loop() {

    if (KeyOne.isOn()) {
    screen = 1;
    lcd.clear();
  }

  if (KeyTwo.isOn()) {
    screen = 2;
    lcd.clear();
  }
  if (KeyThree.isOn()) {
    screen = 3;
    lcd.clear();
  }

  float u1 = analogRead(POTI1_PIN) * V_REF / STEPS;
  float u2 = analogRead(POTI2_PIN) * V_REF / STEPS;

  
if (screen == 1) {
  Alarm(u1, u2);
  displayVoltage(analogRead(POTI2_PIN), 0);
  displayResistance(analogRead(POTI1_PIN), 1);
  //displayVoltage(analogRead(POTI1_PIN), 1);
}

if (screen == 2) {

  displayVoltage(analogRead(POTI1_PIN), 0);
  displayResistance(analogRead(POTI2_PIN), 1);
  
  //displayResistance(analogRead(POTI1_PIN), 1);
}

if (screen == 3) {
  uint16_t adcU1 = analogRead(POTI1_PIN);
  uint16_t adcR1 = analogRead(POTI2_PIN);

  uint16_t adcU2 = analogRead(POTI2_PIN);
  uint16_t adcR2 = analogRead(POTI1_PIN);

  float voltage1 = adcU1 * V_REF / STEPS;
  float resistance1 = (adcR1 / 1023.0) * POTI_OHM;

  float voltage2 = adcU2 * V_REF / STEPS;
  float resistance2 = (adcR2 / 1023.0) * POTI_OHM;

  displayCurrent(voltage1, resistance1, 0);
  lcd.print("IUR");
  displayCurrent(voltage2, resistance2, 1);
  lcd.print("IRU");
}
  

  if (enabled) {
    alarmSound();
    //Alarmbeleuchtung();
  } 



  delay(PAUSE);
}


 
    
