// ************************************************************************************************
// * BALISE D'IDENTIFICATION A DISTANCE POUR UAV (DGAC)                                           *
// ************************************************************************************************
// * Author:     Maxime MARCONATO (maxime@maaax.com, aka MaaaX^^, aka EPOCH84)
// * Repository: https://github.com/EPOCH84/Balise_Identification_A_Distance_UAV_DGAC
// * Based on:
// * - https://github.com/fanfanlatulipe26/BaliseDGAC_GPS_Logger
// * - https://github.com/dev-fred/GPS_Tracker_ESP8266
// * - https://github.com/khancyr/droneID_FR
// * - https://github.com/f5soh/balise_esp32
// * - https://www.tranquille-informatique.fr/modelisme/divers/balise-dgac-signalement-electronique-a-distance-drone-aeromodelisme.html
// ************************************************************************************************
// GPL V3:
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// ================================================================================================

#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Arduino.h>
#include "../core/biadud_platform.h"

// ================================================================================================
// LED PATTERNS (Morse code)
// ================================================================================================

// Morse code timing (in milliseconds)
#define MORSE_DOT_DURATION       200     // Duration of a dot
#define MORSE_DASH_DURATION      600     // Duration of a dash (3x dot)
#define MORSE_SYMBOL_SPACING     200     // Space between symbols in same letter
#define MORSE_LETTER_SPACING     600     // Space between letters (3x dot)
#define MORSE_WORD_SPACING       1400    // Space between words (7x dot)

// Morse code for letters (dot = 1, dash = 0)
// Each letter is 5 bits, LSB is first symbol
#define MORSE_A     0b01001  // .-
#define MORSE_B     0b10000  // -...
#define MORSE_C     0b10101  // -.-.
#define MORSE_D     0b10001  // -..
#define MORSE_E     0b00001  // .
#define MORSE_F     0b00101  // ..-.
#define MORSE_G     0b11001  // --.
#define MORSE_H     0b00000  // ....
#define MORSE_I     0b00010  // ..
#define MORSE_J     0b01110  // .---
#define MORSE_K     0b10100  // -.-
#define MORSE_L     0b01000  // .-..
#define MORSE_M     0b11000  // --
#define MORSE_N     0b10000  // -.
#define MORSE_O     0b11100  // ---
#define MORSE_P     0b01101  // .--.
#define MORSE_Q     0b11011  // --.-
#define MORSE_R     0b01010  // .-.
#define MORSE_S     0b00010  // ...
#define MORSE_T     0b10000  // -
#define MORSE_U     0b00100  // ..-
#define MORSE_V     0b00011  // ...-
#define MORSE_W     0b01100  // .--
#define MORSE_X     0b10011  // -..-
#define MORSE_Y     0b10111  // -.--
#define MORSE_Z     0b11001  // --..

// Numbers
#define MORSE_0     0b11111  // -----
#define MORSE_1     0b01111  // .----
#define MORSE_2     0b00111  // ..---
#define MORSE_3     0b00011  // ...--
#define MORSE_4     0b00001  // ....-
#define MORSE_5     0b00000  // .....
#define MORSE_6     0b10000  // -....
#define MORSE_7     0b11000  // --...
#define MORSE_8     0b11100  // ---..
#define MORSE_9     0b11110  // ----.

// ================================================================================================
// LED STATES
// ================================================================================================

enum class LedState : uint8_t {
    LED_OFF = 0,
    LED_ON = 1,
    LED_BLINK_SLOW = 2,      // 1 Hz blink
    LED_BLINK_FAST = 3,      // 2 Hz blink
    LED_PULSE = 4,           // Smooth pulse
    LED_MORSE_OK = 5,        // Morse code "OK"
    LED_MORSE_ERROR = 6,     // Morse code for error
    LED_MORSEcustom = 7,     // Custom Morse pattern
};

// ================================================================================================
// LED MANAGER CLASS
// ================================================================================================

class LedManager
{
public:
    // -------------------------------------------------------------------------------------------
    // CONSTRUCTOR / DESTRUCTOR
    // -------------------------------------------------------------------------------------------
    
    LedManager();
    ~LedManager();

    // -------------------------------------------------------------------------------------------
    // INITIALIZATION
    // -------------------------------------------------------------------------------------------

    /**
     * Initialize the LED manager
     * 
     * @param pin The GPIO pin for the LED
     * @param active_high true if LED is active high, false if active low
     */
    void begin(uint8_t pin = PIN_LED, bool active_high = true);

    // -------------------------------------------------------------------------------------------
    // BASIC LED CONTROL
    // -------------------------------------------------------------------------------------------

    /**
     * Turn LED on
     */
    void on();

    /**
     * Turn LED off
     */
    void off();

    /**
     * Toggle LED state
     */
    void toggle();

    /**
     * Set LED state
     * 
     * @param state LED_STATE ON or OFF
     */
    void setState(LedState state);

    // -------------------------------------------------------------------------------------------
    // BLINK PATTERNS
    // -------------------------------------------------------------------------------------------

    /**
     * Set LED to blink at a specific interval
     * 
     * @param interval_ms Blink interval in milliseconds (on and off each for interval_ms)
     */
    void blink(uint32_t interval_ms = 500);

    /**
     * Set LED to pulse (fade in/out)
     * Only works on PWM-capable pins
     * 
     * @param period_ms Pulse period in milliseconds
     * @param min_duty Minimum duty cycle (0-255)
     * @param max_duty Maximum duty cycle (0-255)
     */
    void pulse(uint32_t period_ms = 1000, uint8_t min_duty = 50, uint8_t max_duty = 200);

    // -------------------------------------------------------------------------------------------
    // MORSE CODE PATTERNS
    // -------------------------------------------------------------------------------------------

    /**
     * Display Morse code "OK"
     * O = ---, K = -.-
     */
    void morseOK();

    /**
     * Display Morse code for error (E = .)
     * Just a single dot
     */
    void morseError();

    /**
     * Display custom Morse code
     * 
     * @param pattern Morse code pattern as string (e.g., ".-.-.-.-.." for OK)
     */
    void morsePattern(const char* pattern);

    // -------------------------------------------------------------------------------------------
    // STATUS INDICATION
    // -------------------------------------------------------------------------------------------

    /**
     * Indicate system is initializing
     */
    void indicateInitializing();

    /**
     * Indicate system is ready
     */
    void indicateReady();

    /**
     * Indicate GPS is searching
     */
    void indicateGpsSearching();

    /**
     * Indicate GPS has fix
     */
    void indicateGpsFix();

    /**
     * Indicate beacon is transmitting
     */
    void indicateTransmitting();

    /**
     * Indicate error condition
     */
    void indicateError();

    // -------------------------------------------------------------------------------------------
    // UPDATE FUNCTION
    // Call this regularly in the main loop
    // -------------------------------------------------------------------------------------------

    /**
     * Update LED state
     * Call this regularly (e.g., in main loop)
     */
    void update();

private:
    // -------------------------------------------------------------------------------------------
    // INTERNAL DATA
    // -------------------------------------------------------------------------------------------

    uint8_t _pin = PIN_LED;
    bool _active_high = true;
    bool _pwm_capable = false;
    bool _initialized = false;
    
    LedState _current_state = LedState::LED_OFF;
    LedState _target_state = LedState::LED_OFF;
    
    // Blink/pulse parameters
    uint32_t _blink_interval_ms = 500;
    uint32_t _last_blink_time = 0;
    bool _blink_on = false;
    
    // Pulse parameters
    uint32_t _pulse_period_ms = 1000;
    uint8_t _pulse_min_duty = 50;
    uint8_t _pulse_max_duty = 200;
    uint32_t _pulse_start_time = 0;
    
    // Morse code parameters
    String _morse_pattern;
    uint16_t _morse_index = 0;
    uint32_t _morse_start_time = 0;
    bool _morse_complete = true;
    
    // -------------------------------------------------------------------------------------------
    // INTERNAL FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Set the LED pin state
     * 
     * @param on true to turn on, false to turn off
     */
    void setPin(bool on);

    /**
     * Check if pin is PWM capable
     * 
     * @return true if pin supports PWM
     */
    bool isPwmCapable(uint8_t pin) const;

    /**
     * Set PWM duty cycle
     * 
     * @param duty Duty cycle (0-255)
     */
    void setPwm(uint8_t duty);

    /**
     * Update blink state
     */
    void updateBlink();

    /**
     * Update pulse state
     */
    void updatePulse();

    /**
     * Update Morse code display
     */
    void updateMorse();

    /**
     * Display a single Morse symbol
     * 
     * @param symbol '.' for dot, '-' for dash
     */
    void displayMorseSymbol(char symbol);

    /**
     * Calculate PWM duty for current time in pulse cycle
     * 
     * @return Duty cycle (0-255)
     */
    uint8_t calculatePulseDuty();
};

// Global LED manager instance
extern LedManager ledManager;

#endif // #ifndef LED_MANAGER_H
