/*
 * MCCAB_Lib.cpp
 * Rev. 3.3
 *
 *  Created on: 20.12.2021
 *      Author: Wolfgang Trampert
 *
 * History
 * Rev. 3.3
 * The released version of the preliminary draft version 0.16 suitable for MCCAB Trainingsboard 3.3
 */

#include "MCCAB_Lib.h"


/***************************************************************************************************************/
/*                                         Constants in Program Memory                                         */
/***************************************************************************************************************/

const uint8_t PROGMEM flashingFrequency_PGM[] = {
  (uint8_t) F_0_5,
  (uint8_t) F_0_75,
  (uint8_t) F_1,
  (uint8_t) F_1_5,
  (uint8_t) F_2,
  (uint8_t) F_3,
  (uint8_t) F_4,
  (uint8_t) F_5,
  (uint8_t) F_6,
  (uint8_t) F_8,
  (uint8_t) F_10,
  (uint8_t) F_16
};


/***************************************************************************************************************/
/*                                               Global variables                                              */
/***************************************************************************************************************/

volatile struct _isr_t _isr;  // global struct variable for the Interrupt-Handling


/***************************************************************************************************************/
/*                                               Global functions                                              */
/***************************************************************************************************************/

// Function that is performed on the T/C2 Overflow Interrupt
ISR(TIMER2_OVF_vect) {
  static uint8_t stateCounter;
  uint8_t stateCnt = stateCounter;
  if (stateCnt >= 4) {
    if (stateCnt >= 6) {
      if (stateCnt == 7) {    
        /*********************************   Output of matrix line 3   *********************************/
        if (_isr.matrixPtr != nullptr) {
          uint8_t pattern = highByte(_isr.matrixPattern);
          // Row 2 = off, Row 1 = off, Row 3 = active LOW
          SELECT_ROW(_isr.matrixPtr->mxRow[R2].ioPort, _isr.matrixPtr->mxRow[R2].ioMask,
                     _isr.matrixPtr->mxRow[R1].ioPort, _isr.matrixPtr->mxRow[R1].ioMask,
                     _isr.matrixPtr->mxRow[R3].ioPort, _isr.matrixPtr->mxRow[R3].ioMask);
          // Set column A to HIGH or alternatively LOW
          SET_COL(pattern,A3_UPPER, _isr.matrixPtr->mxCol[CA].ioPort, _isr.matrixPtr->mxCol[CA].ioMask)
          pattern = lowByte(_isr.matrixPattern);  // get the low byte of the pattern
          // Set column B to HIGH or alternatively LOW
          SET_COL(pattern,B3_MASK, _isr.matrixPtr->mxCol[CB].ioPort, _isr.matrixPtr->mxCol[CB].ioMask)
          // Set column C to HIGH or alternatively LOW
          SET_COL(pattern,C3_MASK, _isr.matrixPtr->mxCol[CC].ioPort, _isr.matrixPtr->mxCol[CC].ioMask)
        }
      }
      else {  // stateCnt == 6
        /*********************************   Flashing of LEDs D13 ... D8   ********************************/
        if (_isr.flashingHigh & LEDS_MASK) {  // if at least one of the LEDs D13 ... D8 flashes
          for (uint8_t i = 0, idx = 6, mask = 0b1; i < 6; i++, idx++, mask<<=1) {
            if (_isr.flashingHigh & mask) {  // get the flashing status of the LED: 1 = flashing
              uint8_t counter = _isr.ledPtr[idx]->flash.freqCounter;
              counter++;
              if (counter >= _isr.ledPtr[idx]->flash.freqNominal) {
                counter = 0;  // reset in case of overflow
                PORTB ^= mask;  // toggle LED
              }
              _isr.ledPtr[idx]->flash.freqCounter = counter;
            }
          } // end for
        }
      }
    }
    else {    // 4 <= stateCnt < 6
      if (stateCnt == 5) {    
        /*********************************   Flashing of LEDs D7 ... D2    ********************************/
        uint8_t flashing = _isr.flashingLow;
        if (flashing & LEDS_MASK) {  // if at least one of the LEDs D7 ... D2 flashes
          for (uint8_t i = 0, mask = 0b1, bitPos = 0b100; i < 6; i++, mask<<=1, bitPos<<=1) {
            if (flashing & mask) {  // get the flashing status of the LED: 1 = flashing
              uint8_t counter = _isr.ledPtr[i]->flash.freqCounter;
              counter++;
              if (counter >= _isr.ledPtr[i]->flash.freqNominal) {
                counter = 0;  // reset in case of overflow
                PORTD ^= bitPos;  // toggle LED
              }
              _isr.ledPtr[i]->flash.freqCounter = counter;
            }
          } // end for
        }
      }
      else {  // stateCnt == 4
        /*********************************   Output of matrix line 2   *********************************/
        if (_isr.matrixPtr != nullptr) {
          uint8_t pattern = lowByte(_isr.matrixPattern);
          // Row 3 = off, Row 1 = off, Row 2 = active LOW
          SELECT_ROW(_isr.matrixPtr->mxRow[R3].ioPort, _isr.matrixPtr->mxRow[R3].ioMask,
                     _isr.matrixPtr->mxRow[R1].ioPort, _isr.matrixPtr->mxRow[R1].ioMask,
                     _isr.matrixPtr->mxRow[R2].ioPort, _isr.matrixPtr->mxRow[R2].ioMask);
          // Set column A to HIGH or alternatively LOW
          SET_COL(pattern,A2_MASK, _isr.matrixPtr->mxCol[CA].ioPort, _isr.matrixPtr->mxCol[CA].ioMask)
          // Set column B to HIGH or alternatively LOW
          SET_COL(pattern,B2_MASK, _isr.matrixPtr->mxCol[CB].ioPort, _isr.matrixPtr->mxCol[CB].ioMask)
          // Set column C to HIGH or alternatively LOW
          SET_COL(pattern,C2_MASK, _isr.matrixPtr->mxCol[CC].ioPort, _isr.matrixPtr->mxCol[CC].ioMask)
        }
      }
    }
  }
  else {    // 0 <= stateCnt < 4
    if (stateCnt >= 2) {
      if (stateCnt == 3) {
        /**************************************   Read switches S7 ... S3   ************************************/
        uint8_t instances = _isr.instancedKeys;
        if (instances) {
          uint8_t states = _isr.keysStates;
          for (uint8_t i = 3, mask = 0b1000; i < 8; i++, mask <<= 1) {
            if (_isr.keyPtr[i] != nullptr) {
              bool keyNow = ((*_isr.keyPtr[i]->ioKey.ioPort & _isr.keyPtr[i]->ioKey.ioMask) != 0);  // read state of key
              if (_isr.keyPtr[i]->activeState == ACTIVE_LOW) keyNow = !keyNow;  // invert level if active LOW
              if (keyNow) states |= mask;  // save the determined status of the key
              else states &= ~mask;
              // has the key status changed since the last query?
              if (keyNow != ((_isr.keysPrevious & mask) != 0)) {
                _isr.keysPrevious ^= mask;  // invert the assigned last key state
                if (keyNow) {
                  if (_isr.keyPtr[i]->keyClosed != nullptr) _isr.keyPtr[i]->keyClosed();  // execute the assigned function on closing
                }
                else {
                  if (_isr.keyPtr[i]->keyOpened != nullptr) _isr.keyPtr[i]->keyOpened();  // execute the assigned function on opening
                }
              }
            }
          } // end for
          _isr.keysStates = states;
        }
      }
      else {  // stateCnt == 2
        /**************************************   Read switches S2 ... S0   ************************************/
        uint8_t instances = _isr.instancedKeys;
        if (instances) {
          uint8_t states = _isr.keysStates;
          for (uint8_t i = 0, mask = 0b1; i < 3; i++, mask <<= 1) {
            if (_isr.keyPtr[i] != nullptr) {
              bool keyNow = ((*_isr.keyPtr[i]->ioKey.ioPort & _isr.keyPtr[i]->ioKey.ioMask) != 0);  // read state of key
              if (_isr.keyPtr[i]->activeState == ACTIVE_LOW) keyNow = !keyNow;  // invert level if active LOW
              if (keyNow) states |= mask;  // save the determined status of the key
              else states &= ~mask;
              // has the key status changed since the last query?
              if (keyNow != ((_isr.keysPrevious & mask) != 0)) {
                _isr.keysPrevious ^= mask;  // invert the assigned last key state
                if (keyNow) {
                  if (_isr.keyPtr[i]->keyClosed != nullptr) _isr.keyPtr[i]->keyClosed();  // execute the assigned function on closing
                }
                else {
                  if (_isr.keyPtr[i]->keyOpened != nullptr) _isr.keyPtr[i]->keyOpened();  // execute the assigned function on opening
                }
              }
            }
          } // end for
          _isr.keysStates = states;
        }
      }
    }
    else {    // 0 <= stateCnt < 1
      if (stateCnt == 1) {
        /*********************************   Output of matrix line 1   *********************************/
        if (_isr.matrixPtr != nullptr) {
          uint8_t pattern = lowByte(_isr.matrixPattern);
          // Row 3 = off, Row 2 = off, Row 1 = active LOW
          SELECT_ROW(_isr.matrixPtr->mxRow[R3].ioPort, _isr.matrixPtr->mxRow[R3].ioMask,
                     _isr.matrixPtr->mxRow[R2].ioPort, _isr.matrixPtr->mxRow[R2].ioMask,
                     _isr.matrixPtr->mxRow[R1].ioPort, _isr.matrixPtr->mxRow[R1].ioMask);
          // Set column A to HIGH or alternatively LOW
          SET_COL(pattern,A1_MASK, _isr.matrixPtr->mxCol[CA].ioPort, _isr.matrixPtr->mxCol[CA].ioMask)
          // Set column B to HIGH or alternatively LOW
          SET_COL(pattern,B1_MASK, _isr.matrixPtr->mxCol[CB].ioPort, _isr.matrixPtr->mxCol[CB].ioMask)
          // Set column C to HIGH or alternatively LOW
          SET_COL(pattern,C1_MASK, _isr.matrixPtr->mxCol[CC].ioPort, _isr.matrixPtr->mxCol[CC].ioMask)
        }
      }
      else {  // stateCnt == 0
        /****************************************   Tone Generator   **************************************/
        if (_isr.soundPtr != nullptr) {
          uint8_t pulseRemaining;
          struct tone_t * ptr;  
          if (_isr.soundPtr->_state == PULSE) {
            pulseRemaining = _isr.soundPtr->_pulseRemaining;  // the remaining duration of the tone
            if (pulseRemaining) pulseRemaining--;
            else {
              TCCR1B &= ~0b111;  // stop T/C1 
              // clear the sound output if set
              if (PINB & SOUND_BITMASK) TCCR1C |= (1 << FOC1A);  // force an immediate compare match to toggle the output
              _isr.soundPtr->_state = NO_TONE;  // the pulse duration has expired
            }
            _isr.soundPtr->_pulseRemaining = pulseRemaining;
          }          
          else if (_isr.soundPtr->_state == MELODY) {
            ptr = _isr.soundPtr->_ptrProperties;
            if (ptr != nullptr) {
              if (ptr->frequency) {
                    Serial.println(ptr->frequency);
                if (!_isr.tonePlaying) {
                  pulseRemaining = ptr->duration;
                  _isr.toneDuration = ((pulseRemaining > 254 ? 254 : pulseRemaining) * 100) >> 4;  // the interrupt expires every 16 milliseconds
                  pulseRemaining = ptr->pause;
                  _isr.tonePause = ((pulseRemaining > 254 ? 254 : pulseRemaining) * 100) >> 4;  // the interrupt expires every 16 milliseconds
                  _isr.tonePlaying = true;
                    Serial.println(_isr.toneDuration);
                    Serial.println(_isr.tonePause);
                    Serial.println(_isr.tonePlaying );
                }
                if (_isr.toneDuration) {
                  if (TCCR1B & 0b111) _isr.toneDuration--;  // decrement, if T/C1 is already running
                  else {
                    OCR1A = ptr->frequency;  // copy the value corresponding to the note to the compare register
                    TCCR1B |= (1<<CS10);  // start T/C1 in CTC mode, precounter = 1
                  }
                }
                else {
                  TCCR1B &= ~0b111;  // T/C1 off
                  // clear the sound output if set
                  if (PINB & SOUND_BITMASK) TCCR1C |= (1 << FOC1A);  // force an immediate compare match to toggle the output
                  if (_isr.tonePause) _isr.tonePause--;  // decrement if the pause is still ongoing
                  else {
                    ptr++;  // points to the structure of the next tone of the melody
                    _isr.tonePlaying = false;
                  }
                }
              }
              else {
                TCCR1B &= ~0b111;  // T/C1 off
                ptr =  _isr.soundPtr->_ptrMelodyStart;  // contains the nullptr if no repetition, otherwise points to start of melody
                if (ptr == nullptr) _isr.soundPtr->_state = NO_TONE;  // the melody has expired
              }
            }
            _isr.soundPtr->_ptrProperties = ptr;
          }  // end if MELODY
        }
      }
    }
  }
  stateCnt++;
  stateCnt &= 0x07;  // limit the range to 0 ... 7
  stateCounter = stateCnt;  // restore
}


// Since an error has occurred, the program circles in an endless loop.
// The function errorLock() could be expanded in a future version of the library,
// for example by showing an error message on the LCD, flashing LEDs, beeping buzzer ...
void errorLock(errorType_t result) {
  while (1) { }
}


// Function to determine and initialize the IO registers for the transferred pin
struct inOut_t pinToRegs(uint8_t pin, ioMode_t ioMode) {
  struct inOut_t ioReg;
  uint8_t mask, port;
  volatile uint8_t *ptrDDR, *ptrPORT, *ptrPIN;
  mask = digitalPinToBitMask(pin);  // returns e.g. 0b00000100 for pin PD2
  port = digitalPinToPort(pin);  // returns e.g. 1 for Port A, 2 for Port B, 3 for Port C ...
  ptrDDR = portModeRegister(port);  // returns e.g. (uint16_t) &DDRC for port PC
  ptrPORT = portOutputRegister(port);  // returns e.g. (uint16_t) &PORTC for port PC
  ptrPIN = portInputRegister(port);  // returns e.g. (uint16_t) &PINC for port PC
  // Initialize pin as input or output
  uint8_t savedSREG = SREG;  // save flags
  cli();  // disable interrupts
  if (ioMode == IN) {
    *ptrDDR &= ~mask;  // configure pin as high impedance input
    *ptrPORT &= ~mask;
    ioReg.ioPort = ptrPIN;  // pointer to the input register
  }
  else if (ioMode == IN_PULLUP) {
    *ptrDDR &= ~mask;  // configure pin as input
    *ptrPORT |= mask;  // connect pull up - Resistor 
    ioReg.ioPort = ptrPIN;  // pointer to the input register
  }
  else if (ioMode == OUT) {
    *ptrDDR |= mask;  // configure pin as output
    *ptrPORT &= ~mask;  // initialize with 0
    ioReg.ioPort = ptrPORT;  // pointer to the output register
  }
  else if (ioMode == OPEN_DRAIN) {
    *ptrDDR &= ~mask;  // configure pin as high impedance input
    *ptrPORT &= ~mask;  // initialize with 0
    ioReg.ioPort = ptrDDR;  // pointer to the direction register
  }
  SREG = savedSREG;  // restore flags
  ioReg.ioMask = mask;  // the bit mask of this pin in the register
  return ioReg;
}


// Function to initialize a new key instance
enum errorType_t initKey(uint8_t pin, uint8_t active, uint8_t * ptrIndex, struct inOut_t * ptrIO) {
  if (NOT_IN_RANGE(pin)) return KEY_WRONG_PIN;  // this pin cannot be used
  uint8_t instanced = _isr.instancedKeys;  // contains the number of already instantiated key objects
  if (instanced >= KEY_MAX_NUMBER) return KEY_LIMIT_EXCEEDED;  // all keys are already occupied
  _isr.instancedKeys = instanced + 1;  // write back the new number of instantiated keys
  // for quick access within the interrupt routine, the determined I/O registers are stored
  if (active == ACTIVE_HIGH) *ptrIO = pinToRegs(pin, IN);  // high impedance input
  else *ptrIO = pinToRegs(pin, IN_PULLUP);  // input with pullup resistor
  uint8_t index = 0;
  // determine the index of the new key in the pointer array of the interrupt routine
  while ((index < 8) && (_isr.keyPtr[index] != nullptr)) { index++; }
  *ptrIndex = index;  // store the index of the new key in the key pointer array
  uint8_t bitPos = (1 << index);
  // save the current status of the key
  bool keyNow = ((*ptrIO->ioPort & ptrIO->ioMask) != 0);  // read the status of the key
  if (active == ACTIVE_LOW) keyNow = !keyNow;  // invert level if active LOW
  if (keyNow) {
    _isr.keysStates |= bitPos;  // save the key status that has been read in
    _isr.keysPrevious |= bitPos;  // save the key status that has been read in
  }  
  else {
    _isr.keysStates &= ~bitPos;  // save the key status that has been read in
    _isr.keysPrevious &= ~bitPos;  // save the key status that has been read in
  }
  return SUCCESS;
}


/***************************************************************************************************************/
/*                                 Constructors / Destructor of class KeySwitch                                */
/***************************************************************************************************************/

KeySwitch::KeySwitch(uint8_t pin) {
  enum errorType_t result = initKey(pin, ACTIVE_HIGH, &keyIndex, &ioKey);
  if (result == SUCCESS) {
    inputPin = pin;  // store the pin that the switch is connected to
    _isr.keyPtr[keyIndex] = this;  // Pointer to the object for access within the interrupt routine
    activeState = ACTIVE_HIGH;  // default
    keyClosed = nullptr;
    keyOpened = nullptr;
    ENA_TOV2_INT;  // enable the T/C2 TOV2 interrupt routine
  }
  else errorLock(result);  // since an error has occurred, the program circles in an endless loop
} // end of constructor

KeySwitch::KeySwitch(uint8_t pin, uint8_t activeLevel) {
  enum errorType_t result = initKey(pin, activeLevel, &keyIndex, &ioKey);
  if (result == SUCCESS) {
    _isr.keyPtr[keyIndex] = this;  // Pointer to the object for access within the interrupt routine
    activeState = activeLevel;
    keyClosed = nullptr;
    keyOpened = nullptr;
    ENA_TOV2_INT;  // enable the T/C2 TOV2 interrupt routine
  }
  else errorLock(result);  // since an error has occurred, the program circles in an endless loop
} // end of constructor

KeySwitch::KeySwitch(uint8_t pin, uint8_t activeLevel, void(*switchedOn)()) {
  enum errorType_t result = initKey(pin, activeLevel, &keyIndex, &ioKey);
  if (result == SUCCESS) {
    _isr.keyPtr[keyIndex] = this;  // Pointer to the object for access within the interrupt routine
    activeState = activeLevel;
    keyClosed = switchedOn;
    keyOpened = nullptr;
    ENA_TOV2_INT;  // enable the T/C2 TOV2 interrupt routine
  }
  else errorLock(result);  // since an error has occurred, the program circles in an endless loop
} // end of constructor

KeySwitch::KeySwitch(uint8_t pin, uint8_t activeLevel, void(*switchedOn)(), void(*switchedOff)()) {
enum errorType_t result = initKey(pin, activeLevel, &keyIndex, &ioKey);
  if (result == SUCCESS) {
    _isr.keyPtr[keyIndex] = this;  // Pointer to the object for access within the interrupt routine
    activeState = activeLevel;
    keyClosed = switchedOn;
    keyOpened = switchedOff;
    ENA_TOV2_INT;  // enable the T/C2 TOV2 interrupt routine
  }
  else errorLock(result);  // since an error has occurred, the program circles in an endless loop
} // end of constructor

// Destructor (removes the instance from the sketch)
KeySwitch::~KeySwitch() {
  _isr.instancedKeys--;  // reduce the number of instanced switches by 1
  _isr.keyPtr[keyIndex] = nullptr;  // remove this key from the array of pointers to the instantiated keys
  uint8_t port = digitalPinToPort(inputPin);  // returns e.g. 1 for Port A, 2 for Port B, 3 for Port C ...
  volatile uint8_t *ptrPORT = portOutputRegister(port);  // returns e.g. (uint16_t) &PORTC for port PC
  uint8_t savedSREG = SREG;  // save flags
  cli();  // disable interrupts
  *ptrPORT &= ~ioKey.ioMask;  // configure pin as high impedance input
  SREG = savedSREG;  // restore flags
}


/***********************************************/
/*   Methods of class KeySwitch                */
/***********************************************/

// returns "true", if the switch is OFF and "false", if the switch is ON
bool KeySwitch::isOff() {
  return !((_isr.keysStates & (1 << keyIndex)) != 0);
}

// returns "true", if the switch is ON and "false", if the switch is OFF
bool KeySwitch::isOn() {
  return ((_isr.keysStates & (1 << keyIndex)) != 0);
}

// returns the state of the switch (HIGH for ON or LOW for OFF)
bool KeySwitch::keyState() {
  return ((_isr.keysStates & (1 << keyIndex)) != 0);
}


/***************************************************************************************************************/
/*                                          Constructors of class Matrix                                       */
/***************************************************************************************************************/

Matrix::Matrix() {
  _isr.matrixPtr = this;  // Pointer to the object for access within the interrupt routine
  // the connection pins of the matrix are configured and initialized
  // for quick access within the interrupt routine, the determined I/O registers are stored
  // configure pinA (the connection pin for column A of the matrix
  mxCol[CA] = pinToRegs(COLUMN_A_PIN, OUT);  // initialize the pin's registers and save them
  // configure pinB (the connection pin for column B of the matrix
  mxCol[CB] = pinToRegs(COLUMN_B_PIN, OUT);  // initialize the pin's registers and save them
  // configure pinC (the connection pin for column C of the matrix
  mxCol[CC] = pinToRegs(COLUMN_C_PIN, OUT);  // initialize the pin's registers and save them
  // configure pin1 (the connection pin for row 1 of the matrix
  mxRow[R1] = pinToRegs(ROW_1_PIN, OPEN_DRAIN);  // initialize the registers and save them
  // configure pin2 (the connection pin for row 2 of the matrix
  mxRow[R2] = pinToRegs(ROW_2_PIN, OPEN_DRAIN);  // initialize the registers and save them
  // configure pin3 (the connection pin for row 3 of the matrix
  mxRow[R3] = pinToRegs(ROW_3_PIN, OPEN_DRAIN);  // initialize the registers and save them
  ENA_TOV2_INT;  // enable the T/C2 TOV2 interrupt routine
}
 
Matrix::Matrix(uint8_t pinR1, uint8_t pinR2, uint8_t pinR3) {
  if (NOT_IN_RANGE(pinR1) || NOT_IN_RANGE(pinR2) || NOT_IN_RANGE(pinR3)) errorLock(MATRIX_WRONG_PIN);  // since an error has occurred, the program circles in an endless loop
  _isr.matrixPtr = this;  // Pointer to the object for access within the interrupt routine
  // the connection pins of the matrix are configured and initialized
  // for quick access within the interrupt routine, the determined I/O registers are stored
  // configure pinA (the connection pin for column A of the matrix
  mxCol[CA] = pinToRegs(COLUMN_A_PIN, OUT);  // initialize the pin's registers and save them
  // configure pinB (the connection pin for column B of the matrix
  mxCol[CB] = pinToRegs(COLUMN_B_PIN, OUT);  // initialize the pin's registers and save them
  // configure pinC (the connection pin for column C of the matrix
  mxCol[CC] = pinToRegs(COLUMN_C_PIN, OUT);  // initialize the pin's registers and save them
  // configure pin1 (the connection pin for row 1 of the matrix
  mxRow[R1] = pinToRegs(pinR1, OPEN_DRAIN);  // initialize the registers and save them
  // configure pin2 (the connection pin for row 2 of the matrix
  mxRow[R2] = pinToRegs(pinR2, OPEN_DRAIN);  // initialize the registers and save them
  // configure pin3 (the connection pin for row 3 of the matrix
  mxRow[R3] = pinToRegs(pinR3, OPEN_DRAIN);  // initialize the registers and save them
  ENA_TOV2_INT;  // enable the T/C2 TOV2 interrupt routine
}

/***********************************************/
/*   Methods of class Matrix                   */
/***********************************************/

// displays the masked bits in "leds"
void Matrix::pattern(uint16_t leds) {
  // Parameter: The 9 bits of the matrix LEDs, packed in the 16 bit - parameter "leds"
  _isr.matrixPattern = leds;  // Bits 8 ... 0: Bit pattern of the 3 x 3 matrix LEDs
}

// displays the numbers 1..6 as shown on a dice or switches all LEDs of the matrix off
enum errorType_t Matrix::dice(uint8_t number) {
  enum errorType_t result = SUCCESS;
  // Parameter: The number to be displayed on the dice
  if (number == 0) _isr.matrixPattern = DICE_OFF;  // Bits 8 ... 0: Bit pattern for switching all LEDs off
  else if (number == 1) _isr.matrixPattern = DICE_1;  // Bits 8 ... 0: Bit pattern for displaying the 1 on the dice
  else if (number == 2) _isr.matrixPattern = DICE_2;  // Bits 8 ... 0: Bit pattern for displaying the 2 on the dice
  else if (number == 3) _isr.matrixPattern = DICE_3;  // Bits 8 ... 0: Bit pattern for displaying the 3 on the dice
  else if (number == 4) _isr.matrixPattern = DICE_4;  // Bits 8 ... 0: Bit pattern for displaying the 4 on the dice
  else if (number == 5) _isr.matrixPattern = DICE_5;  // Bits 8 ... 0: Bit pattern for displaying the 5 on the dice
  else if (number == 6) _isr.matrixPattern = DICE_6;  // Bits 8 ... 0: Bit pattern for displaying the 6 on the dice
  else {
    _isr.matrixPattern = 0x1FF;  // turn on all the LEDs of the matrix
    result = OUT_OF_RANGE;
  }
  return result;
}


/***************************************************************************************************************/
/*                                          Constructor of class LED                                           */
/***************************************************************************************************************/

LED::LED(uint8_t pin) {
  // the connection pin of the LED is configured and initialized
  // for quick access within the interrupt routine, the determined I/O registers are stored
  if (pin < PIN_MIN_NUMBER || pin > PIN_LED_MAX_NUMBER) errorLock(LED_WRONG_PIN);  // since an error has occurred, the program circles in an endless loop
  uint8_t instanced = _isr.instancedLEDs;  // contains the number of already instantiated LED objects
  if (instanced >= LED_NUMBER) errorLock(LED_LIMIT_EXCEEDED);  // since an error has occurred, the program circles in an endless loop
  _isr.instancedLEDs = instanced + 1;  // write back the new number of instantiated LEDs
  ledIndex = pin - 2;  // the index 0..11 of the LED object in the ISR array
  _isr.ledPtr[ledIndex] = this;  // Pointer to the object for access within the interrupt routine
  // for quick access within the interrupt routine, the determined I/O registers are stored
  led_io = pinToRegs(pin, OUT);  // initialize the pin's registers as output and save them
  flash.freqNominal = 0;  // the nominal frequency of the flashing LED
  flash.freqCounter = 0;  // the counter for frequency generation of the flashing LED
  if (ledIndex < 6) _isr.flashingLow &= ~(1<<ledIndex);  // Flashing: Off (if 1 -> LED flashes)
  else _isr.flashingHigh &= ~(1<<(ledIndex - 6));  // Flashing: Off (if 1 -> LED flashes)
  ENA_TOV2_INT;  // enable the T/C2 TOV2 interrupt routine
} // end of constructor

/***********************************************/
/*    Methods of class LED                     */
/***********************************************/

// methode returns the state of the LED
enum ledState_t LED::getState() {
  if ((ledIndex < 6) && (_isr.flashingLow & (1<<ledIndex))) return FLASHING;
  else if (_isr.flashingHigh & (1<<(ledIndex - 6))) return FLASHING;
  else if (*led_io.ioPort & led_io.ioMask) return ON;
  else return OFF;
}

// methode switches the LED off or on
void LED::set(bool level) {
  flash.freqCounter = 0;  // the counter for handling the flashing of the LED is reset
  if (ledIndex < 6) _isr.flashingLow &= ~(1<<ledIndex);  // Blinking: Off (if 1 -> LED flashes)
  else _isr.flashingHigh &= ~(1<<(ledIndex - 6));  // Flashing: Off (if 1 -> LED flashes)
  flash.freqNominal = 0;  // the nominal frequency of the flashing LED is 0 Hz
  if (level) *led_io.ioPort |= led_io.ioMask;  // set LED-pin to HIGH
  else *led_io.ioPort &= ~led_io.ioMask;  // set LED-pin to LOW
}

// methode switches the LED off
void LED::off(void) {
  flash.freqCounter = 0;  // the counter for handling the flashing of the LED is reset
  if (ledIndex < 6) _isr.flashingLow &= ~(1<<ledIndex);  // Flashing: Off (if 1 -> LED flashes)
  else _isr.flashingHigh &= ~(1<<(ledIndex - 6));  // Flashing: Off (if 1 -> LED flashes)
  flash.freqNominal = 0;  // the nominal frequency of the flashing LED is 0 Hz
  *led_io.ioPort &= ~led_io.ioMask;  // set LED-pin to LOW
}

// methode switches the LED on
void LED::on(void) {
  flash.freqCounter = 0;  // the counter for handling the flashing of the LED is reset
  if (ledIndex < 6) _isr.flashingLow &= ~(1<<ledIndex);  // Flashing: Off (if 1 -> LED flashes)
  else _isr.flashingHigh &= ~(1<<(ledIndex - 6));  // Flashing: Off (if 1 -> LED flashes)
  flash.freqNominal = 0;  // the nominal frequency of the flashing LED is 0 Hz
  *led_io.ioPort |= led_io.ioMask;  // set LED-pin to HIGH
}

// methode inverts the state of the LED
void LED::toggle(void) {
  flash.freqCounter = 0;  // the counter for handling the flashing of the LED is reset
  if (ledIndex < 6) _isr.flashingLow &= ~(1<<ledIndex);  // Flashing: Off (if 1 -> LED flashes)
  else _isr.flashingHigh &= ~(1<<(ledIndex - 6));  // Flashing: Off (if 1 -> LED flashes)
  flash.freqNominal = 0;  // the nominal frequency of the flashing LED is 0 Hz
  *led_io.ioPort = *led_io.ioPort ^ led_io.ioMask;  // toggle LED output level
}

// methode makes the LED flash at the specified frequency
void LED::blink(enum ledFreq_t freq) {
  flash.freqNominal = GET_FREQ(freq);  // the constant for setting the nominal frequency of the flashing LED
  flash.freqCounter = 0;  // the counter for handling the flashing of the LED is reset
  if (ledIndex < 6) _isr.flashingLow |= (1<<ledIndex);  // Flashing: On (if 1 -> LED flashes)
  else _isr.flashingHigh |= (1<<(ledIndex - 6));  // Flashing: On (if 1 -> LED flashes)
}


/***************************************************************************************************************/
/*                                         Constructors of class LedBlock                                      */
/***************************************************************************************************************/

LedBlock::LedBlock(uint8_t pinFirst, uint8_t blockSize) {
  if ((pinFirst < PIN_MIN_NUMBER) || ((pinFirst + blockSize) > (PIN_LED_BLOCK_MAX+1))) errorLock(BLOCK_LIMIT_EXCEEDED);  // since an error has occurred, the program circles in an endless loop
  uint16_t mask = 1 << pinFirst;  // the bit where the block starts is set to one
  union w2b_t blck;
  blck.w = 0;
  // the lower-order mask and the higher-order mask are created for the block
  for (uint8_t i = 0; i < blockSize; mask <<= 1, i++) {
    blck.w |= mask;  // the bit is set to one
  }
  DDRB |= blck.b[1];  // configure the pins assigned to the bits in the higher part of the block as outputs
  PORTB &= ~blck.b[1];  // switch the outputs in the higher part of the block to LOW
  DDRD |= blck.b[0];  // configure the pins assigned to the bits in the lower part of the block as outputs
  PORTD &= ~blck.b[0];  // switch the outputs in the lower part of the block to LOW
  _firstLED = pinFirst;  // store the first LED of the block
  _size = blockSize;  // store the number of LEDs that the block contains
  _blockMask = blck;  // store the created masks
  _isr.flashingHigh &= ~blck.b[1];  // switch off the flashing of the bits in the higher part of the block
  _isr.flashingLow &= ~(blck.b[0] >> 2);  // switch off the flashing of the bits in the lower part of the block
}

LedBlock::LedBlock() {
  DDRB |= BLOCK_MASK_HIGH;  // configure the pins assigned to the bits in the higher part of the block as outputs
  PORTB &= ~BLOCK_MASK_HIGH;  // switch the outputs in the higher part of the block to LOW
  DDRD |= BLOCK_MASK_LOW;  // configure the pins assigned to the bits in the lower part of the block as outputs
  PORTD &= ~BLOCK_MASK_LOW;  // switch the outputs in the lower part of the block to LOW
  _firstLED = 2;  // the LED at pin D2 is the first LED
  _size = 11;  // the number of LEDs from D2 ... D12
  _blockMask.w = BLOCK_MASK;  // initialization of the masks
  _isr.flashingHigh &= 0x20;  // switch off the flashing of the bits in the higher part of the block with the exception of D13
  _isr.flashingLow = 0;  // switch off the flashing of the bits in the lower part of the block
}

/***********************************************/
/*    Methods of class LedBlock                */
/***********************************************/

// displays the bits in "pattern" on the LED block
void LedBlock::pattern(uint16_t bits) {
  union w2b_t blck;
  blck.w = bits;
  blck.w <<= _firstLED;  // shift the block to the LED where the block starts
  blck.w &= _blockMask.w;  // mask out the bits of the block
  PORTB = ((PORTB & ~_blockMask.b[1]) | blck.b[1]);  // output the bit pattern of the higher part of the block
  PORTD = ((PORTD & ~_blockMask.b[0]) | blck.b[0]);  // output the bit pattern of the lower part of the block
}

// returns the bit pattern of the LEDs in the block
uint16_t LedBlock::getState() {
  union w2b_t blck;
  blck.b[1] = PORTB & _blockMask.b[1];  // read the output pins of the higher part of the block
  blck.b[0] = PORTD & _blockMask.b[0];  // read the output pins of the lower part of the block
  blck.w >>= _firstLED;  // shift the block to align it to the right
  return blck.w;
}


/***************************************************************************************************************/
/*                                             Constructor of class Sound                                      */
/***************************************************************************************************************/

Sound::Sound() {
  _state = NO_TONE;  // stores the actual state of the sound generator
  _isr.soundPtr = this;  // Pointer to the object for access within the interrupt routine
  // configure the sound output
  DDRB |= SOUND_BITMASK;  // configure the Sound output
  PORTB &= ~SOUND_BITMASK;  // the output is initialized with 0
  ENA_TOV2_INT;  // enable the T/C2 TOV2 interrupt routine
}

/***********************************************/
/*    Methods of class Sound                   */
/***********************************************/

// plays this note until the stop() command. The value of note is the content of the compare register for this note
void Sound::play(uint16_t note) {  
  _state = CONTINUOUS_TONE;
  // configure T/C1 in Mode 4: CTC, WGM13..WGM10 = 0b0100, TOP in OCR1A
  TCCR1A = (0b01 << COM1A0) | (0b00<< WGM10);  // toggle OC1A on compare match, Mode = 4: CTC, TOP in OCR1A
  OCR1A = note;  // assign the value corresponding to the note to the compare register
  TCCR1B = (0b01 << WGM12) | 1;  // Mode = 4: CTC, TOP in OCR1A, start T/C1 with precounter = 1
}

// Plays this note for the specified duration or until the stop() command.
// The value of note is the content of the compare register OCR1A of T/C1 for this note.
// duration is in multiples of 0.1 seconds (tenth of a second), the maximum is 25,4 s (e.g. duration = 15 -> 1.5 s)
void Sound::pulse(uint16_t note, uint8_t duration) {  
  _state = PULSE;
  // configure T/C1 in Mode 4: CTC, WGM13..WGM10 = 0b0100, TOP in OCR1A
  TCCR1A = (0b01 << COM1A0) | (0b00<< WGM10);  // toggle OC1A on compare match, Mode = 4: CTC, TOP in OCR1A
  OCR1A = note;  // assign the value corresponding to the note to the compare register
  TCCR1B = (0b01 << WGM12) | 1;  // Mode = 4: CTC, TOP in OCR1A, start T/C1 with precounter = 1
  Serial.print("note = ");
  Serial.println(note);
  Serial.print("OCR1A = ");
  Serial.println(OCR1A);
  _pulseRemaining = ((duration > 254 ? 254 : duration) * 100) >> 4;  // the interrupt expires every 16 milliseconds
}

// Plays a series of consecutive tones. For each tone, its frequency, duration and the pause after its output are 
// stored in a structure. The passed pointer points to an array in which the structures of the individual tones 
// are stored. The melody ends when the pointer points to an array element with the frequency 0.
void Sound::melody(struct tone_t * ptrMelody) {
  _state = MELODY;
  // configure T/C1 in Mode 4: CTC, WGM13..WGM10 = 0b0100, TOP in OCR1A
  TCCR1A = (0b01 << COM1A0) | (0b00<< WGM10);  // toggle OC1A on compare match, Mode = 4: CTC, TOP in OCR1A
  TCCR1B = (0b01 << WGM12);  // Mode = 4: CTC, TOP in OCR1A, T/C1 not running
  _ptrProperties = ptrMelody;  // the pointer points to the first note of the melody
  _ptrMelodyStart = nullptr;  // pointer to first tone of melody, indicates that the melody should not be repeated
}

// Plays a series of consecutive tones. For each tone, its frequency, duration and the pause after its output are 
// stored in a structure. The passed pointer points to an array in which the structures of the individual tones 
// are stored. The melody ends when the pointer points to an array element with the frequency 0. 
// The second parameter determines whether the melody is played only once (repetitiv = false) or whether it
// should be repeated indefinitely until the stop() command (repetitiv = true).
void Sound::melody(struct tone_t * ptrMelody, bool repetitiv) {
  _state = MELODY;
  // configure T/C1 in Mode 4: CTC, WGM13..WGM10 = 0b0100, TOP in OCR1A
  TCCR1A = (0b01 << COM1A0) | (0b00<< WGM10);  // toggle OC1A on compare match, Mode = 4: CTC, TOP in OCR1A
  TCCR1B = (0b01 << WGM12);  // Mode = 4: CTC, TOP in OCR1A, T/C1 not running
  _ptrProperties = ptrMelody;  // the pointer points to the first note of the melody
  if (repetitiv) {
    _ptrMelodyStart = ptrMelody;  // pointer to first tone of melody, indicates that the melody should be repeated
  }
  else {
    _ptrMelodyStart = nullptr;  // pointer to first tone of melody, indicates that the melody should not be repeated
  }
}

// outputs a square wave with the specified frequency (50% duty cycle) until the stop() command. 
// Parameter "frequency" is the frequency in Hz in the range 0 ... 8 MHz.
uint32_t Sound::squareWave(uint32_t frequency) { 
  _state = SQUARE_WAVE;
  uint8_t precounter = 0b001;  // the default precounter = 1
  uint32_t clockFreq = F_CPU/2;  // the default clock signal of timer T/C1
  if (frequency == 0) {
    TCCR1B &= ~0b111;  // stop T/C1
    PORTB &= ~SOUND_BITMASK;  // the square wave output is set to 0
    return 0;  // the frequency actually set is returned
  }
  if (frequency < 2) {
    precounter = 0b100;  // precounter = 256 for frequencies below 2 Hz
    clockFreq = F_CPU/512;  // the clock signal of timer T/C1 for frequencies below 2 Hz
  }
  else if (frequency < 16) {
    precounter = 0b011;  // precounter = 64 if  2 <= frequency < 16 Hz
    clockFreq = F_CPU/128;  // the clock signal of timer T/C1 if  2 <= frequency < 16 Hz
  }
  else if (frequency < 123) {
    precounter = 0b010;  // precounter = 8 if  16 <= frequency < 123 Hz
    clockFreq = F_CPU/16;  // the clock signal of timer T/C1 if  2 <= frequency < 16 Hz
  }
  // configure T/C1 in Mode 4: CTC, WGM13 ... WGM10 = 0b0100, TOP in OCR1A
  if (frequency >= clockFreq) OCR1A = 0;  // the square wave has its maximum frequency when OCR1A is 0
  else OCR1A = (clockFreq / (uint32_t)frequency) - 1;  // load the comparison value corresponding to the frequency
  TCCR1A = (0b01 << COM1A0) | (0b00<< WGM10);  // toggle OC1A on compare match, Mode = 4: CTC, TOP in OCR1A
  TCCR1B = (0b01 << WGM12) | precounter;  // start T/C1 in CTC mode with the selected precounter
  if (OCR1A != 0) return (clockFreq / (uint32_t)OCR1A) + 1;  // the frequency actually set is returned
  else return (F_CPU/2);  // the maximum frequency is returned
}

// Outputs a PWM signal in Fast Mode with the specified duty cycle until the stop() command. 
// Parameter "resolution": The number of sections into which the period T of the pwm signal is divided (with N bits the N-th power of 2).
// Possible values for resolution are PWM_256 (8 bits), PWM_1024 (10 bits), PWM_4096 (12 bits).
// Parameter "dutyHigh": The PWM duty cycle is the ratio of the number of sections of the high pulse to the total number of sections (the 
// resolution) of the PWM signal: duty cycle = dutyHigh / resolution, dutyHigh must be < resolution.
// Possible values for dutyHigh are 0 ... (resolution - 1)
// Parameter "prescaler":  Possible values are PRESCALE_1, PRESCALE_8, PRESCALE_64
void Sound::pwm(resolution_t resolution, uint16_t dutyHigh, prescaler_t prescaler) { 
  _state = PWM;
  if (prescaler > PRESCALE_64) errorLock(SOUND_WRONG_PRESCALER);  // since an error has occurred, the program circles in an endless loop
  if (resolution > PWM_4096) errorLock(SOUND_WRONG_RESOLUTION);  // since an error has occurred, the program circles in an endless loop
  // the following commented out checks could be implemented in a future version of the library:
  //  uint32_t clockFreq;  // the clock signal of timer T/C1
  //  if (prescaler == PRESCALE_1) {
  //    clockFreq = F_CPU;  // the clock signal of timer T/C1
  //  }
  //  else if (prescaler == PRESCALE_8) {
  //    clockFreq = F_CPU>>3;  // the clock signal of timer T/C1
  //  }
  //  else if (prescaler == PRESCALE_64) {
  //    clockFreq = F_CPU>>6;  // the clock signal of timer T/C1
  //  }
  //  else errorLock(SOUND_WRONG_PRESCALER);  // since an error has occurred, the program circles in an endless loop
  //  if (resolution == PWM_256) {
  //    ICR1 = 0xFF;  // TOP value for 8 bit resolution
  //  }
  //  else if (resolution == PWM_1024) {
  //    ICR1 = 0x3FF;  // TOP value for 10 bit resolution
  //  }
  //  else if (resolution == PWM_4096) {
  //    ICR1 = 0xFFF;  // TOP value for 12 bit resolution
  //  }
  //  else errorLock(SOUND_WRONG_RESOLUTION);  // since an error has occurred, the program circles in an endless loop
  ICR1 = (0x100 << (resolution << 1)) - 1;  // TOP value for the selected resolution
  if (dutyHigh > ICR1) OCR1A = ICR1;  // the PWM compare value
  else OCR1A = dutyHigh;  // the PWM compare value
  // configure T/C1 in Mode 14: Fast PWM, WGM13 ... WGM10 = 0b1110, TOP in ICR1
  TCCR1A = (0b10 << COM1A0) | (0b10<< WGM10);  // Clear OC1A on Compare Match, Mode = 14: Fast PWM, TOP in ICR1
  TCCR1B = (0b11 << WGM12) | (prescaler + 1);  // start T/C1 in PWM Fast Mode with the selected prescaler
}

// Stops playing
void Sound::stop() {
  _state = NO_TONE;
  TCCR1B &= ~0b111;  // stop T/C1
  _pulseRemaining = 0;
  _ptrProperties = nullptr;  // pointer to a structure that contains the properties of a melody tone
  _ptrMelodyStart = nullptr;  // pointer to first tone of melody, indicates that the melody should not be repeated
  // clear the sound output if set
  if (PINB & SOUND_BITMASK) TCCR1C |= (1 << FOC1A);  // force an immediate compare match 
}

// Returns the current status of the sound generator
sound_t Sound::getState() {
  return _state;
}


// end of file
