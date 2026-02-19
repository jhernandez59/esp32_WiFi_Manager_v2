#include "CH4Sensor.h"
#include "../storage/FileManager.h"
#include "../config/Config.h"

// Ruta del archivo de calibración
#define CH4_CAL_PATH "/ch4_cal.txt"

// Constantes de CH4
#define CH4_LEL_PPM 50000  // 5% de CH4 en aire = Lower Explosive Limit

// Inicializar instancia estática
CH4Sensor* CH4Sensor::instance = nullptr;

CH4Sensor::CH4Sensor(int sensorPin) 
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

CH4Sensor* CH4Sensor::getInstance(int sensorPin) {
    if (instance == nullptr) {
        instance = new CH4Sensor(sensorPin);
    }
    return instance;
}

void CH4Sensor::resetCalibration() {
    calibration.baselineMin = 0;
    calibration.baselineMax = 500;
    calibration.baselineAvg = 250;
    calibration.thresholdCaution = 500;    // ~0.5% LEL
    calibration.thresholdWarning = 1000;   // ~2% LEL
    calibration.thresholdAlarm = 1800;     // ~4% LEL
    calibration.thresholdExplosive = 2200; // ~5% LEL
    calibration.isCalibrated = false;
}

bool CH4Sensor::begin(bool enableWarmup) {
    // Configurar ADC
    analogSetAttenuation(ADC_11db);  // Rango completo 0-3.3V
    analogReadResolution(12);         // 12 bits (0-4095)
    
    if (DEBUG_SERIAL) {
        Serial.println("\n╔═══════════════════════════════════╗");
        Serial.println("║      INICIALIZANDO CH4 SENSOR     ║");
        Serial.println("╚═══════════════════════════════════╝");
        Serial.printf("Pin: GPIO %d\n", pin);
        Serial.println("Tipo: Metano (CH4)");
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
            Serial.println("⚠ IMPORTANTE: Sensor CH4 requiere 3 minutos de calentamiento");
            Serial.printf("⏳ Calentando sensor (%d segundos)...\n", WARMUP_TIME / 1000);
        }
    } else {
        isWarmedUp = true;
    }
    
    // Primera lectura
    lastReading = read();
    
    if (DEBUG_SERIAL) {
        Serial.println("✓ Sensor inicializado");
        Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
        Serial.println("⚠ ADVERTENCIA DE SEGURIDAD:");
        Serial.println("  • CH4 es INFLAMABLE y EXPLOSIVO");
        Serial.println("  • LEL (Límite Explosivo) = 5% en aire");
        Serial.println("  • No usar llamas cerca del sensor");
        Serial.println("  • Ventilación adecuada requerida");
        Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    }
    
    return true;
}

int CH4Sensor::readRawValue() {
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

float CH4Sensor::rawToVoltage(int raw) {
    return (raw / 4095.0) * 3.3;
}

int CH4Sensor::rawToPercentage(int raw) {
    // Mapear basado en calibración
    if (!calibration.isCalibrated) {
        return map(raw, 0, 4095, 0, 100);
    }
    
    // Calcular porcentaje relativo al baseline
    int range = calibration.thresholdExplosive - calibration.baselineAvg;
    int value = raw - calibration.baselineAvg;
    
    if (value < 0) value = 0;
    if (value > range) value = range;
    
    return map(value, 0, range, 0, 100);
}

int CH4Sensor::rawToPPM(int raw) {
    // Estimación de PPM basada en calibración
    if (!calibration.isCalibrated) {
        return map(raw, 0, 4095, 0, CH4_LEL_PPM);
    }
    
    // Calcular PPM basado en desviación del baseline
    int deviation = raw - calibration.baselineAvg;
    if (deviation < 0) deviation = 0;
    
    // Mapear al rango de PPM (0 a LEL)
    int range = calibration.thresholdExplosive - calibration.baselineAvg;
    if (range <= 0) range = 1;
    
    return map(deviation, 0, range, 0, CH4_LEL_PPM);
}

float CH4Sensor::rawToLEL(int raw) {
    // LEL% = (PPM / LEL_PPM) * 100
    // Donde LEL_PPM = 50,000 (5% CH4 en aire)
    int ppm = rawToPPM(raw);
    return (ppm / (float)CH4_LEL_PPM) * 100.0;
}

CH4State CH4Sensor::determineState(int raw) {
    if (!isWarmedUp) {
        return CH4State::INITIALIZING;
    }
    
    if (raw < 0 || raw > 4095) {
        return CH4State::ERROR;
    }
    
    if (raw >= calibration.thresholdExplosive) {
        return CH4State::EXPLOSIVE;
    }
    
    if (raw >= calibration.thresholdAlarm) {
        return CH4State::CRITICAL;
    }
    
    if (raw >= calibration.thresholdWarning) {
        return CH4State::DETECTED;
    }
    
    return CH4State::NORMAL;
}

CH4Reading CH4Sensor::read() {
    // Verificar warmup
    if (!isWarmedUp) {
        unsigned long elapsed = millis() - warmupStartTime;
        
        if (elapsed >= WARMUP_TIME) {
            isWarmedUp = true;
            if (DEBUG_SERIAL) {
                Serial.println("✓ Sensor CH4 calentado - Listo para usar");
            }
        } else {
            // Mostrar progreso cada 30 segundos
            static unsigned long lastProgress = 0;
            if (millis() - lastProgress >= 30000) {
                lastProgress = millis();
                int remaining = (WARMUP_TIME - elapsed) / 1000;
                if (DEBUG_SERIAL) {
                    Serial.printf("⏳ Calentando... %d segundos restantes\n", remaining);
                }
            }
        }
    }
    
    // Leer valor
    int raw = readRawValue();
    
    // Crear estructura de lectura
    CH4Reading reading;
    reading.rawValue = raw;
    reading.voltage = rawToVoltage(raw);
    reading.percentage = rawToPercentage(raw);
    reading.ppm = rawToPPM(raw);
    reading.lel = rawToLEL(raw);
    reading.state = determineState(raw);
    reading.timestamp = millis();
    
    // Guardar como última lectura
    lastReading = reading;
    
    return reading;
}

bool CH4Sensor::calibrate(int samples, int delayMs) {
    if (DEBUG_SERIAL) {
        Serial.println("\n╔═══════════════════════════════════╗");
        Serial.println("║       CALIBRANDO CH4 SENSOR       ║");
        Serial.println("╚═══════════════════════════════════╝");
        Serial.println("⚠ MUY IMPORTANTE:");
        Serial.println("  • Calibrar en ambiente EXTERIOR o VENTILADO");
        Serial.println("  • AIRE LIMPIO sin fugas de gas");
        Serial.println("  • NO encender llamas durante calibración");
        Serial.println("  • Sensor debe estar calentado (3 min)");
        Serial.printf("  • Tomando %d muestras...\n\n", samples);
        
        if (!isWarmedUp) {
            Serial.println("❌ ERROR: Sensor no está calentado");
            Serial.println("   Espera 3 minutos antes de calibrar");
            return false;
        }
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
    
    // Calcular umbrales basados en LEL
    int range = maxVal - minVal;
    int variance = range / 2;
    
    // Umbrales más conservadores para CH4 (gas explosivo)
    calibration.thresholdCaution = calibration.baselineAvg + variance * 3;    // ~0.5% LEL
    calibration.thresholdWarning = calibration.baselineAvg + variance * 6;    // ~2% LEL
    calibration.thresholdAlarm = calibration.baselineAvg + variance * 10;     // ~4% LEL
    calibration.thresholdExplosive = calibration.baselineAvg + variance * 12; // ~5% LEL
    
    calibration.isCalibrated = true;
    
    if (DEBUG_SERIAL) {
        Serial.println("\n✓ Calibración completada:");
        Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
        Serial.printf("  Baseline Min:      %d\n", calibration.baselineMin);
        Serial.printf("  Baseline Max:      %d\n", calibration.baselineMax);
        Serial.printf("  Baseline Avg:      %d\n", calibration.baselineAvg);
        Serial.printf("  Umbral Caution:    %d (~0.5%% LEL)\n", calibration.thresholdCaution);
        Serial.printf("  Umbral Warning:    %d (~2%% LEL)\n", calibration.thresholdWarning);
        Serial.printf("  Umbral Alarm:      %d (~4%% LEL)\n", calibration.thresholdAlarm);
        Serial.printf("  Umbral Explosive:  %d (~5%% LEL)\n", calibration.thresholdExplosive);
        Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    }
    
    // Guardar calibración
    saveCalibration();
    
    return true;
}

bool CH4Sensor::isReady() const {
    return isWarmedUp && calibration.isCalibrated;
}

CH4Reading CH4Sensor::getLastReading() const {
    return lastReading;
}

CH4State CH4Sensor::getState() const {
    return lastReading.state;
}

CH4Calibration CH4Sensor::getCalibration() const {
    return calibration;
}

bool CH4Sensor::loadCalibration() {
    FileManager* fm = FileManager::getInstance();
    
    if (!fm->exists(CH4_CAL_PATH)) {
        return false;
    }
    
    String data = fm->readFile(CH4_CAL_PATH);
    if (data.length() == 0) {
        return false;
    }
    
    // Parse formato: min,max,avg,caution,warning,alarm,explosive
    int values[7];
    int index = 0;
    int start = 0;
    
    for (int i = 0; i <= data.length(); i++) {
        if (i == data.length() || data[i] == ',') {
            if (index < 7) {
                values[index++] = data.substring(start, i).toInt();
                start = i + 1;
            }
        }
    }
    
    if (index == 7) {
        calibration.baselineMin = values[0];
        calibration.baselineMax = values[1];
        calibration.baselineAvg = values[2];
        calibration.thresholdCaution = values[3];
        calibration.thresholdWarning = values[4];
        calibration.thresholdAlarm = values[5];
        calibration.thresholdExplosive = values[6];
        calibration.isCalibrated = true;
        return true;
    }
    
    return false;
}

bool CH4Sensor::saveCalibration() {
    FileManager* fm = FileManager::getInstance();
    
    // Formato: min,max,avg,caution,warning,alarm,explosive
    String data = String(calibration.baselineMin) + "," +
                  String(calibration.baselineMax) + "," +
                  String(calibration.baselineAvg) + "," +
                  String(calibration.thresholdCaution) + "," +
                  String(calibration.thresholdWarning) + "," +
                  String(calibration.thresholdAlarm) + "," +
                  String(calibration.thresholdExplosive);
    
    bool result = fm->writeFile(CH4_CAL_PATH, data);
    
    if (DEBUG_SERIAL) {
        if (result) {
            Serial.println("✓ Calibración CH4 guardada en LittleFS");
        } else {
            Serial.println("❌ Error al guardar calibración CH4");
        }
    }
    
    return result;
}

String CH4Sensor::getStateString() const {
    switch (lastReading.state) {
        case CH4State::INITIALIZING: return "INITIALIZING";
        case CH4State::NORMAL:       return "NORMAL";
        case CH4State::DETECTED:     return "DETECTED";
        case CH4State::CRITICAL:     return "CRITICAL";
        case CH4State::EXPLOSIVE:    return "EXPLOSIVE";
        case CH4State::ERROR:        return "ERROR";
        default:                     return "UNKNOWN";
    }
}

bool CH4Sensor::isCH4Detected() const {
    return (lastReading.state == CH4State::DETECTED || 
            lastReading.state == CH4State::CRITICAL ||
            lastReading.state == CH4State::EXPLOSIVE);
}

bool CH4Sensor::isCritical() const {
    return (lastReading.state == CH4State::CRITICAL ||
            lastReading.state == CH4State::EXPLOSIVE);
}

bool CH4Sensor::isExplosive() const {
    return lastReading.state == CH4State::EXPLOSIVE;
}

int CH4Sensor::getPercentage() const {
    return lastReading.percentage;
}

int CH4Sensor::getPPM() const {
    return lastReading.ppm;
}

float CH4Sensor::getLEL() const {
    return lastReading.lel;
}