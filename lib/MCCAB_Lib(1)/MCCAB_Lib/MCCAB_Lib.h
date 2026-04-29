/*
 * MCCAB_Lib.h
 * Rev. 3.3
 *
 *  Created on: 20.12.2021
 *      Author: Wolfgang Trampert
 *      
 * History
 * Rev. 3.3
 * The released version of the preliminary draft version 0.16 suitable for MCCAB Trainingsboard 3.3
 */

#ifndef MCCAB_Lib_HPP_
#define MCCAB_Lib_HPP_

#include "Arduino.h"


/**************************************************************************************************/
/*  Hardware definitions for the MCCAB Trainingsboard                                             */
/**************************************************************************************************/

// Hardware Equipment
#define PIN_MIN_NUMBER         2  // pins can be used from D2
#define PIN_MAX_NUMBER        17  // pins can be used up to D17 (= A3)
#define PIN_LED_MAX_NUMBER    13  // LEDs are connected to D2 to D13
#define PIN_LED_BLOCK_MAX     12  // the LED block comprises LEDs D2 to D12
#define KEY_MAX_NUMBER         8  // maximum number of keys that can be instantiated
#define LED_NUMBER            12  // the number of LEDs on the board (D2 ... D13)
#define MATRIX_ROW_NUMBER      3  // the number of matrix rows
#define MATRIX_COL_NUMBER      3  // the number of matrix columns


/*************************************************************************************************/
/*                                     Structs and Unions                                        */
/*************************************************************************************************/

union w2b_t {
  uint16_t  w;     // one word
  uint8_t   b[2];  // two bytes
};

// Structs for interrupt handling
struct inOut_t {
  volatile uint8_t * ioPort;  // pointer to the input / output port
  volatile uint8_t ioMask;    // mask of the bit position within the port
};

struct flash_t {
  uint8_t  freqNominal;  // contains the nominal frequency of the flashing LED
  uint8_t  freqCounter;  // contains the counter for the frequency generation of the flashing LED
};

struct tone_t {
  uint16_t frequency;  // contains the frequency of the tone
  uint8_t  duration;   // contains the duration that the tone is played in multiples of 0.1 seconds
  uint8_t  pause;      // contains the duration of the pause after switching off the tone in multiples of 0.1 seconds
};


/*************************************************************************************************/
/*                                 Constants used by the classes                                 */
/*************************************************************************************************/

// Constants for key handling
#define ACTIVE_HIGH      0   // when the switch is closed there is a HIGH level on the input pin
#define ACTIVE_LOW       1   // when the switch is closed there is a LOW level on the input pin
#define SK1              A0  // the switch/key is connected to this pin if the corresponding jumper is placed on the JP2 pin header
#define SK2              A1  // the switch/key is connected to this pin if the corresponding jumper is placed on the JP2 pin header
#define SK3              A2  // the switch/key is connected to this pin if the corresponding jumper is placed on the JP2 pin header
#define SK4              A3  // the switch/key is connected to this pin if the corresponding jumper is placed on the JP2 pin header
#define SK5              12  // the switch/key is connected to this pin if the corresponding jumper is placed on the JP2 pin header
#define SK6              13  // the switch/key is connected to this pin if the corresponding jumper is placed on the JP2 pin header
#define S1_DEFAULT_PIN   A0  // the switch/key is connected to this pin if the corresponding jumper is placed on the JP2 pin header
#define S2_DEFAULT_PIN   A1  // the switch/key is connected to this pin if the corresponding jumper is placed on the JP2 pin header
#define S3_DEFAULT_PIN   A2  // the switch/key is connected to this pin if the corresponding jumper is placed on the JP2 pin header
#define S4_DEFAULT_PIN   A3  // the switch/key is connected to this pin if the corresponding jumper is placed on the JP2 pin header
#define S5_DEFAULT_PIN   12  // the switch/key is connected to this pin if the corresponding jumper is placed on the JP2 pin header
#define S6_DEFAULT_PIN   13  // the switch/key is connected to this pin if the corresponding jumper is placed on the JP2 pin header

// GPIOs of the matrix connections
#define ROW_1_PIN        3  // Row 1 is connected to GPIO D3 via a jumper
#define ROW_2_PIN        4  // Row 2 is connected to GPIO D4 via a jumper
#define ROW_3_PIN        5  // Row 3 is connected to GPIO D5 via a jumper
#define COLUMN_A_PIN     8  // column A is connected to GPIO D8
#define COLUMN_B_PIN     7  // column B is connected to GPIO D7
#define COLUMN_C_PIN     6  // column C is connected to GPIO D6
// Group masks for the rows of the 3 x 3 LED matrix
#define ROW1_MASK        0x01C0  // Bits 8 ... 6 = A1 - B1 - C1
#define ROW2_MASK        0x0038  // Bits 5 ... 3 = A2 - B2 - C2
#define ROW3_MASK        0x0007  // Bits 2 ... 0 = A3 - B3 - C3
// Masks for the columns of the 3 x 3 LED matrix
#define A3_UPPER         0x01   // Bit 8 = Bit 0 in the higher byte
#define A3_MASK          0b100000000  // Bit 8
#define B3_MASK          0b010000000  // Bit 7
#define C3_MASK          0b001000000  // Bit 6
#define A2_MASK          0b000100000  // Bit 5
#define B2_MASK          0b000010000  // Bit 4
#define C2_MASK          0b000001000  // Bit 3
#define A1_MASK          0b000000100  // Bit 2
#define B1_MASK          0b000000010  // Bit 1
#define C1_MASK          0b000000001  // Bit 0
// Array indices for the 3 x 3 LED matrix
#define CA               0  // Index of column A in array mxCol[MATRIX_COL_NUMBER}
#define CB               1  // Index of column B in Array mxCol[MATRIX_COL_NUMBER}
#define CC               2  // Index of column C in Array mxCol[MATRIX_COL_NUMBER}
#define R1               0  // Index of row 1 in Array mxRow[MATRIX_ROW_NUMBER}
#define R2               1  // Index of row 2 in Array mxRow[MATRIX_ROW_NUMBER}
#define R3               2  // Index of row 3 in Array mxRow[MATRIX_ROW_NUMBER}
// Bit patterns for the dice display
#define DICE_OFF         0
#define DICE_1           (B2_MASK)
#define DICE_2           (A3_MASK | C1_MASK)
#define DICE_3           ((A1_MASK) | B2_MASK | C3_MASK)
#define DICE_4           ((A1_MASK) | A3_MASK | C1_MASK | C3_MASK)
#define DICE_5           ((A1_MASK) | A3_MASK | B2_MASK | C1_MASK | C3_MASK)
#define DICE_6           ((A1_MASK) | A2_MASK | A3_MASK | C1_MASK | C2_MASK | C3_MASK)

// Group mask for the flashing LEDs
#define LEDS_MASK        0x3F    // Bits 5 ... 0
// Single masks for the flashing LEDs
#define LED_D2_MASK      0x0001  // Bit 0
#define LED_D8_MASK      0x0001  // Bit 0
// Periods of the flashing LEDs if T/C2 interrupt occurs every 2 ms
#define F_0_5            62
#define F_0_75           42
#define F_1              31
#define F_1_5            42
#define F_2              16
#define F_3              10
#define F_4              8
#define F_5              6
#define F_6              5
#define F_8              4
#define F_10             3
#define F_16             2
// Array indexes for the flashing LEDs
#define _LED_D2_         0
#define _LED_D8_         6
// Output bits of the LEDs
#define _BIT_D2_         PD2
#define _BIT_D7_         PD7
#define _BIT_D8_         PB0
#define _BIT_D13_        PB5

// Masks for the LedBlock handling
#define BLOCK_MASK_LOW   0xFC  // pins D7 ... D2
#define BLOCK_MASK_HIGH  0x1F  // pins D12 ... D8
#define BLOCK_MASK       0x1FFC  // pins D12 ... D2

// Constants for the Sound handling
#define SOUND_PIN        D9
#define SOUND_BITMASK    (1 << PB1)  // D9 is bit 1 on port B
#define SOUND_INFINITE   0xFFFF
// the content of the compare register of T/C1 for the notes
#define NOTE_B2  (((F_CPU/2) / (uint32_t)123) - 1)  // German notation: H
#define NOTE_C3  (((F_CPU/2) / (uint32_t)131) - 1)  // c
#define NOTE_CS3 (((F_CPU/2) / (uint32_t)139) - 1)  // cis/des
#define NOTE_D3  (((F_CPU/2) / (uint32_t)147) - 1)  // d
#define NOTE_DS3 (((F_CPU/2) / (uint32_t)156) - 1)  // dis/es
#define NOTE_E3  (((F_CPU/2) / (uint32_t)165) - 1)  // e
#define NOTE_F3  (((F_CPU/2) / (uint32_t)175) - 1)  // f
#define NOTE_FS3 (((F_CPU/2) / (uint32_t)185) - 1)  // fis/ges
#define NOTE_G3  (((F_CPU/2) / (uint32_t)196) - 1)  // g
#define NOTE_GS3 (((F_CPU/2) / (uint32_t)208) - 1)  // gis/as 
#define NOTE_A3  (((F_CPU/2) / (uint32_t)220) - 1)  // a
#define NOTE_AS3 (((F_CPU/2) / (uint32_t)233) - 1)  // ais/b
#define NOTE_B3  (((F_CPU/2) / (uint32_t)247) - 1)  // h
#define NOTE_C4  (((F_CPU/2) / (uint32_t)262) - 1)  // c1
#define NOTE_CS4 (((F_CPU/2) / (uint32_t)277) - 1)
#define NOTE_D4  (((F_CPU/2) / (uint32_t)294) - 1)
#define NOTE_DS4 (((F_CPU/2) / (uint32_t)311) - 1)
#define NOTE_E4  (((F_CPU/2) / (uint32_t)330) - 1)
#define NOTE_F4  (((F_CPU/2) / (uint32_t)349) - 1)
#define NOTE_FS4 (((F_CPU/2) / (uint32_t)370) - 1)
#define NOTE_G4  (((F_CPU/2) / (uint32_t)392) - 1)
#define NOTE_GS4 (((F_CPU/2) / (uint32_t)415) - 1)
#define NOTE_A4  (((F_CPU/2) / (uint32_t)440) - 1)
#define NOTE_AS4 (((F_CPU/2) / (uint32_t)466) - 1)
#define NOTE_B4  (((F_CPU/2) / (uint32_t)494) - 1)  // h1
#define NOTE_C5  (((F_CPU/2) / (uint32_t)523) - 1)  // c2
#define NOTE_CS5 (((F_CPU/2) / (uint32_t)554) - 1)
#define NOTE_D5  (((F_CPU/2) / (uint32_t)587) - 1)
#define NOTE_DS5 (((F_CPU/2) / (uint32_t)622) - 1)
#define NOTE_E5  (((F_CPU/2) / (uint32_t)659) - 1)
#define NOTE_F5  (((F_CPU/2) / (uint32_t)698) - 1)
#define NOTE_FS5 (((F_CPU/2) / (uint32_t)740) - 1)
#define NOTE_G5  (((F_CPU/2) / (uint32_t)784) - 1)
#define NOTE_GS5 (((F_CPU/2) / (uint32_t)831) - 1)
#define NOTE_A5  (((F_CPU/2) / (uint32_t)880) - 1)
#define NOTE_AS5 (((F_CPU/2) / (uint32_t)932) - 1)
#define NOTE_B5  (((F_CPU/2) / (uint32_t)988) - 1)  // h2
#define NOTE_C6  (((F_CPU/2) / (uint32_t)1047) - 1)  // c3
#define NOTE_CS6 (((F_CPU/2) / (uint32_t)1109) - 1)
#define NOTE_D6  (((F_CPU/2) / (uint32_t)1175) - 1)
#define NOTE_DS6 (((F_CPU/2) / (uint32_t)1245) - 1)
#define NOTE_E6  (((F_CPU/2) / (uint32_t)1319) - 1)
#define NOTE_F6  (((F_CPU/2) / (uint32_t)1397) - 1)
#define NOTE_FS6 (((F_CPU/2) / (uint32_t)1480) - 1)
#define NOTE_G6  (((F_CPU/2) / (uint32_t)1568) - 1)
#define NOTE_GS6 (((F_CPU/2) / (uint32_t)1661) - 1)
#define NOTE_A6  (((F_CPU/2) / (uint32_t)1760) - 1)
#define NOTE_AS6 (((F_CPU/2) / (uint32_t)1865) - 1)
#define NOTE_B6  (((F_CPU/2) / (uint32_t)1976) - 1)  // h3
#define NOTE_C7  (((F_CPU/2) / (uint32_t)2093) - 1)  // c4
#define NOTE_CS7 (((F_CPU/2) / (uint32_t)2217) - 1)  // cis4/des4
#define NOTE_D7  (((F_CPU/2) / (uint32_t)2349) - 1)  // d4
#define NOTE_DS7 (((F_CPU/2) / (uint32_t)2489) - 1)  // dis4/es4
#define NOTE_E7  (((F_CPU/2) / (uint32_t)2637) - 1)  // e4
#define NOTE_F7  (((F_CPU/2) / (uint32_t)2794) - 1)  // f4
#define NOTE_FS7 (((F_CPU/2) / (uint32_t)2960) - 1)  // fis4/ges4
#define NOTE_G7  (((F_CPU/2) / (uint32_t)3136) - 1)  // g4
#define NOTE_GS7 (((F_CPU/2) / (uint32_t)3322) - 1)  // gis4/as4
#define NOTE_A7  (((F_CPU/2) / (uint32_t)3520) - 1)  // a4
#define NOTE_AS7 (((F_CPU/2) / (uint32_t)3729) - 1)  // ais4/b4
#define NOTE_B7  (((F_CPU/2) / (uint32_t)3951) - 1)  // h4
#define NOTE_C8  (((F_CPU/2) / (uint32_t)4186) - 1)  // c5
#define NOTE_CS8 (((F_CPU/2) / (uint32_t)4435) - 1)  // cis5/des5
#define NOTE_D8  (((F_CPU/2) / (uint32_t)4699) - 1)  // d5
#define NOTE_DS8 (((F_CPU/2) / (uint32_t)4978) - 1)  // dis5/es5


/*************************************************************************************************/
/*                                                Macros                                         */
/*************************************************************************************************/

#define GET_FREQ(P) ( pgm_read_byte( flashingFrequency_PGM + (P) ) )
// the rows are set to a high-impedance state or to LOW by switching the data direction register
#define SELECT_ROW(ptrOff1,mask1,ptrOff2,mask2,ptrOn,maskOn) *ptrOff1 &= ~mask1; \
                                                             *ptrOff2 &= ~mask2; \
                                                             *ptrOn   |= maskOn
#define SET_COL(bits,colMask,ptr,mask) if(bits & colMask) { *ptr |= mask; } else { *ptr &= ~mask; }
#define NOT_IN_RANGE(pin) (((pin < PIN_MIN_NUMBER) || (pin > PIN_MAX_NUMBER)) ? true : false)

// T/C2 is configured for phase correct pwm (8-bit) in the file wiring.c
// enable the T/C2 TOV2 interrupt routine to generate an interrupt each time the counter reaches the BOTTOM value
#define ENA_TOV2_INT TIMSK2 |= (1<<TOIE2)


/*************************************************************************************************/
/*                                             Enumerators                                       */
/*************************************************************************************************/

// define enumerators for the error types
enum errorType_t { SUCCESS, FAILED, OUT_OF_RANGE, 
                   KEY_LIMIT_EXCEEDED, LED_LIMIT_EXCEEDED, BLOCK_LIMIT_EXCEEDED,
                   KEY_WRONG_PIN, LED_WRONG_PIN, MATRIX_WRONG_PIN, 
                   SOUND_WRONG_PRESCALER, SOUND_WRONG_RESOLUTION, endERRORTYPE };

// define enumerators for the pin type
enum ioMode_t { IN, IN_PULLUP, OUT, OPEN_DRAIN, endIO_MODE };

// define enumerators for the LED state
enum ledState_t { OFF, ON, FLASHING, endLEDSTATE };

// define enumerators for the LED flashing frequency
enum ledFreq_t { F_0_5_HZ, F_0_75_HZ, F_1_HZ, F_1_5_HZ, F_2_HZ, F_3_HZ, 
                 F_4_HZ, F_5_HZ, F_6_HZ, F_8_HZ, F_10_HZ, F_16_HZ, endLEDFREQ };

// define enumerators for the state of the sound generator
enum sound_t { NO_TONE, CONTINUOUS_TONE, PULSE, MELODY, SQUARE_WAVE, PWM, endTONES };

// define enumerators for the resolution of the sound PWM generator
enum resolution_t { PWM_256, PWM_1024, PWM_4096, endRES };

// define enumerators for the prescaler of the sound PWM generator
enum prescaler_t { PRESCALE_1, PRESCALE_8, PRESCALE_64, endPRE };


/*************************************************************************************************/
/*                                               Classes                                         */
/*************************************************************************************************/

// definition of class KeySwitch
class KeySwitch {
  private:
    uint8_t inputPin;  // the pin that the key is connected to
  public:
    // Variables (indicated as public because they are processed by the interrupt routine)
    struct inOut_t ioKey;  // contains the input port and bit mask of the switch
    uint8_t keyIndex;  // the index of the object in the key array
    uint8_t activeState;  // the switch can be ACTIVE_HIGH or ACTIVE_LOW
    // Constructors
    KeySwitch(uint8_t pin);  // switch is ACTIVE_HIGH, no actions when closing or opening the switch
    KeySwitch(uint8_t pin, uint8_t activeLevel);  // switch can be ACTIVE_HIGH or ACTIVE_LOW
    KeySwitch(uint8_t pin, uint8_t activeLevel, void(*switchedOn)());  // additional function that is executed when the switch is closed
    KeySwitch(uint8_t pin, uint8_t activeLevel, void(*switchedOn)(), void(*switchedOff)());  // additional function that is executed when the switch is opened
    // Destructor
    ~KeySwitch();  // removes the instance from the sketch
    // Methods
    bool isOff();  // returns "true", if the switch is OFF and "false", if the switch is ON
    bool isOn();  // returns "true", if the switch is ON and "false", if the switch is OFF
    bool keyState();  // returns the state of the switch (ON = true, OFF = false)
    void (*keyClosed)(void) = nullptr;  // pointer to the function that is executed when the switch is closed
    void (*keyOpened)(void) = nullptr;  // pointer to the function that is executed when the switch is opened
};
 

// definition of class Matrix
class Matrix {
  public:
    // Variables (indicated as public because they are processed by the interrupt routine)
    struct inOut_t mxRow[MATRIX_ROW_NUMBER];  // contains output port and bit mask of the matrix rows
    struct inOut_t mxCol[MATRIX_COL_NUMBER];  // contains output port and bit mask of the matrix columns
    // Constructors
    Matrix();  // the Row Pins are D3, D4 and D5 by default
    // Parameter: none
    Matrix(uint8_t pinR1, uint8_t pinR2, uint8_t pinR3);
    // Parameter: The connection pins of the rows of the matrix
    // Methods
    void pattern(uint16_t leds);  // displays the masked bits 0 ... 8 in "leds"
    // Parameter: The 9 bits of the matrix LEDs, packed in the 16 bit - parameter "leds"
    enum errorType_t dice(uint8_t number);  // displays the numbers 1..6 as shown on a dice
    // Parameter: The number to be displayed on the dice
};


// definition of class LED
class LED {
  public:
    // Variables (indicated as public because they are processed by the interrupt routine)
    uint8_t ledIndex;  // the index of the LED object in the ISR array, public because accessed by the interrupt routine
    struct inOut_t led_io;  // contains the port and bit mask of the LED
    struct flash_t flash;  // contains the nominal frequency and the counter of the flashing LED
    // Constructor
    LED(uint8_t pin);  // Parameter: The connection pin of the LED | Der Anschlusspin der LED
    // Methods
    enum ledState_t getState();  // returns the state of the LED
    void set(bool);  // switches the LED on or off
    void on(void);  // switches the LED on
    void off(void);  // switches the LED off
    void toggle(void);  // inverts the state of the LED
    void blink(enum ledFreq_t freq);  // makes the LED flash at the specified frequency
};


// definition of class LedBlock
class LedBlock {
  private:
    // Variables
    uint8_t _firstLED;  // the first LED of the block
    uint8_t _size;  // the number of LEDs that the block contains
    union w2b_t _blockMask;  // contains the lower-order mask and the higher-order mask for the block bits
  public:
    // Constructors
    LedBlock();  // The pin of the first LED is D2 and the blocksize is 11 (the number of LEDs D2 ... D12)
    LedBlock(uint8_t pinFirst, uint8_t blockSize);  // Parameters: The pin of the first LED and the number of LEDs 
    // Methods
    void pattern(uint16_t bits);  // displays the bit pattern on the LED block
    uint16_t getState();  // returns the bit pattern of the LEDs in the block
};


// definition of class Sound
class Sound {
  public:
    // Variables  (indicated as public because they are processed by the interrupt routine)
    enum sound_t _state;  // stores the actual state of the sound generator
    uint8_t _pulseRemaining = 0; // the remaining time of the tone pulse duration
    struct tone_t * _ptrProperties = nullptr;  // pointer to a structure that contains the properties of a melody tone
    struct tone_t * _ptrMelodyStart = nullptr;  // pointer to a structure that contains the first tone of the melody
    // Constructor
    Sound();  // initialize the output pin (on the MCCAB Trainingsboard the buzzer is connected to D9)
    // Methods
    void play(uint16_t note);  // plays this note until the stop() command
    void pulse(uint16_t note, uint8_t duration);  // plays the note for the specified duration or until stop(),
                                                  // duration is in multiples of 0.1 s (tenth of a second)
    void melody(struct tone_t * ptrMelody);  // plays the melody pointed to by the pointer
    void melody(struct tone_t * ptrMelody, bool repetitiv);  // plays the melody again and again
    uint32_t squareWave(uint32_t frequency);  // outputs a square wave with the specified frequency (50% duty cycle)
    void pwm(resolution_t resolution, uint16_t dutyHigh, prescaler_t prescaler);  // outputs a PWM signal
    void stop();  // stops playing
    enum sound_t getState();  // returns the actual state of the sound generator
};


/*************************************************************************************************/
/*          struct of global variables that are processed in the T/C2 interrupt routine          */
/*************************************************************************************************/

struct _isr_t {
  uint8_t instancedKeys = 0;  // contains the number of instantiated key objects
  // array of pointers to the instantiated keys
  KeySwitch * keyPtr[KEY_MAX_NUMBER] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
  // Bit pattern of indexed keys
  uint8_t keysStates = 0;   // Bits 7 ... 0: if bit = 1 -> switch is closed
  uint8_t keysPrevious = 0;  // Bits 7 ... 0: switch status at the previous query

  Matrix * matrixPtr = nullptr;  // pointer to the instantiated matrix
  // Matrix Pattern State: 0 - 0 - 0 - 0 - 0 - 0 - 0 - A1 | B1 - C1 - A2 - B2 - C2 - A3 - B3 - C3
  uint16_t matrixPattern = 0;  // Bits 8 ... 0: bit pattern of the 3 x 3 matrix LEDs

  uint8_t instancedLEDs = 0;  // contains the number of the instantiated LED objects
  LED * ledPtr[LED_NUMBER];  // array of pointers to the instantiated LED objects
  // LED Flashing State High: 0 - 0 - D13 - D12 - D11 - D10 - D9 - D8
  uint8_t flashingHigh = 0;  // Bits 5 ... 0: Flashing of the higher 6 LEDs: if 1 -> LED flashes 
  // LED Flashing State Low: 0 - 0 - D7 - D6 - D5 - D4 - D3 - D2
  uint8_t flashingLow = 0;  // Bits 5 ... 0: Flashing of the lower 6 LEDs: if 1 -> LED flashes 

  Sound * soundPtr = nullptr;  // pointer to the instantiated Sound object
  bool    tonePlaying = false;  // indicates whether a tone of the melody is being played
  uint8_t toneDuration = 0;  // contains the duration that the tone is played in multiples of 0.1 seconds
  uint8_t tonePause = 0;  // contains the duration of the pause after switching off the tone in multiples of 0.1 seconds
};


#endif /* MCCAB_Lib_HPP_ */
