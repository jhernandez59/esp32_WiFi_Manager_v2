#include "EnvironmentSensor.h"
#include "../storage/FileManager.h"
#include "../config/Config.h"

// Ruta del archivo de baseline
#define ENV_BASELINE_PATH "/env_baseline.txt"

// Umbrales de detección
#define TEMP_HIGH_THRESHOLD 40.0        // °C - Temperatura alta
#define TEMP_CRITICAL_THRESHOLD 60.0    // °C - Temperatura crítica
#define TEMP_RATE_THRESHOLD 5.0         // °C/min - Subida rápida
#define HUMIDITY_LOW_THRESHOLD 20.0     // % - Humedad baja
#define HUMIDITY_HIGH_THRESHOLD 80.0    // % - Humedad alta
#define PRESSURE_DROP_THRESHOLD -5.0    // hPa - Caída significativa

// Inicializar instancia estática
EnvironmentSensor* EnvironmentSensor::instance = nullptr;

EnvironmentSensor::EnvironmentSensor() 
    : aht20Ready(false),
      bmp280Ready(false) {
    
    // Inicializar tendencias
    tempTrend.index = 0;
    pressureTrend.index = 0;
    for (int i = 0; i < 10; i++) {
        tempTrend.values[i] = 0;
        tempTrend.times[i] = 0;
        pressureTrend.values[i] = 0;
        pressureTrend.times[i] = 0;
    }
    
    // Baseline por defecto
    resetBaseline();
}

EnvironmentSensor* EnvironmentSensor::getInstance() {
    if (instance == nullptr) {
        instance = new EnvironmentSensor();
    }
    return instance;
}

bool EnvironmentSensor::initAHT20() {
    if (DEBUG_SERIAL) {
        Serial.print("  Inicializando AHT20... ");
    }
    
    if (aht20.begin()) {
        aht20Ready = true;
        if (DEBUG_SERIAL) {
            Serial.println("✓ OK");
        }
        return true;
    } else {
        aht20Ready = false;
        if (DEBUG_SERIAL) {
            Serial.println("❌ FALLO");
        }
        return false;
    }
}

bool EnvironmentSensor::initBMP280() {
    if (DEBUG_SERIAL) {
        Serial.print("  Inicializando BMP280... ");
    }
    
    // Probar dirección 0x76 primero, luego 0x77
    if (bmp280.begin(0x76) || bmp280.begin(0x77)) {
        bmp280Ready = true;
        
        // Configurar BMP280
        bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Modo operación
                          Adafruit_BMP280::SAMPLING_X2,     // Temp oversampling
                          Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                          Adafruit_BMP280::FILTER_X16,      // Filtrado
                          Adafruit_BMP280::STANDBY_MS_500); // Standby time
        
        if (DEBUG_SERIAL) {
            Serial.println("✓ OK");
        }
        return true;
    } else {
        bmp280Ready = false;
        if (DEBUG_SERIAL) {
            Serial.println("❌ FALLO");
        }
        return false;
    }
}

bool EnvironmentSensor::begin(int sdaPin, int sclPin) {
    if (DEBUG_SERIAL) {
        Serial.println("\n╔═══════════════════════════════════════╗");
        Serial.println("║   INICIALIZANDO SENSORES AMBIENTALES  ║");
        Serial.println("╚═══════════════════════════════════════╝");
    }
    
    // Inicializar I2C
    Wire.begin(sdaPin, sclPin);
    
    if (DEBUG_SERIAL) {
        Serial.printf("I2C configurado: SDA=%d, SCL=%d\n\n", sdaPin, sclPin);
    }
    
    // Inicializar sensores
    bool ahtOK = initAHT20();
    bool bmpOK = initBMP280();
    
    if (!ahtOK && !bmpOK) {
        if (DEBUG_SERIAL) {
            Serial.println("\n❌ Error: Ningún sensor ambiental detectado");
            Serial.println("   Verificar conexiones I2C");
        }
        return false;
    }
    
    // Cargar o establecer baseline
    if (loadBaseline()) {
        if (DEBUG_SERIAL) {
            Serial.println("\n✓ Baseline cargado desde archivo");
        }
    } else {
        if (DEBUG_SERIAL) {
            Serial.println("\n⚠ Usando baseline por defecto");
            Serial.println("  Recomendación: Ejecutar calibrateBaseline()");
        }
    }
    
    // Primera lectura
    lastReading = read();
    
    if (DEBUG_SERIAL) {
        Serial.println("\n✓ Sensores ambientales inicializados");
        if (ahtOK && bmpOK) {
            Serial.println("  Estado: AHT20 ✓ + BMP280 ✓");
        } else if (ahtOK) {
            Serial.println("  Estado: Solo AHT20 (Temp + Humedad)");
        } else if (bmpOK) {
            Serial.println("  Estado: Solo BMP280 (Temp + Presión)");
        }
        Serial.println("══════════════════════════════════════════\n");
    }
    
    return true;
}

void EnvironmentSensor::addToTrend(TrendData& trend, float value, unsigned long time) {
    trend.values[trend.index] = value;
    trend.times[trend.index] = time;
    trend.index = (trend.index + 1) % 10;
}

float EnvironmentSensor::calculateRate(const TrendData& trend) {
    // Calcular tasa de cambio (por minuto)
    int oldestIndex = trend.index;
    int newestIndex = (trend.index - 1 + 10) % 10;
    
    float valueDelta = trend.values[newestIndex] - trend.values[oldestIndex];
    unsigned long timeDelta = trend.times[newestIndex] - trend.times[oldestIndex];
    
    if (timeDelta == 0) return 0.0;
    
    // Convertir a por minuto
    float ratePerMs = valueDelta / (float)timeDelta;
    return ratePerMs * 60000.0;  // ms a minutos
}

bool EnvironmentSensor::detectRapidTempRise() {
    return (lastReading.tempRate > TEMP_RATE_THRESHOLD);
}

bool EnvironmentSensor::detectPressureDrop() {
    return (lastReading.pressureDelta < PRESSURE_DROP_THRESHOLD);
}

EnvironmentState EnvironmentSensor::evaluateState() {
    // Verificar si hay lecturas válidas
    if (!aht20Ready && !bmp280Ready) {
        return EnvironmentState::ERROR;
    }
    
    // Patrón de incendio sospechoso
    // Temperatura alta + caída de presión + humedad baja
    if (lastReading.tempDelta > 20 && 
        lastReading.pressureDelta < -3 && 
        lastReading.humidityDelta < -20) {
        return EnvironmentState::FIRE_SUSPECTED;
    }
    
    // Subida rápida de temperatura
    if (detectRapidTempRise() && lastReading.tempDelta > 10) {
        return EnvironmentState::RAPID_TEMP_RISE;
    }
    
    // Caída de presión significativa
    if (detectPressureDrop()) {
        return EnvironmentState::PRESSURE_DROP;
    }
    
    // Temperatura alta
    if (lastReading.tempDelta > 15) {
        return EnvironmentState::HIGH_TEMP;
    }
    
    // Humedad muy baja (aire seco - riesgo incendio)
    if (lastReading.humidity < HUMIDITY_LOW_THRESHOLD) {
        return EnvironmentState::LOW_HUMIDITY;
    }
    
    // Humedad muy alta (vapor/cocina)
    if (lastReading.humidity > HUMIDITY_HIGH_THRESHOLD) {
        return EnvironmentState::HIGH_HUMIDITY;
    }
    
    return EnvironmentState::NORMAL;
}

EnvironmentReading EnvironmentSensor::read() {
    EnvironmentReading reading;
    reading.timestamp = millis();
    
    // Leer AHT20 (Temperatura + Humedad)
    if (aht20Ready) {
        sensors_event_t humidity_event, temp_event;
        aht20.getEvent(&humidity_event, &temp_event);
        
        reading.temperature = temp_event.temperature;
        reading.humidity = humidity_event.relative_humidity;
    } else {
        reading.temperature = 0;
        reading.humidity = 0;
    }
    
    // Leer BMP280 (Presión + Temperatura)
    if (bmp280Ready) {
        reading.temperatureBMP = bmp280.readTemperature();
        reading.pressure = bmp280.readPressure() / 100.0;  // Pa a hPa
        reading.altitude = bmp280.readAltitude(1013.25);   // Nivel del mar estándar
    } else {
        reading.temperatureBMP = 0;
        reading.pressure = 0;
        reading.altitude = 0;
    }
    
    // Calcular deltas con baseline
    reading.tempDelta = reading.temperature - baseline.temperature;
    reading.humidityDelta = reading.humidity - baseline.humidity;
    reading.pressureDelta = reading.pressure - baseline.pressure;
    
    // Agregar a tendencias
    addToTrend(tempTrend, reading.temperature, reading.timestamp);
    addToTrend(pressureTrend, reading.pressure, reading.timestamp);
    
    // Calcular tasas de cambio
    reading.tempRate = calculateRate(tempTrend);
    
    // Evaluar estado
    reading.state = evaluateState();
    
    // Guardar como última lectura
    lastReading = reading;
    
    return reading;
}

bool EnvironmentSensor::calibrateBaseline() {
    if (!isReady()) {
        if (DEBUG_SERIAL) {
            Serial.println("❌ No se puede calibrar: sensores no listos");
        }
        return false;
    }
    
    if (DEBUG_SERIAL) {
        Serial.println("\n╔═══════════════════════════════════════╗");
        Serial.println("║    CALIBRANDO BASELINE AMBIENTAL      ║");
        Serial.println("╚═══════════════════════════════════════╝");
        Serial.println("Tomando 100 muestras en 5 minutos...");
        Serial.println("Asegúrate de condiciones normales\n");
    }
    
    float tempSum = 0;
    float humiditySum = 0;
    float pressureSum = 0;
    int samples = 100;
    int delayMs = 3000;  // 3 segundos entre muestras
    
    for (int i = 0; i < samples; i++) {
        EnvironmentReading reading = read();
        
        tempSum += reading.temperature;
        humiditySum += reading.humidity;
        pressureSum += reading.pressure;
        
        if (DEBUG_SERIAL && (i % 10 == 0)) {
            Serial.printf("Progreso: %d%% (T:%.1f H:%.1f P:%.1f)\n", 
                         (i * 100) / samples,
                         reading.temperature,
                         reading.humidity,
                         reading.pressure);
        }
        
        delay(delayMs);
    }
    
    // Calcular promedios
    baseline.temperature = tempSum / samples;
    baseline.humidity = humiditySum / samples;
    baseline.pressure = pressureSum / samples;
    baseline.timestamp = millis();
    baseline.isCalibrated = true;
    
    if (DEBUG_SERIAL) {
        Serial.println("\n✓ Calibración completada:");
        Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
        Serial.printf("  Temperatura:  %.2f °C\n", baseline.temperature);
        Serial.printf("  Humedad:      %.2f %%\n", baseline.humidity);
        Serial.printf("  Presión:      %.2f hPa\n", baseline.pressure);
        Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    }
    
    // Guardar baseline
    saveBaseline();
    
    return true;
}

void EnvironmentSensor::setBaseline(float temp, float humidity, float pressure) {
    baseline.temperature = temp;
    baseline.humidity = humidity;
    baseline.pressure = pressure;
    baseline.timestamp = millis();
    baseline.isCalibrated = true;
}

bool EnvironmentSensor::loadBaseline() {
    FileManager* fm = FileManager::getInstance();
    
    if (!fm->exists(ENV_BASELINE_PATH)) {
        return false;
    }
    
    String data = fm->readFile(ENV_BASELINE_PATH);
    if (data.length() == 0) {
        return false;
    }
    
    // Parse formato: temp,humidity,pressure
    float values[3];
    int index = 0;
    int start = 0;
    
    for (int i = 0; i <= data.length(); i++) {
        if (i == data.length() || data[i] == ',') {
            if (index < 3) {
                values[index++] = data.substring(start, i).toFloat();
                start = i + 1;
            }
        }
    }
    
    if (index == 3) {
        baseline.temperature = values[0];
        baseline.humidity = values[1];
        baseline.pressure = values[2];
        baseline.isCalibrated = true;
        return true;
    }
    
    return false;
}

bool EnvironmentSensor::saveBaseline() {
    FileManager* fm = FileManager::getInstance();
    
    // Formato: temp,humidity,pressure
    String data = String(baseline.temperature, 2) + "," +
                  String(baseline.humidity, 2) + "," +
                  String(baseline.pressure, 2);
    
    bool result = fm->writeFile(ENV_BASELINE_PATH, data);
    
    if (DEBUG_SERIAL) {
        if (result) {
            Serial.println("✓ Baseline guardado en LittleFS");
        } else {
            Serial.println("❌ Error al guardar baseline");
        }
    }
    
    return result;
}

void EnvironmentSensor::resetBaseline() {
    baseline.temperature = 25.0;   // 25°C
    baseline.humidity = 50.0;      // 50%
    baseline.pressure = 1013.25;   // hPa nivel del mar
    baseline.timestamp = 0;
    baseline.isCalibrated = false;
}

EnvironmentReading EnvironmentSensor::getLastReading() const {
    return lastReading;
}

EnvironmentBaseline EnvironmentSensor::getBaseline() const {
    return baseline;
}

bool EnvironmentSensor::isAHT20Ready() const {
    return aht20Ready;
}

bool EnvironmentSensor::isBMP280Ready() const {
    return bmp280Ready;
}

bool EnvironmentSensor::isReady() const {
    return (aht20Ready || bmp280Ready);
}

EnvironmentState EnvironmentSensor::getState() const {
    return lastReading.state;
}

String EnvironmentSensor::getStateString() const {
    switch (lastReading.state) {
        case EnvironmentState::NORMAL:           return "NORMAL";
        case EnvironmentState::HIGH_TEMP:        return "HIGH_TEMP";
        case EnvironmentState::RAPID_TEMP_RISE:  return "RAPID_TEMP_RISE";
        case EnvironmentState::LOW_HUMIDITY:     return "LOW_HUMIDITY";
        case EnvironmentState::HIGH_HUMIDITY:    return "HIGH_HUMIDITY";
        case EnvironmentState::PRESSURE_DROP:    return "PRESSURE_DROP";
        case EnvironmentState::FIRE_SUSPECTED:   return "FIRE_SUSPECTED";
        case EnvironmentState::ERROR:            return "ERROR";
        default:                                 return "UNKNOWN";
    }
}

bool EnvironmentSensor::isFireSuspected() const {
    return (lastReading.state == EnvironmentState::FIRE_SUSPECTED ||
            lastReading.state == EnvironmentState::RAPID_TEMP_RISE);
}

bool EnvironmentSensor::isHighTemperature() const {
    return (lastReading.tempDelta > 15);
}

bool EnvironmentSensor::isPressureDrop() const {
    return (lastReading.pressureDelta < PRESSURE_DROP_THRESHOLD);
}

float EnvironmentSensor::getTemperature() const {
    // Promedio de ambos sensores si están disponibles
    if (aht20Ready && bmp280Ready) {
        return (lastReading.temperature + lastReading.temperatureBMP) / 2.0;
    } else if (aht20Ready) {
        return lastReading.temperature;
    } else if (bmp280Ready) {
        return lastReading.temperatureBMP;
    }
    return 0;
}

float EnvironmentSensor::getHumidity() const {
    return lastReading.humidity;
}

float EnvironmentSensor::getPressure() const {
    return lastReading.pressure;
}

float EnvironmentSensor::getTempRate() const {
    return lastReading.tempRate;
}

float EnvironmentSensor::calculateFireProbability() const {
    float probability = 0.0;
    
    // Factor 1: Temperatura absoluta y delta
    if (lastReading.temperature > TEMP_CRITICAL_THRESHOLD) {
        probability += 0.5;  // Muy alta
    } else if (lastReading.temperature > TEMP_HIGH_THRESHOLD) {
        probability += 0.3;  // Alta
    } else if (lastReading.tempDelta > 15) {
        probability += 0.2;  // Subida moderada
    }
    
    // Factor 2: Tasa de cambio de temperatura
    if (lastReading.tempRate > 10) {
        probability += 0.4;  // Subida muy rápida
    } else if (lastReading.tempRate > TEMP_RATE_THRESHOLD) {
        probability += 0.2;  // Subida rápida
    }
    
    // Factor 3: Humedad baja (fuego seca el aire)
    if (lastReading.humidity < 20) {
        probability += 0.2;
    } else if (lastReading.humidity < 30) {
        probability += 0.1;
    }
    
    // Factor 4: Caída de presión (aire caliente sube)
    if (lastReading.pressureDelta < -5) {
        probability += 0.3;
    } else if (lastReading.pressureDelta < -3) {
        probability += 0.15;
    }
    
    // Bonus: Correlación de múltiples factores
    int activeFactors = 0;
    if (lastReading.tempDelta > 15) activeFactors++;
    if (lastReading.tempRate > TEMP_RATE_THRESHOLD) activeFactors++;
    if (lastReading.humidity < 30) activeFactors++;
    if (lastReading.pressureDelta < -3) activeFactors++;
    
    if (activeFactors >= 3) {
        probability += 0.3;  // Múltiples indicadores
    } else if (activeFactors >= 2) {
        probability += 0.15;
    }
    
    // Limitar a 0.0 - 1.0
    return constrain(probability, 0.0, 1.0);
}