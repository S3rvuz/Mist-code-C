#include <Arduino.h>
#include <LiquidCrystal_I2C.h> //
#include <MCCAB_Lib.h>


#define LED_PIN 5 //LED ansteuern 5
#define LED_PIN2 8 //LED ansteuern 8
#define POTI1_PIN A6 //Poti Links
#define POTI2_PIN A7 //Poti Rechts
#define V_REF 3.3 //Standardvolt auf 3,3V eingestellt
#define STEPS 1024.0 //10 Bit ADC, also 2^10 = 1024 Schritte -> siehe Buch
#define VOLT_POS 0 //Position des Spannungswerts auf dem Display
#define PAUSE 300 //Aktualisierungsrate
#define BUZZER_PIN 9 //Piep Piep
#define PERIOD 1000 //Periode des Tons in ms
#define DURATION_TONE 1000 //Dauer des Tons in ms
#define PAUSE_TONE 1000 //Pause zwischen den Tönen in ms
/*#define TONE1 0 
#define PAUSE_TONE1 1 //
#define INTERVAL_PAUSE 500*/

#define LED_PIN3 2 //LED ansteuern 2

#define POTI_OHM 4300.0 //vorher gemessener max Widerstand des Arduinos


/*uint8_t thisState = TONE1;
uint32_t counter;
uint32_t deltaTime;
uint32_t deltaPeriod;
uint32_t tonePeriod = 0;
uint32_t toneTimeout = 0;*/

KeySwitch KeyOne(S1_DEFAULT_PIN, ACTIVE_HIGH); //S1_DEFAULT_PIN = 2, S2_DEFAULT_PIN = 3, S3_DEFAULT_PIN = 4, S4_DEFAULT_PIN = 5, S5_DEFAULT_PIN = 6, S6_DEFAULT_PIN = 7
KeySwitch KeyTwo(S2_DEFAULT_PIN, ACTIVE_HIGH);
KeySwitch KeyThree(S3_DEFAULT_PIN, ACTIVE_HIGH);
KeySwitch KeyFour(S4_DEFAULT_PIN, ACTIVE_HIGH);
KeySwitch KeyFive(S5_DEFAULT_PIN, ACTIVE_HIGH);
KeySwitch KeySix(S6_DEFAULT_PIN, ACTIVE_HIGH);

uint8_t screen = 0; //0 = Menu, 1 = Spannung, 2 = Widerstand, 3 = Strom

/*LED LedA(LED_PIN2);
Sound Horn;
struct tone_t tones[] = {{ 1000, 5, 5}, { 587, 5, 5}, { 0, 0, 0}};*/


bool enabled; //Alarm aktiviert oder nicht

void alarmSound() { //Methode für den Alarmton, der zwischen 1000 Hz und 2000 Hz wechselt, um einen Warnton zu erzeugen
  static bool high = false; //Variable, um zwischen den beiden Frequenzen zu wechseln

  if (high) { //Wenn high true ist, wird der Ton mit 2000 Hz erzeugt
    tone(BUZZER_PIN, 2000); //Buzzer an Pin 9 mit 2000 Hz
  } else { //Wenn high false ist, wird der Ton mit 1000 Hz erzeugt
    tone(BUZZER_PIN, 1000); //Buzzer an Pin 9 mit 1000 Hz
  }

  high = !high; //Wechselt den Wert von high, damit beim nächsten Aufruf der Methode die andere Frequenz erzeugt wird
}

void Alarmbeleuchtung() { //Methode, um die Alarmbeleuchtung zu steuern, die alle 13 LEDs nacheinander einschaltet und dann wieder ausschaltet

  for(int i = 0; i <= 12; i++) { //Schleife, um die LEDs von 0 bis 12 anzusteuern
    digitalWrite(i, HIGH); //Schaltet die LED an Pin i ein
    pinMode(i, OUTPUT); //Setzt den Pin i als Ausgang, damit die LED angesteuert werden kann
    delay(10); //Kurze Verzögerung, damit die LEDs nacheinander eingeschaltet werden 10 ms

    if (i == 12) { //Wenn die letzte LED (Pin 12) eingeschaltet ist, werden alle LEDs wieder ausgeschaltet
      for(int j = 12; j >= 0; j--) { //Schleife, um die LEDs von 12 bis 0 auszuschalten
        digitalWrite(j, LOW); //Schaltet die LED an Pin j aus
        delay(10); //Kurze Verzögerung, damit die LEDs nacheinander ausgeschaltet werden 10 ms
      }
    }

  }
}
void switchedOn() { //Methode, um den Alarm zu aktivieren, die LED einzuschalten und den Warnton zu starten
  //LedA.blink(F_1_5_HZ);
  enabled = true; //Setzt enabled auf true, damit der Alarmton in der loop() Methode abgespielt wird
  Alarmbeleuchtung(); //Ruft die Methode Alarmbeleuchtung() auf, um die Alarmbeleuchtung zu steuern, die alle 13 LEDs nacheinander einschaltet und dann wieder ausschaltet
}
void switchedOff() { //Methode, um den Alarm zu deaktivieren, die LED auszuschalten und den Warnton zu stoppen
  //LedA.off();
  noTone(BUZZER_PIN); //Stoppt den Ton am Buzzer-Pin
  enabled = false; //Setzt enabled auf false, damit der Alarmton in der loop() Methode nicht mehr abgespielt wird
}

LiquidCrystal_I2C lcd(0x27, 16, 2); //Adresse des LCD-Displays (0x27) und die Anzahl der Spalten (16) und Zeilen (2)

  float displayVoltage(uint16_t adcValue, uint8_t row) { //Methode, um die Spannung auf dem Display anzuzeigen
  float voltageFloat; //Variable, um die berechnete Spannung zu speichern
  voltageFloat = adcValue * V_REF / STEPS; //Berechnet die Spannung aus dem ADC-Wert, der Referenzspannung und der Anzahl der Schritte (10 Bit ADC = 1024 Schritte)
  lcd.setCursor(VOLT_POS, row); //Setzt den Cursor auf die Position VOLT_POS (0) und die angegebene Zeile (row), damit die Spannung an der richtigen Stelle auf dem Display angezeigt wird
  lcd.print("U = " + String(voltageFloat, 3) + " V"); //Gibt die Spannung mit 3 Nachkommastellen und der Einheit "V" (Volt) auf dem Display aus
  return voltageFloat; //Gibt die berechnete Spannung als Rückgabewert der Methode zurück, damit sie in anderen Methoden (z.B. displayCurrent) verwendet werden kann
} 


float displayResistance(uint16_t adcValue, uint8_t row) { //Methode, um den Widerstand auf dem Display anzuzeigen
  
  float resistanceFloat; //Variable, um den berechneten Widerstand zu speichern
  
  resistanceFloat = (adcValue / 1023.0) * POTI_OHM; //Berechnet den Widerstand aus dem ADC-Wert, 
  // der Anzahl der Schritte (10 Bit ADC = 1024 Schritte) und dem maximalen Widerstand des Potentiometers (POTI_OHM), der vorher gemessen wurde. 
  // Der ADC-Wert wird durch 1023 geteilt, um ihn in einen Bereich von 0 bis 1 zu bringen, und dann mit POTI_OHM multipliziert, um den tatsächlichen Widerstand zu erhalten.

  if (resistanceFloat > POTI_OHM) { //Falls der berechnete Widerstand größer als der maximale Widerstand des Potentiometers ist, wird er auf den maximalen Wert begrenzt, um unrealistische Werte zu vermeiden.
  resistanceFloat = POTI_OHM; //Setzt den Widerstand auf den maximalen Wert von POTI_OHM, wenn er größer ist, um unrealistische Werte zu vermeiden.
}
  lcd.setCursor(VOLT_POS, row); //Setzt den Cursor auf die Position VOLT_POS (0) und die angegebene Zeile (row), damit der Widerstand an der richtigen Stelle auf dem Display angezeigt wird
  lcd.print("R = " + String(resistanceFloat / 1000.0, 3) + " KOhm"); //Gibt den Widerstand mit 3 Nachkommastellen und der Einheit "KOhm" (Kiloohm) auf dem Display aus. Der Widerstand wird durch 1000 geteilt, um ihn in Kiloohm umzuwandeln, da die Werte in Ohm zu groß sein könnten, um sie übersichtlich anzuzeigen.
  return resistanceFloat; //Gibt den berechneten Wiserstand zurück.
}

float displayCurrent(float voltage, float resistance, uint8_t row) { //Methode, um den Strom anzuzeigen.
  float currentFloat; //Variable, um den berechneten Strom zu speichern. 
  if (resistance > 0) { //Falls der Widerstand größer als 0 ist, wird der Strom berechnet, um eine Division durch Null zu vermeiden.
    currentFloat = voltage / resistance; //Berechnet den Strom aus der Spannung und dem Widerstand, (Ohmsches Gesetz: I = U / R).
  } else { 
    currentFloat = 0; //Setzt den Strom auf 0, wenn der Widerstand 0 oder negativ ist, um unrealistische Werte zu vermeiden.
  }
  lcd.setCursor(VOLT_POS, row); //Setzt den Cursor auf die Position VOLT_POS (0) und die angegebene Zeile (row).
  lcd.print("I = " + String(currentFloat * 1000.0, 3) + " mA "); //Gibt den Strom mit 3 Nachkommastellen und der Einheit "mA" (Milliampere) auf dem Display aus.
  return currentFloat; //Gibt den berechneten Strom zurück.
}

void Alarm(float u1, float u2) { //Methode, um den Alarm zu steuern.
  
  if (u1 > 2.5 || u2 > 2.5) { //Wenn die Spannung u1 oder u2 größer als 2,5 V ist, wird der Alarm aktiviert.
  //digitalWrite(LED_PIN, HIGH); //Schaltet die LED an Pin 5 ein, um den Alarm visuell anzuzeigen.
  switchedOn(); //Ruft die Methode switchedOn() auf, um den Alarm zu aktivieren, die LED einzuschalten und den Warnton zu starten.
  
} else {
  //digitalWrite(LED_PIN, LOW); 
  switchedOff(); //Ruft die Methode switchedOff() auf, um den Alarm zu deaktivieren, die LED auszuschalten und den Warnton zu stoppen.
}
  }


  void Menu() {
     
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("     Menu :");
    lcd.setCursor(0, 1);
    lcd.print(" S1=L S2=R S3=I");
  
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
  Alarm(u1, u2);
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


 
    
