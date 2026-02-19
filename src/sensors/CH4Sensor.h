#ifndef CH4SENSOR_H
#define CH4SENSOR_H

#include <Arduino.h>

// Estados del sensor
enum class CH4State {
    INITIALIZING,   // Inicializando/calentando
    NORMAL,         // Aire limpio
    DETECTED,       // Metano detectado
    CRITICAL,       // Nivel crítico de metano
    EXPLOSIVE,      // Nivel explosivo (> 5% LEL)
    ERROR           // Error de lectura
};

// Estructura de calibración
struct CH4Calibration {
    int baselineMin;        // Valor mínimo en aire limpio
    int baselineMax;        // Valor máximo en aire limpio
    int baselineAvg;        // Promedio del baseline
    int thresholdCaution;   // Umbral de precaución (~0.5% LEL)
    int thresholdWarning;   // Umbral de advertencia (~2% LEL)
    int thresholdAlarm;     // Umbral de alarma (~4% LEL)
    int thresholdExplosive; // Umbral explosivo (~5% LEL)
    bool isCalibrated;      // ¿Está calibrado?
};

// Estructura de lectura del sensor
struct CH4Reading {
    int rawValue;           // Valor crudo ADC (0-4095)
    float voltage;          // Voltaje (0-3.3V)
    int percentage;         // Porcentaje (0-100%)
    int ppm;                // Partes por millón
    float lel;              // Lower Explosive Limit percentage (0-100%)
    CH4State state;         // Estado actual
    unsigned long timestamp; // Timestamp de la lectura
};

class CH4Sensor {
private:
    static CH4Sensor* instance;
    int pin;
    CH4Calibration calibration;
    CH4Reading lastReading;
    
    // Variables de warmup (CH4 necesita más tiempo)
    unsigned long warmupStartTime;
    bool isWarmedUp;
    static const unsigned long WARMUP_TIME = 180000; // 3 minutos
    
    // Variables de lectura suavizada
    static const int SAMPLE_SIZE = 15;  // Más muestras para CH4
    int readings[SAMPLE_SIZE];
    int readIndex;
    long total;
    
    CH4Sensor(int sensorPin); // Constructor privado
    
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
     * Estima LEL (Lower Explosive Limit) percentage
     * CH4 LEL = 5% en aire (50,000 PPM)
     */
    float rawToLEL(int raw);
    
    /**
     * Determina el estado basado en el valor
     */
    CH4State determineState(int raw);
    
public:
    /**
     * Obtiene la instancia única de CH4Sensor (Singleton)
     * @param sensorPin Pin GPIO conectado a la salida analógica
     * @return Puntero a la instancia de CH4Sensor
     */
    static CH4Sensor* getInstance(int sensorPin = 39);
    
    /**
     * Inicializa el sensor
     * @param enableWarmup Si true, esperará el tiempo de calentamiento (3 min)
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
    CH4Reading read();
    
    /**
     * Verifica si el sensor está listo (warmed up)
     * @return true si el sensor está listo para usar
     */
    bool isReady() const;
    
    /**
     * Obtiene la última lectura sin leer nuevamente
     * @return Última lectura almacenada
     */
    CH4Reading getLastReading() const;
    
    /**
     * Obtiene el estado actual del sensor
     * @return Estado actual
     */
    CH4State getState() const;
    
    /**
     * Obtiene la calibración actual
     * @return Estructura de calibración
     */
    CH4Calibration getCalibration() const;
    
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
     * Verifica si hay metano detectado (cualquier nivel)
     * @return true si hay metano
     */
    bool isCH4Detected() const;
    
    /**
     * Verifica si es nivel crítico
     * @return true si es crítico
     */
    bool isCritical() const;
    
    /**
     * Verifica si está en nivel explosivo (>5% LEL)
     * @return true si es nivel explosivo
     */
    bool isExplosive() const;
    
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
    
    /**
     * Obtiene LEL percentage del último reading
     * @return LEL% (0-100%, donde 100% = 5% CH4 en aire)
     */
    float getLEL() const;
};

#endif // CH4SENSOR_H