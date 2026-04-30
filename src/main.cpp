#include <Arduino.h> //Einbindung der Arduino-Bibliothek, die grundlegende Funktionen und Definitionen für die Programmierung von Arduino-Boards bereitstellt.
#include <LiquidCrystal_I2C.h>  //Einbindung der LiquidCrystal_I2C-Bibliothek, die Funktionen für die Ansteuerung von LCD-Displays mit I2C-Schnittstelle bereitstellt.
#include <MCCAB_Lib.h> //Einbindung der MCCAB_Lib-Bibliothek, die Funktionen für die Ansteuerung von Tasten und LEDs bereitstellt, um die Interaktion mit dem Benutzer zu ermöglichen.


#define LED_PIN 5 //LED ansteuern 5
#define LED_PIN2 8 //LED ansteuern 8
#define POTI1_PIN A6 //Poti Links
#define POTI2_PIN A7 //Poti Rechts
#define V_REF 3.3 //Standardvolt auf 3,3V eingestellt
#define STEPS 1024.0 //10 Bit ADC, also 2^10 = 1024 Schritte -> siehe Buch
#define VOLT_POS 0 //Position des Spannungswerts auf dem Display
#define PAUSE 30 //Aktualisierungsrate des Displays und des Alarms in ms
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

int alarmLeds[] = {2, 3, 4, 5, 6, 7, 8, 10, 11, 12}; //Array mit den Pins der LEDs, die für die Alarmbeleuchtung verwendet werden.
const int alarmLedCount = 10; //Anzahl der LEDs in dem Array, damit die Schleife in der Methode Alarmbeleuchtung() weiß, wie viele LEDs sie ansteuern soll.

void Alarmbeleuchtung() { //Methode, um die Alarmbeleuchtung zu steuern.
  for (int i = 0; i < alarmLedCount; i++) { //Schleife, die von 0 bis alarmLedCount (10) läuft, um alle LEDs im Array anzusteuern.
    pinMode(alarmLeds[i], OUTPUT); //Setzt den Pin der aktuellen LED im Array als Ausgang, damit sie angesteuert werden kann.
    digitalWrite(alarmLeds[i], HIGH); //Schaltet die aktuelle LED im Array ein, um die Alarmbeleuchtung zu aktivieren.
    delay(10); //Verzögert die Schleife um 10 ms.
  }

  for (int i = alarmLedCount - 1; i >= 0; i--) { //Schleife, die von alarmLedCount - 1 (9) bis 0 läuft, um alle LEDs im Array in umgekehrter Reihenfolge auszuschalten.
    digitalWrite(alarmLeds[i], LOW); //Schaltet die aktuelle LED im Array aus, um die Alarmbeleuchtung zu deaktivieren.
    delay(10); //Verzögert die Schleife um 10 ms, damit die LEDs nacheinander ausgeschaltet werden.
  }
}

void switchedOn() { //Methode, um den Alarm zu aktivieren, die LED einzuschalten und den Warnton zu starten
  //LedA.blink(F_1_5_HZ);
  enabled = true; //Setzt enabled auf true, damit der Alarmton in der loop() Methode abgespielt wird
   //Ruft die Methode Alarmbeleuchtung() auf, um die Alarmbeleuchtung zu steuern, die alle 13 LEDs nacheinander einschaltet und dann wieder ausschaltet
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

void alarmAusloesen(float u) { //Methode, um den Alarm auszulösen, wenn die Spannung u größer als 2,5 V ist.
  if (u > 2.5) { //Wenn die Spannung u größer als 2,5 V ist, wird der Alarm aktiviert.
    switchedOn(); //Ruft die Methode switchedOn() auf, um den Alarm zu aktivieren, die LED einzuschalten und den Warnton zu starten.
  } else {
    switchedOff(); //Ruft die Methode switchedOff() auf, um den Alarm zu deaktivieren, die LED auszuschalten und den Warnton zu stoppen.
  }
}


  void Menu() { //Methode, um das Menü auf dem LCD-Display anzuzeigen.
     
    lcd.clear(); //Löscht den Inhalt des LCD-Displays, um das Menü anzuzeigen.
    lcd.setCursor(0, 0); //Setzt den Cursor auf die Position (0, 0), um die Menüüberschrift an der ersten Zeile und ersten Spalte des Displays anzuzeigen.
    lcd.print("     Menu :"); //Gibt die Menüüberschrift "Menu :" auf dem LCD-Display aus, zentriert in der ersten Zeile.
    lcd.setCursor(0, 1); //Setzt den Cursor auf die Position (0, 1), um die Anweisungen für die Tasten S1, S2 und S3 in der zweiten Zeile des Displays anzuzeigen.
    lcd.print(" S1=L S2=R S3=I"); //Gibt die Anweisungen "S1=L S2=R S3=I" auf dem LCD-Display aus, um dem Benutzer zu zeigen, welche Tasten er drücken muss.
  
}

void setup() { //Initialisierung der Pins, des LCD-Displays und des Menüs
  pinMode(LED_PIN, OUTPUT); //Setzt den Pin der LED (Pin 5) als Ausgang, damit er angesteuert werden kann.
  analogReference(DEFAULT); //Setzt die Referenzspannung für die analogen Eingänge auf den Standardwert (5 V oder 3,3 V. Je nach Arduino-Modell).
  lcd.init(); //Initialisiert das LCD-Display, damit es bereit ist, Daten anzuzeigen.
  lcd.backlight(); //Schaltet die Hintergrundbeleuchtung des LCD-Displays ein, damit die angezeigten Informationen besser sichtbar sind.

  Menu(); //Ruft die Methode Menu() auf, um das Menü auf dem LCD-Display anzuzeigen.
}


void loop() { //Hauptschleife.

    if (KeyOne.isOn()) { //Wenn die Taste S1 (KeyOne) gedrückt wird, wird der Bildschirm auf 1 gesetzt.
    screen = 1; //Setzt die Variable screen auf 1, um anzuzeigen, dass der Bildschirm für die Spannung angezeigt werden soll.
    lcd.clear(); //Löscht den Inhalt des LCD-Displays, um Platz für die neuen Informationen zu schaffen, die auf dem Bildschirm 1 angezeigt werden sollen.
  }

  if (KeyTwo.isOn()) { //Wenn die Taste S2 (KeyTwo) gedrückt wird, wird der Bildschirm auf 2 gesetzt.
    screen = 2; //Setzt die Variable screen auf 2, um anzuzeigen, dass der Bildschirm für den Widerstand angezeigt werden soll.
    lcd.clear(); //Löscht den Inhalt des LCD-Displays, um Platz für die neuen Informationen zu schaffen, die auf dem Bildschirm 2 angezeigt werden sollen.
  }
  if (KeyThree.isOn()) { //Wenn die Taste S3 (KeyThree) gedrückt wird, wird der Bildschirm auf 3 gesetzt.
    screen = 3; //Setzt die Variable screen auf 3, um anzuzeigen, dass der Bildschirm für den Strom angezeigt werden soll.
    lcd.clear(); //Löscht den Inhalt des LCD-Displays, um Platz für die neuen Informationen zu schaffen, die auf dem Bildschirm 3 angezeigt werden sollen.
  }

  float u1 = analogRead(POTI1_PIN) * V_REF / STEPS; //Liess den ADC-Wert von POTI1_PIN (Poti Links) aus, multipliziert ihn mit der Referenzspannung (V_REF) und teilt ihn durch die Anzahl der Schritte (STEPS), um die Spannung u1 zu berechnen.
  float u2 = analogRead(POTI2_PIN) * V_REF / STEPS; //Liess den ADC-Wert von POTI2_PIN (Poti Rechts) aus, multipliziert ihn mit der Referenzspannung (V_REF) und teilt ihn durch die Anzahl der Schritte (STEPS), um die Spannung u2 zu berechnen.

  
if (screen == 1) { //Wenn der Bildschirm 1 (Spannung) angezeigt wird, werden die Spannung von POTI2_PIN (Poti Rechts) angezeigt und die Alarmbedingung überprüft.
  
  float u = displayVoltage(analogRead(POTI2_PIN), 0); //Ruft die Methode displayVoltage() auf, um die Spannung von POTI2_PIN (Poti Rechts) anzuzeigen. 
  displayResistance(analogRead(POTI1_PIN), 1); //Ruft die Methode displayResistance() auf, um den Widerstand von POTI1_PIN (Poti Links) anzuzeigen. 
  //displayVoltage(analogRead(POTI1_PIN), 1);

  alarmAusloesen(u); //Ruft die Methode alarmAusloesen() auf, um den Alarm auszulösen.
}

if (screen == 2) { //Wenn der Bildschirm 2 (Widerstand) angezeigt wird, werden der Widerstand von POTI2_PIN (Poti Rechts) angezeigt und die Alarmbedingung überprüft.

  float u = displayVoltage(analogRead(POTI1_PIN), 0); //Ruft die Methode displayVoltage() auf, um die Spannung von POTI1_PIN (Poti Links) anzuzeigen.
  displayResistance(analogRead(POTI2_PIN), 1); //Ruft die Methode displayResistance() auf, um den Widerstand von POTI2_PIN (Poti Rechts) anzuzeigen.

  alarmAusloesen(u); //Ruft die Methode alarmAusloesen() auf, um den Alarm auszulösen.
  
  //displayResistance(analogRead(POTI1_PIN), 1);
}

if (screen == 3) { //Wenn der Bildschirm 3 (Strom) angezeigt wird, werden die Spannung von POTI1_PIN (Poti Links) und der Widerstand von POTI2_PIN (Poti Rechts) angezeigt, um den Strom zu berechnen und anzuzeigen.
  uint16_t adcU1 = analogRead(POTI1_PIN); //Liest den ADC-Wert von POTI1_PIN (Poti Links) aus und speichert ihn in der Variable adcU1, um die Spannung u1 zu berechnen und anzuzeigen.
  uint16_t adcR1 = analogRead(POTI2_PIN); //Liest den ADC-Wert von POTI2_PIN (Poti Rechts) aus und speichert ihn in der Variable adcR1, um den Widerstand r1 zu berechnen und anzuzeigen.

  uint16_t adcU2 = analogRead(POTI2_PIN); //Liest den ADC-Wert von POTI2_PIN (Poti Rechts) aus und speichert ihn in der Variable adcU2, um die Spannung u2 zu berechnen und anzuzeigen.
  uint16_t adcR2 = analogRead(POTI1_PIN); //Liest den ADC-Wert von POTI1_PIN (Poti Links) aus und speichert ihn in der Variable adcR2, um den Widerstand r2 zu berechnen und anzuzeigen.

  float voltage1 = adcU1 * V_REF / STEPS; //Berechnet die Spannung u1 
  float resistance1 = (adcR1 / 1023.0) * POTI_OHM; //Berechnet den Widerstand r1.

  float voltage2 = adcU2 * V_REF / STEPS; //Berechnet die Spannung u2
  float resistance2 = (adcR2 / 1023.0) * POTI_OHM; //Berechnet den Widerstand r2.

  displayCurrent(voltage1, resistance1, 0); //Ruft die Methode displayCurrent() auf, um den Strom anzuzeigen.
  lcd.print("IUR"); //Gibt die Einheit "IUR" (Strom in Ampere) auf dem LCD-Display aus, um dem Benutzer zu zeigen, dass der angezeigte Wert der Strom ist.
  displayCurrent(voltage2, resistance2, 1); //Ruft die Methode displayCurrent() auf, um den Strom anzuzeigen.
  lcd.print("IRU"); //Gibt die Einheit "IRU" (Strom in Ampere) auf dem LCD-Display aus, um dem Benutzer zu zeigen, dass der angezeigte Wert der Strom ist.
}
  

  if (enabled) { //Wenn der Alarm aktiviert ist (enabled ist true). 
    alarmSound(); //wird die Methode alarmSound() aufgerufen.
    Alarmbeleuchtung(); //und die Methode Alarmbeleuchtung() aufgerufen.
  } 



  delay(PAUSE); //Verzögert die Hauptschleife um die in der Konstante PAUSE (300 ms) festgelegte Zeit, um die Aktualisierungsrate des Displays und des Alarms zu steuern.
}


 
    
