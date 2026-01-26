#include "LEDController.h"
#include "../config/Config.h"

// Inicializar instancia estática
LEDController* LEDController::instance = nullptr;

LEDController::LEDController(int ledPin) 
    : pin(ledPin), 
      currentState(LEDState::OFF), 
      lastBlinkTime(0), 
      blinkState(false) {
}

LEDController* LEDController::getInstance(int ledPin) {
    if (instance == nullptr) {
        instance = new LEDController(ledPin);
    }
    return instance;
}

void LEDController::begin() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    
    if (DEBUG_SERIAL) {
        Serial.printf("LED inicializado en GPIO %d\n", pin);
    }
}

void LEDController::update() {
    switch (currentState) {
        case LEDState::OFF:
            digitalWrite(pin, LOW);
            break;
            
        case LEDState::ON:
            digitalWrite(pin, HIGH);
            break;
            
        case LEDState::CONNECTING:
        case LEDState::AP_MODE:
            // Parpadeo
            if (millis() - lastBlinkTime >= LED_BLINK_INTERVAL) {
                lastBlinkTime = millis();
                blinkState = !blinkState;
                digitalWrite(pin, blinkState ? HIGH : LOW);
            }
            break;
    }
}

void LEDController::setState(LEDState state) {
    currentState = state;
    
    if (DEBUG_SERIAL) {
        const char* stateStr;
        switch (state) {
            case LEDState::OFF: stateStr = "OFF"; break;
            case LEDState::ON: stateStr = "ON"; break;
            case LEDState::CONNECTING: stateStr = "CONNECTING"; break;
            case LEDState::AP_MODE: stateStr = "AP_MODE"; break;
            default: stateStr = "UNKNOWN"; break;
        }
        Serial.printf("LED estado cambiado a: %s\n", stateStr);
    }
}

void LEDController::turnOn() {
    digitalWrite(pin, HIGH);
    currentState = LEDState::ON;
}

void LEDController::turnOff() {
    digitalWrite(pin, LOW);
    currentState = LEDState::OFF;
}

void LEDController::toggle() {
    if (digitalRead(pin) == HIGH) {
        turnOff();
    } else {
        turnOn();
    }
}

LEDState LEDController::getState() const {
    return currentState;
}

bool LEDController::isOn() const {
    return digitalRead(pin) == HIGH;
}

String LEDController::getStateString() const {
    return isOn() ? "ON" : "OFF";
}