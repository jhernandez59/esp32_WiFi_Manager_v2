#ifndef SMOKESENSOR_H
#define SMOKESENSOR_H

#include <Arduino.h>

// Estados del sensor
enum class SmokeState {
    INITIALIZING,   // Inicializando/calentando
    NORMAL,         // Aire limpio
    DETECTED,       // Humo detectado
    CRITICAL,       // Nivel crítico de humo
    ERROR           // Error de lectura
};

// Estructura de calibración
struct SmokeCalibration {
    int baselineMin;        // Valor mínimo en aire limpio
    int baselineMax;        // Valor máximo en aire limpio
    int baselineAvg;        // Promedio del baseline
    int thresholdCaution;   // Umbral de precaución
    int thresholdWarning;   // Umbral de advertencia
    int thresholdAlarm;     // Umbral de alarma
    bool isCalibrated;      // ¿Está calibrado?
};

// Estructura de lectura del sensor
struct SmokeReading {
    int rawValue;           // Valor crudo ADC (0-4095)
    float voltage;          // Voltaje (0-3.3V)
    int percentage;         // Porcentaje (0-100%)
    int ppm;                // Partes por millón (estimado)
    SmokeState state;       // Estado actual
    unsigned long timestamp; // Timestamp de la lectura
};

class SmokeSensor {
private:
    static SmokeSensor* instance;
    int pin;
    SmokeCalibration calibration;
    SmokeReading lastReading;
    
    // Variables de warmup
    unsigned long warmupStartTime;
    bool isWarmedUp;
    static const unsigned long WARMUP_TIME = 60000; // 60 segundos
    
    // Variables de lectura suavizada
    static const int SAMPLE_SIZE = 10;
    int readings[SAMPLE_SIZE];
    int readIndex;
    long total;
    
    SmokeSensor(int sensorPin); // Constructor privado
    
    /**
     * Lee el valor crudo del ADC con promediado
     */
    int readRawValue();
    
    /**
     * Convierte valor ADC a voltaje
     */
    float rawToVoltage(int raw);
    
    /**
     * Convierte valor ADC a porcentaje
     */
    int rawToPercentage(int raw);
    
    /**
     * Estima PPM basado en calibración
     */
    int rawToPPM(int raw);
    
    /**
     * Determina el estado basado en el valor
     */
    SmokeState determineState(int raw);
    
public:
    /**
     * Obtiene la instancia única de SmokeSensor (Singleton)
     * @param sensorPin Pin GPIO conectado a la salida analógica
     * @return Puntero a la instancia de SmokeSensor
     */
    static SmokeSensor* getInstance(int sensorPin = 36);
    
    /**
     * Inicializa el sensor
     * @param enableWarmup Si true, esperará el tiempo de calentamiento
     * @return true si se inicializó correctamente
     */
    bool begin(bool enableWarmup = true);
    
    /**
     * Calibra el sensor en aire limpio
     * @param samples Número de muestras a tomar
     * @param delayMs Delay entre muestras
     * @return true si la calibración fue exitosa
     */
    bool calibrate(int samples = 300, int delayMs = 1000);
    
    /**
     * Lee el sensor y actualiza el estado
     * @return Estructura con la lectura completa
     */
    SmokeReading read();
    
    /**
     * Verifica si el sensor está listo (warmed up)
     * @return true si el sensor está listo para usar
     */
    bool isReady() const;
    
    /**
     * Obtiene la última lectura sin leer nuevamente
     * @return Última lectura almacenada
     */
    SmokeReading getLastReading() const;
    
    /**
     * Obtiene el estado actual del sensor
     * @return Estado actual
     */
    SmokeState getState() const;
    
    /**
     * Obtiene la calibración actual
     * @return Estructura de calibración
     */
    SmokeCalibration getCalibration() const;
    
    /**
     * Carga calibración desde archivo (si existe)
     * @return true si se cargó correctamente
     */
    bool loadCalibration();
    
    /**
     * Guarda calibración en archivo
     * @return true si se guardó correctamente
     */
    bool saveCalibration();
    
    /**
     * Resetea la calibración a valores por defecto
     */
    void resetCalibration();
    
    /**
     * Obtiene el estado como string
     * @return Texto descriptivo del estado
     */
    String getStateString() const;
    
    /**
     * Verifica si hay humo detectado (cualquier nivel)
     * @return true si hay humo
     */
    bool isSmokeDetected() const;
    
    /**
     * Verifica si es nivel crítico
     * @return true si es crítico
     */
    bool isCritical() const;
    
    /**
     * Obtiene el porcentaje del último reading
     * @return Porcentaje 0-100
     */
    int getPercentage() const;
    
    /**
     * Obtiene PPM del último reading
     * @return PPM estimado
     */
    int getPPM() const;
};

#endif // SMOKESENSOR_H