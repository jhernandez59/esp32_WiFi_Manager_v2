#ifndef ENVIRONMENTSENSOR_H
#define ENVIRONMENTSENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>

// Estados del sensor ambiental
enum class EnvironmentState {
    NORMAL,             // Condiciones normales
    HIGH_TEMP,          // Temperatura alta
    RAPID_TEMP_RISE,    // Subida rápida de temperatura
    LOW_HUMIDITY,       // Humedad baja (riesgo incendio)
    HIGH_HUMIDITY,      // Humedad alta (vapor/cocina)
    PRESSURE_DROP,      // Caída de presión (aire caliente)
    FIRE_SUSPECTED,     // Patrón de incendio detectado
    ERROR               // Error de lectura
};

// Estructura de lectura ambiental
struct EnvironmentReading {
    // Temperatura
    float temperature;          // °C (de AHT20)
    float temperatureBMP;       // °C (de BMP280 - validación)
    float tempDelta;            // Diferencia con baseline
    float tempRate;             // °C/min (tasa de cambio)
    
    // Humedad
    float humidity;             // % (de AHT20)
    float humidityDelta;        // Diferencia con baseline
    
    // Presión
    float pressure;             // hPa (de BMP280)
    float pressureDelta;        // Diferencia con baseline hPa
    float altitude;             // Metros (calculado)
    
    // Estado
    EnvironmentState state;     // Estado evaluado
    unsigned long timestamp;    // Timestamp de lectura
};

// Estructura de baseline (valores normales)
struct EnvironmentBaseline {
    float temperature;
    float humidity;
    float pressure;
    unsigned long timestamp;
    bool isCalibrated;
};

// Estructura para detección de tendencias
struct TrendData {
    float values[10];       // Últimas 10 lecturas
    int index;              // Índice actual
    unsigned long times[10]; // Timestamps
};

class EnvironmentSensor {
private:
    static EnvironmentSensor* instance;
    
    Adafruit_AHTX0 aht20;
    Adafruit_BMP280 bmp280;
    
    EnvironmentReading lastReading;
    EnvironmentBaseline baseline;
    
    // Tendencias
    TrendData tempTrend;
    TrendData pressureTrend;
    
    bool aht20Ready;
    bool bmp280Ready;
    
    EnvironmentSensor(); // Constructor privado
    
    /**
     * Inicializa el sensor AHT20
     */
    bool initAHT20();
    
    /**
     * Inicializa el sensor BMP280
     */
    bool initBMP280();
    
    /**
     * Agrega un valor a la tendencia
     */
    void addToTrend(TrendData& trend, float value, unsigned long time);
    
    /**
     * Calcula la tasa de cambio (unidad/minuto)
     */
    float calculateRate(const TrendData& trend);
    
    /**
     * Detecta subida rápida de temperatura
     */
    bool detectRapidTempRise();
    
    /**
     * Detecta caída de presión significativa
     */
    bool detectPressureDrop();
    
    /**
     * Evalúa el estado basado en las lecturas
     */
    EnvironmentState evaluateState();
    
public:
    /**
     * Obtiene la instancia única de EnvironmentSensor (Singleton)
     * @return Puntero a la instancia
     */
    static EnvironmentSensor* getInstance();
    
    /**
     * Inicializa los sensores ambientales
     * @param sdaPin Pin SDA para I2C (default: 21)
     * @param sclPin Pin SCL para I2C (default: 22)
     * @return true si al menos uno se inicializó
     */
    bool begin(int sdaPin = 21, int sclPin = 22);
    
    /**
     * Lee todos los sensores ambientales
     * @return Estructura con todas las lecturas
     */
    EnvironmentReading read();
    
    /**
     * Establece el baseline (valores normales)
     * Toma 100 muestras en 5 minutos
     * @return true si se estableció correctamente
     */
    bool calibrateBaseline();
    
    /**
     * Establece baseline manualmente
     */
    void setBaseline(float temp, float humidity, float pressure);
    
    /**
     * Carga baseline desde archivo
     */
    bool loadBaseline();
    
    /**
     * Guarda baseline en archivo
     */
    bool saveBaseline();
    
    /**
     * Resetea baseline a valores por defecto
     */
    void resetBaseline();
    
    /**
     * Obtiene la última lectura
     */
    EnvironmentReading getLastReading() const;
    
    /**
     * Obtiene el baseline actual
     */
    EnvironmentBaseline getBaseline() const;
    
    /**
     * Verifica si el sensor AHT20 está listo
     */
    bool isAHT20Ready() const;
    
    /**
     * Verifica si el sensor BMP280 está listo
     */
    bool isBMP280Ready() const;
    
    /**
     * Verifica si ambos sensores están listos
     */
    bool isReady() const;
    
    /**
     * Obtiene el estado actual
     */
    EnvironmentState getState() const;
    
    /**
     * Obtiene el estado como string
     */
    String getStateString() const;
    
    /**
     * Verifica si hay condiciones de incendio sospechosas
     */
    bool isFireSuspected() const;
    
    /**
     * Verifica si hay temperatura anormalmente alta
     */
    bool isHighTemperature() const;
    
    /**
     * Verifica si hay caída de presión significativa
     */
    bool isPressureDrop() const;
    
    /**
     * Obtiene temperatura (promedio AHT20 y BMP280)
     */
    float getTemperature() const;
    
    /**
     * Obtiene humedad
     */
    float getHumidity() const;
    
    /**
     * Obtiene presión
     */
    float getPressure() const;
    
    /**
     * Obtiene tasa de cambio de temperatura (°C/min)
     */
    float getTempRate() const;
    
    /**
     * Calcula probabilidad de incendio (0.0 - 1.0)
     * Basado en múltiples factores ambientales
     */
    float calculateFireProbability() const;
};

#endif // ENVIRONMENTSENSOR_H