#include "SmokeSensor.h"
#include "../storage/FileManager.h"
#include "../config/Config.h"

// Ruta del archivo de calibración
#define SMOKE_CAL_PATH "/smoke_cal.txt"

// Inicializar instancia estática
SmokeSensor* SmokeSensor::instance = nullptr;

SmokeSensor::SmokeSensor(int sensorPin) 
    : pin(sensorPin),
      isWarmedUp(false),
      warmupStartTime(0),
      readIndex(0),
      total(0) {
    
    // Inicializar array de lecturas
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        readings[i] = 0;
    }
    
    // Valores por defecto de calibración
    resetCalibration();
}

SmokeSensor* SmokeSensor::getInstance(int sensorPin) {
    if (instance == nullptr) {
        instance = new SmokeSensor(sensorPin);
    }
    return instance;
}

void SmokeSensor::resetCalibration() {
    calibration.baselineMin = 0;
    calibration.baselineMax = 500;
    calibration.baselineAvg = 250;
    calibration.thresholdCaution = 400;
    calibration.thresholdWarning = 800;
    calibration.thresholdAlarm = 1500;
    calibration.isCalibrated = false;
}

bool SmokeSensor::begin(bool enableWarmup) {
    // Configurar ADC
    analogSetAttenuation(ADC_11db);  // Rango completo 0-3.3V
    analogReadResolution(12);         // 12 bits (0-4095)
    
    if (DEBUG_SERIAL) {
        Serial.println("\n╔═══════════════════════════════════╗");
        Serial.println("║    INICIALIZANDO SMOKE SENSOR     ║");
        Serial.println("╚═══════════════════════════════════╝");
        Serial.printf("Pin: GPIO %d\n", pin);
    }
    
    // Intentar cargar calibración guardada
    if (loadCalibration()) {
        if (DEBUG_SERIAL) {
            Serial.println("✓ Calibración cargada desde archivo");
        }
    } else {
        if (DEBUG_SERIAL) {
            Serial.println("⚠ Usando calibración por defecto");
            Serial.println("  Recomendación: Ejecutar calibración");
        }
    }
    
    // Iniciar warmup si está habilitado
    if (enableWarmup) {
        warmupStartTime = millis();
        isWarmedUp = false;
        
        if (DEBUG_SERIAL) {
            Serial.printf("⏳ Calentando sensor (%d segundos)...\n", WARMUP_TIME / 1000);
        }
    } else {
        isWarmedUp = true;
    }
    
    // Primera lectura
    lastReading = read();
    
    if (DEBUG_SERIAL) {
        Serial.println("✓ Sensor inicializado\n");
    }
    
    return true;
}

int SmokeSensor::readRawValue() {
    // Remover la lectura más antigua
    total -= readings[readIndex];
    
    // Leer nuevo valor
    readings[readIndex] = analogRead(pin);
    
    // Agregar al total
    total += readings[readIndex];
    
    // Avanzar al siguiente índice
    readIndex = (readIndex + 1) % SAMPLE_SIZE;
    
    // Calcular promedio
    return total / SAMPLE_SIZE;
}

float SmokeSensor::rawToVoltage(int raw) {
    return (raw / 4095.0) * 3.3;
}

int SmokeSensor::rawToPercentage(int raw) {
    // Mapear basado en calibración
    if (!calibration.isCalibrated) {
        return map(raw, 0, 4095, 0, 100);
    }
    
    // Calcular porcentaje relativo al baseline
    int range = calibration.thresholdAlarm - calibration.baselineAvg;
    int value = raw - calibration.baselineAvg;
    
    if (value < 0) value = 0;
    if (value > range) value = range;
    
    return map(value, 0, range, 0, 100);
}

int SmokeSensor::rawToPPM(int raw) {
    // Estimación básica de PPM
    // Esto debe ajustarse según el datasheet específico del sensor
    if (!calibration.isCalibrated) {
        return map(raw, 0, 4095, 0, 1000);
    }
    
    // Calcular PPM basado en desviación del baseline
    int deviation = raw - calibration.baselineAvg;
    if (deviation < 0) deviation = 0;
    
    // Factor de conversión aproximado (ajustar según sensor)
    return deviation * 2;  // 1 unidad ADC ≈ 2 PPM (ejemplo)
}

SmokeState SmokeSensor::determineState(int raw) {
    if (!isWarmedUp) {
        return SmokeState::INITIALIZING;
    }
    
    if (raw < 0 || raw > 4095) {
        return SmokeState::ERROR;
    }
    
    if (raw >= calibration.thresholdAlarm) {
        return SmokeState::CRITICAL;
    }
    
    if (raw >= calibration.thresholdWarning) {
        return SmokeState::DETECTED;
    }
    
    return SmokeState::NORMAL;
}

SmokeReading SmokeSensor::read() {
    // Verificar warmup
    if (!isWarmedUp && (millis() - warmupStartTime >= WARMUP_TIME)) {
        isWarmedUp = true;
        if (DEBUG_SERIAL) {
            Serial.println("✓ Sensor calentado - Listo para usar");
        }
    }
    
    // Leer valor
    int raw = readRawValue();
    
    // Crear estructura de lectura
    SmokeReading reading;
    reading.rawValue = raw;
    reading.voltage = rawToVoltage(raw);
    reading.percentage = rawToPercentage(raw);
    reading.ppm = rawToPPM(raw);
    reading.state = determineState(raw);
    reading.timestamp = millis();
    
    // Guardar como última lectura
    lastReading = reading;
    
    return reading;
}

bool SmokeSensor::calibrate(int samples, int delayMs) {
    if (DEBUG_SERIAL) {
        Serial.println("\n╔═══════════════════════════════════╗");
        Serial.println("║     CALIBRANDO SMOKE SENSOR       ║");
        Serial.println("╚═══════════════════════════════════╝");
        Serial.println("⚠ IMPORTANTE: Asegúrate de estar en");
        Serial.println("  un ambiente con AIRE LIMPIO");
        Serial.printf("  Tomando %d muestras...\n\n", samples);
    }
    
    int minVal = 4095;
    int maxVal = 0;
    long sum = 0;
    
    for (int i = 0; i < samples; i++) {
        int reading = analogRead(pin);
        
        if (reading < minVal) minVal = reading;
        if (reading > maxVal) maxVal = reading;
        sum += reading;
        
        // Mostrar progreso cada 10%
        if (DEBUG_SERIAL && (i % (samples / 10) == 0)) {
            Serial.printf("Progreso: %d%% (Actual: %d)\n", (i * 100) / samples, reading);
        }
        
        delay(delayMs);
    }
    
    // Calcular baseline
    calibration.baselineMin = minVal;
    calibration.baselineMax = maxVal;
    calibration.baselineAvg = sum / samples;
    
    // Calcular umbrales automáticamente
    int range = maxVal - minVal;
    int variance = range / 2;
    
    calibration.thresholdCaution = calibration.baselineAvg + variance * 2;
    calibration.thresholdWarning = calibration.baselineAvg + variance * 4;
    calibration.thresholdAlarm = calibration.baselineAvg + variance * 8;
    
    calibration.isCalibrated = true;
    
    if (DEBUG_SERIAL) {
        Serial.println("\n✓ Calibración completada:");
        Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
        Serial.printf("  Baseline Min:    %d\n", calibration.baselineMin);
        Serial.printf("  Baseline Max:    %d\n", calibration.baselineMax);
        Serial.printf("  Baseline Avg:    %d\n", calibration.baselineAvg);
        Serial.printf("  Umbral Caution:  %d\n", calibration.thresholdCaution);
        Serial.printf("  Umbral Warning:  %d\n", calibration.thresholdWarning);
        Serial.printf("  Umbral Alarm:    %d\n", calibration.thresholdAlarm);
        Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    }
    
    // Guardar calibración
    saveCalibration();
    
    return true;
}

bool SmokeSensor::isReady() const {
    return isWarmedUp && calibration.isCalibrated;
}

SmokeReading SmokeSensor::getLastReading() const {
    return lastReading;
}

SmokeState SmokeSensor::getState() const {
    return lastReading.state;
}

SmokeCalibration SmokeSensor::getCalibration() const {
    return calibration;
}

bool SmokeSensor::loadCalibration() {
    FileManager* fm = FileManager::getInstance();
    
    if (!fm->exists(SMOKE_CAL_PATH)) {
        return false;
    }
    
    String data = fm->readFile(SMOKE_CAL_PATH);
    if (data.length() == 0) {
        return false;
    }
    
    // Parse formato: min,max,avg,caution,warning,alarm
    int values[6];
    int index = 0;
    int start = 0;
    
    for (int i = 0; i <= data.length(); i++) {
        if (i == data.length() || data[i] == ',') {
            if (index < 6) {
                values[index++] = data.substring(start, i).toInt();
                start = i + 1;
            }
        }
    }
    
    if (index == 6) {
        calibration.baselineMin = values[0];
        calibration.baselineMax = values[1];
        calibration.baselineAvg = values[2];
        calibration.thresholdCaution = values[3];
        calibration.thresholdWarning = values[4];
        calibration.thresholdAlarm = values[5];
        calibration.isCalibrated = true;
        return true;
    }
    
    return false;
}

bool SmokeSensor::saveCalibration() {
    FileManager* fm = FileManager::getInstance();
    
    // Formato: min,max,avg,caution,warning,alarm
    String data = String(calibration.baselineMin) + "," +
                  String(calibration.baselineMax) + "," +
                  String(calibration.baselineAvg) + "," +
                  String(calibration.thresholdCaution) + "," +
                  String(calibration.thresholdWarning) + "," +
                  String(calibration.thresholdAlarm);
    
    bool result = fm->writeFile(SMOKE_CAL_PATH, data);
    
    if (DEBUG_SERIAL) {
        if (result) {
            Serial.println("✓ Calibración guardada en LittleFS");
        } else {
            Serial.println("❌ Error al guardar calibración");
        }
    }
    
    return result;
}

String SmokeSensor::getStateString() const {
    switch (lastReading.state) {
        case SmokeState::INITIALIZING: return "INITIALIZING";
        case SmokeState::NORMAL:       return "NORMAL";
        case SmokeState::DETECTED:     return "DETECTED";
        case SmokeState::CRITICAL:     return "CRITICAL";
        case SmokeState::ERROR:        return "ERROR";
        default:                       return "UNKNOWN";
    }
}

bool SmokeSensor::isSmokeDetected() const {
    return (lastReading.state == SmokeState::DETECTED || 
            lastReading.state == SmokeState::CRITICAL);
}

bool SmokeSensor::isCritical() const {
    return lastReading.state == SmokeState::CRITICAL;
}

int SmokeSensor::getPercentage() const {
    return lastReading.percentage;
}

int SmokeSensor::getPPM() const {
    return lastReading.ppm;
}