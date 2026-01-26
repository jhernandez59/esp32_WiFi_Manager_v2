/*
Control del LED:

Estados del LED
Parpadeo
Indicadores visuales
*/
#ifndef LEDCONTROLLER_H
#define LEDCONTROLLER_H

#include <Arduino.h>
#include "../config/Config.h"

// Estados del LED
enum class LEDState {
    OFF,            // LED apagado
    ON,             // LED encendido fijo
    CONNECTING,     // Parpadeando (conectando a WiFi)
    AP_MODE         // Parpadeando (modo Access Point)
};

class LEDController {
private:
    static LEDController* instance;
    int pin;
    LEDState currentState;
    unsigned long lastBlinkTime;
    bool blinkState;
    
    LEDController(int ledPin); // Constructor privado
    
public:
    /**
     * Obtiene la instancia única de LEDController (Singleton)
     * @param ledPin Pin GPIO del LED
     * @return Puntero a la instancia de LEDController
     */
    static LEDController* getInstance(int ledPin = LED_PIN);
    
    /**
     * Inicializa el LED
     */
    void begin();
    
    /**
     * Actualiza el estado del LED (debe llamarse en loop())
     */
    void update();
    
    /**
     * Establece el estado del LED
     * @param state Nuevo estado del LED
     */
    void setState(LEDState state);
    
    /**
     * Enciende el LED
     */
    void turnOn();
    
    /**
     * Apaga el LED
     */
    void turnOff();
    
    /**
     * Alterna el estado del LED (on/off)
     */
    void toggle();
    
    /**
     * Obtiene el estado actual del LED
     * @return Estado actual
     */
    LEDState getState() const;
    
    /**
     * Verifica si el LED está encendido
     * @return true si el LED está encendido
     */
    bool isOn() const;
    
    /**
     * Obtiene el estado como string (para web interface)
     * @return "ON" o "OFF"
     */
    String getStateString() const;
};

#endif // LEDCONTROLLER_H