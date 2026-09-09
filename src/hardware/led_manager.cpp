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

#include "led_manager.h"

// Global LED manager instance
LedManager ledManager;

// ================================================================================================
// CONSTRUCTOR / DESTRUCTOR
// ================================================================================================

LedManager::LedManager() {
    _initialized = false;
}

LedManager::~LedManager() {
    // Turn off LED
    if (_initialized) {
        off();
    }
}

// ================================================================================================
// INITIALIZATION
// ================================================================================================

void LedManager::begin(uint8_t pin, bool active_high) {
    _pin = pin;
    _active_high = active_high;
    
    // Check if pin is valid
    if (_pin == 255 || _pin == 0xFF) {
        // Invalid pin - disable LED
        _initialized = false;
        return;
    }
    
    // Check if pin is PWM capable
    _pwm_capable = isPwmCapable(_pin);
    
    // Initialize pin
    pinMode(_pin, OUTPUT);
    setPin(false); // Start with LED off
    
    _initialized = true;
    _current_state = LedState::LED_OFF;
    _target_state = LedState::LED_OFF;
}

// ================================================================================================
// BASIC LED CONTROL
// ================================================================================================

void LedManager::on() {
    setState(LedState::LED_ON);
}

void LedManager::off() {
    setState(LedState::LED_OFF);
}

void LedManager::toggle() {
    if (_current_state == LedState::LED_ON) {
        off();
    } else {
        on();
    }
}

void LedManager::setState(LedState state) {
    _target_state = state;

    // Each blink state carries its own cadence. Without this, both states shared the
    // default interval and were indistinguishable, the enum promising two rates and
    // delivering one.
    if (state == LedState::LED_BLINK_SLOW) {
        _blink_interval_ms = 1000;
    } else if (state == LedState::LED_BLINK_FAST) {
        _blink_interval_ms = 200;
    }
    
    // If it's a simple state, apply immediately
    if (state == LedState::LED_ON) {
        setPin(true);
        _current_state = state;
    } else if (state == LedState::LED_OFF) {
        setPin(false);
        _current_state = state;
    }
    // For other states, they will be handled in update()
}

// ================================================================================================
// BLINK PATTERNS
// ================================================================================================

void LedManager::blink(uint32_t interval_ms) {
    _blink_interval_ms = interval_ms;
    _last_blink_time = millis();
    _blink_on = false;
    _target_state = LedState::LED_BLINK_SLOW;
}

void LedManager::pulse(uint32_t period_ms, uint8_t min_duty, uint8_t max_duty) {
    _pulse_period_ms = period_ms;
    _pulse_min_duty = min_duty;
    _pulse_max_duty = max_duty;
    _pulse_start_time = millis();
    _target_state = LedState::LED_PULSE;
}

// ================================================================================================
// MORSE CODE PATTERNS
// ================================================================================================

void LedManager::morseOK() {
    // Morse code for "OK": --- (O) then -. - (K)
    morsePattern("--.---.-.");
}

void LedManager::morseError() {
    // Morse code for error: just a single dot
    morsePattern(".");
}

void LedManager::morsePattern(const char* pattern) {
    if (pattern == nullptr) {
        _morse_pattern = "";
        return;
    }
    
    _morse_pattern = String(pattern);
    _morse_index = 0;
    _morse_start_time = millis();
    _morse_complete = false;
    _target_state = LedState::LED_MORSEcustom;
}

// ================================================================================================
// STATUS INDICATION
// ================================================================================================

void LedManager::indicateInitializing() {
    // Fast blink during initialization
    blink(100);
}

void LedManager::indicateReady() {
    // Solid on when ready
    on();
}

void LedManager::indicateGpsSearching() {
    // Slow blink while searching for GPS
    blink(500);
}

void LedManager::indicateGpsFix() {
    // Morse "OK" when GPS has fix
    morseOK();
}

void LedManager::indicateTransmitting() {
    // Quick pulse when transmitting
    pulse(500, 100, 255);
}

void LedManager::indicateError() {
    // Morse error pattern
    morseError();
}

// ================================================================================================
// UPDATE FUNCTION
// ================================================================================================

void LedManager::update() {
    if (!_initialized) {
        return;
    }
    
    // Update based on current state
    switch (_current_state) {
        case LedState::LED_BLINK_SLOW:
        case LedState::LED_BLINK_FAST:
            updateBlink();
            break;
        case LedState::LED_PULSE:
            updatePulse();
            break;
        case LedState::LED_MORSE_OK:
        case LedState::LED_MORSE_ERROR:
        case LedState::LED_MORSEcustom:
            updateMorse();
            break;
        default:
            // No update needed for ON/OFF states
            break;
    }
    
    // Check if we need to transition to target state
    if (_current_state != _target_state) {
        _current_state = _target_state;
        
        // Initialize state-specific variables
        switch (_current_state) {
            case LedState::LED_ON:
                setPin(true);
                break;
            case LedState::LED_OFF:
                setPin(false);
                break;
            case LedState::LED_BLINK_SLOW:
            case LedState::LED_BLINK_FAST:
                _last_blink_time = millis();
                _blink_on = false;
                setPin(false);
                break;
            case LedState::LED_PULSE:
                _pulse_start_time = millis();
                setPin(false);
                break;
            case LedState::LED_MORSE_OK:
            case LedState::LED_MORSE_ERROR:
            case LedState::LED_MORSEcustom:
                _morse_index = 0;
                _morse_start_time = millis();
                _morse_complete = false;
                break;
        }
    }
}

// ================================================================================================
// INTERNAL FUNCTIONS
// ================================================================================================

void LedManager::setPin(bool on) {
    if (!_initialized) {
        return;
    }
    
    if (_active_high) {
        digitalWrite(_pin, on ? HIGH : LOW);
    } else {
        digitalWrite(_pin, on ? LOW : HIGH);
    }
}

bool LedManager::isPwmCapable(uint8_t pin) const {
    // Check if pin supports PWM
    // This is platform-specific
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        // ESP32: most pins support PWM (LED PWM controllers)
        // But some pins are input-only or have restrictions
        return true;
    #elif defined(PLATFORM_ESP8266)
        // ESP8266: PWM on pins 0-16 (except some)
        return (pin <= 16);
    #else
        // Default: assume PWM capable
        return true;
    #endif
}

void LedManager::setPwm(uint8_t duty) {
    if (!_initialized || !_pwm_capable) {
        return;
    }
    
    // Set PWM duty cycle
    analogWrite(_pin, duty);
}

void LedManager::updateBlink() {
    uint32_t current_time = millis();
    
    if (current_time - _last_blink_time >= _blink_interval_ms) {
        _last_blink_time = current_time;
        _blink_on = !_blink_on;
        setPin(_blink_on);
    }
}

void LedManager::updatePulse() {
    if (!_pwm_capable) {
        // Fallback to blink if PWM not available
        updateBlink();
        return;
    }
    
    uint8_t duty = calculatePulseDuty();
    setPwm(duty);
}

void LedManager::updateMorse() {
    uint32_t current_time = millis();
    uint32_t elapsed = current_time - _morse_start_time;
    
    // If we're in the middle of displaying a symbol
    if (!_morse_complete) {
        // Check if it's time to start the next symbol
        if (_morse_index < _morse_pattern.length()) {
            char symbol = _morse_pattern[_morse_index];
            
            if (symbol == '.' || symbol == '-') {
                // Display the symbol
                displayMorseSymbol(symbol);
                _morse_index++;
                _morse_start_time = current_time;
            } else if (symbol == ' ' || symbol == '/') {
                // Space between letters or words
                setPin(false);
                _morse_index++;
                _morse_start_time = current_time;
            } else {
                // Unknown symbol - skip
                _morse_index++;
                _morse_start_time = current_time;
            }
        } else {
            // Pattern complete
            setPin(false);
            _morse_complete = true;
            // Don't change state - will be handled by state machine
        }
    }
}

void LedManager::displayMorseSymbol(char symbol) {
    if (symbol == '.') {
        // Dot: LED on for dot duration
        setPin(true);
    } else if (symbol == '-') {
        // Dash: LED on for dash duration
        setPin(true);
    }
}

uint8_t LedManager::calculatePulseDuty() {
    uint32_t current_time = millis();
    uint32_t elapsed = current_time - _pulse_start_time;
    
    // Calculate position in cycle (0-255)
    uint32_t cycle_position = (elapsed % _pulse_period_ms) * 256 / _pulse_period_ms;
    
    // Triangle wave: linear interpolation between min and max
    if (cycle_position < 128) {
        // Rising
        return _pulse_min_duty + (cycle_position * (_pulse_max_duty - _pulse_min_duty) / 128);
    } else {
        // Falling
        return _pulse_max_duty - ((cycle_position - 128) * (_pulse_max_duty - _pulse_min_duty) / 128);
    }
}
