// ============================================================
// SISTEMA COMPLETO DE ALARMA DE INCENDIO Y GAS
// Smoke + CH4 + Temperatura + Humedad + Presión
// ============================================================

#include <Arduino.h>
#include "config/Config.h"
#include "storage/FileManager.h"
#include "wifi/WiFiManager.h"
#include "led/LEDController.h"
#include "web/MyWebServer.h"
#include "ota/OTAManager.h"
#include "sensors/SmokeSensor.h"
#include "sensors/CH4Sensor.h"
#include "sensors/EnvironmentSensor.h"

// Instancias de módulos
FileManager* fileManager;
WiFiManager* wifiManager;
LEDController* ledController;
MyWebServer* webServer;
OTAManager* otaManager;

// Sensores
SmokeSensor* smokeSensor;
CH4Sensor* ch4Sensor;
EnvironmentSensor* envSensor;

// Control
unsigned long lastSensorRead = 0;

// Nivel de alerta global mejorado
enum GlobalAlertLevel {
    ALERT_NORMAL,           // Todo OK
    ALERT_COOKING,          // Vapor de cocina (falsa alarma evitada)
    ALERT_ANOMALY,          // Algo raro, monitorear
    ALERT_CAUTION,          // Precaución, sensor activado
    ALERT_WARNING,          // Advertencia, 2+ sensores
    ALERT_FIRE_SUSPECTED,   // Incendio probable
    ALERT_FIRE_CONFIRMED,   // Incendio confirmado
    ALERT_GAS_CRITICAL,     // Gas crítico
    ALERT_EXPLOSIVE         // Nivel explosivo
};

GlobalAlertLevel currentAlert = ALERT_NORMAL;

/**
 * Evaluación inteligente con múltiples sensores
 */
GlobalAlertLevel evaluateSmartAlert() {
    // Leer últimas lecturas
    SmokeReading smoke = smokeSensor->getLastReading();
    CH4Reading ch4 = ch4Sensor->getLastReading();
    EnvironmentReading env = envSensor->getLastReading();
    
    int activeSensors = 0;
    
    // ========== ANÁLISIS DE PATRONES ==========
    
    // PATRÓN 1: GAS + FUEGO = MÁXIMA PRIORIDAD
    if (ch4Sensor->isExplosive()) {
        return ALERT_EXPLOSIVE;  // 💥 EVACUAR
    }
    
    if (ch4Sensor->isCritical() && (smokeSensor->isSmokeDetected() || env.temperature > 45)) {
        return ALERT_GAS_CRITICAL;  // 🚨 Gas + indicios de fuego
    }
    
    // PATRÓN 2: INCENDIO CONFIRMADO
    // Múltiples indicadores concuerdan
    if (smokeSensor->isCritical() && 
        env.temperature > 60 && 
        env.pressureDelta < -5) {
        return ALERT_FIRE_CONFIRMED;  // 🔥 Incendio real
    }
    
    // PATRÓN 3: INCENDIO SOSPECHOSO/TEMPRANO
    // Ambiente sugiere incendio antes de humo denso
    if (envSensor->isFireSuspected() || 
        (env.temperature > 50 && env.tempRate > 5)) {
        if (smokeSensor->isSmokeDetected() || smoke.ppm > 300) {
            return ALERT_FIRE_SUSPECTED;  // ⚠️ Probable incendio
        }
    }
    
    // PATRÓN 4: COCINA/VAPOR (Evitar falsa alarma)
    // Humo detectado + humedad alta + temperatura normal
    if (smokeSensor->isSmokeDetected() && 
        env.humidity > 75 && 
        env.temperature < 35 &&
        !ch4Sensor->isCH4Detected()) {
        return ALERT_COOKING;  // 🍳 Solo vapor, no peligroso
    }
    
    // PATRÓN 5: MÚLTIPLES SENSORES ACTIVADOS
    if (smokeSensor->isSmokeDetected()) activeSensors++;
    if (ch4Sensor->isCH4Detected()) activeSensors++;
    if (env.temperature > 40) activeSensors++;
    if (env.pressureDelta < -3) activeSensors++;
    
    if (activeSensors >= 3) {
        return ALERT_WARNING;  // 🟠 Múltiples indicadores
    }
    
    if (activeSensors >= 2) {
        return ALERT_CAUTION;  // 🟡 Dos sensores
    }
    
    // PATRÓN 6: UN SOLO SENSOR ACTIVADO
    if (activeSensors == 1) {
        return ALERT_ANOMALY;  // 🤔 Monitorear
    }
    
    return ALERT_NORMAL;  // ✅ Todo bien
}

/**
 * Muestra estado detallado de todos los sensores
 */
void displayFullStatus() {
    Serial.println("\n╔═══════════════════════════════════════════════════════════════╗");
    Serial.println("║          SISTEMA INTELIGENTE DE ALARMA - ESTADO COMPLETO     ║");
    Serial.println("╠═══════════════════════════════════════════════════════════════╣");
    
    // SENSOR DE HUMO
    if (smokeSensor->isReady()) {
        SmokeReading smoke = smokeSensor->getLastReading();
        Serial.println("║  🔥 SENSOR DE HUMO:                                           ║");
        Serial.printf("║    Estado:      %-45s  ║\n", smokeSensor->getStateString().c_str());
        Serial.printf("║    PPM:         %-45d  ║\n", smoke.ppm);
        Serial.printf("║    Porcentaje:  %-44d%%  ║\n", smoke.percentage);
    } else {
        Serial.println("║  🔥 SENSOR DE HUMO: Inicializando...                         ║");
    }
    
    Serial.println("╠═══════════════════════════════════════════════════════════════╣");
    
    // SENSOR DE CH4
    if (ch4Sensor->isReady()) {
        CH4Reading ch4 = ch4Sensor->getLastReading();
        Serial.println("║  💨 SENSOR DE METANO (CH4):                                   ║");
        Serial.printf("║    Estado:      %-45s  ║\n", ch4Sensor->getStateString().c_str());
        Serial.printf("║    PPM:         %-45d  ║\n", ch4.ppm);
        Serial.printf("║    LEL:         %-43.2f%%  ║\n", ch4.lel);
    } else {
        Serial.println("║  💨 SENSOR DE METANO: Inicializando...                       ║");
    }
    
    Serial.println("╠═══════════════════════════════════════════════════════════════╣");
    
    // SENSORES AMBIENTALES
    if (envSensor->isReady()) {
        EnvironmentReading env = envSensor->getLastReading();
        Serial.println("║  🌡️ SENSORES AMBIENTALES:                                     ║");
        Serial.printf("║    Estado:      %-45s  ║\n", envSensor->getStateString().c_str());
        Serial.printf("║    Temperatura: %.2f°C (Δ: %+.2f°C, tasa: %.2f°C/min)   ║\n", 
                     env.temperature, env.tempDelta, env.tempRate);
        Serial.printf("║    Humedad:     %.2f%% (Δ: %+.2f%%)                       ║\n",
                     env.humidity, env.humidityDelta);
        Serial.printf("║    Presión:     %.2f hPa (Δ: %+.2f hPa)                   ║\n",
                     env.pressure, env.pressureDelta);
        
        // Probabilidad de incendio
        float fireProbability = envSensor->calculateFireProbability();
        Serial.printf("║    Prob. Incendio: %.1f%%                                     ║\n",
                     fireProbability * 100);
    } else {
        Serial.println("║  🌡️ SENSORES AMBIENTALES: Error o no disponibles            ║");
    }
    
    Serial.println("╠═══════════════════════════════════════════════════════════════╣");
    
    // ALERTA GLOBAL
    const char* alertText;
    const char* alertIcon;
    const char* action;
    
    switch (currentAlert) {
        case ALERT_NORMAL:
            alertText = "NORMAL - Todo en orden";
            alertIcon = "🟢";
            action = "Monitoreo rutinario";
            break;
        case ALERT_COOKING:
            alertText = "COCINA/VAPOR - Falsa alarma evitada";
            alertIcon = "🍳";
            action = "Solo vapor, no hay peligro";
            break;
        case ALERT_ANOMALY:
            alertText = "ANOMALÍA - Un sensor activado";
            alertIcon = "🤔";
            action = "Monitorear situación";
            break;
        case ALERT_CAUTION:
            alertText = "PRECAUCIÓN - Dos sensores activados";
            alertIcon = "🟡";
            action = "Investigar fuente";
            break;
        case ALERT_WARNING:
            alertText = "ADVERTENCIA - Múltiples sensores";
            alertIcon = "🟠";
            action = "Ventilar y preparar evacuación";
            break;
        case ALERT_FIRE_SUSPECTED:
            alertText = "INCENDIO SOSPECHOSO";
            alertIcon = "⚠️";
            action = "Verificar y preparar evacuación";
            break;
        case ALERT_FIRE_CONFIRMED:
            alertText = "INCENDIO CONFIRMADO";
            alertIcon = "🔥";
            action = "EVACUAR INMEDIATAMENTE";
            break;
        case ALERT_GAS_CRITICAL:
            alertText = "GAS CRÍTICO + FUEGO";
            alertIcon = "🚨";
            action = "EVACUAR - RIESGO EXPLOSIÓN";
            break;
        case ALERT_EXPLOSIVE:
            alertText = "NIVEL EXPLOSIVO";
            alertIcon = "💥";
            action = "¡¡¡EVACUAR YA!!!";
            break;
    }
    
    Serial.printf("║  %s ALERTA GLOBAL: %-42s║\n", alertIcon, alertText);
    Serial.printf("║    Acción:      %-45s  ║\n", action);
    
    Serial.println("╚═══════════════════════════════════════════════════════════════╝\n");
}

void setup() {
    // Serial
    if (DEBUG_SERIAL) {
        Serial.begin(SERIAL_BAUD_RATE);
        delay(1000);
        Serial.println("\n\n╔════════════════════════════════════════════════╗");
        Serial.println("║  SISTEMA INTELIGENTE DE ALARMA v2.0          ║");
        Serial.println("║  Incendio + Gas + Ambiente                   ║");
        Serial.println("╚════════════════════════════════════════════════╝\n");
    }
    
    // 1. File System
    fileManager = FileManager::getInstance();
    if (!fileManager->begin()) {
        Serial.println("❌ Error: LittleFS");
        return;
    }
    
    // 2. LED
    ledController = LEDController::getInstance(LED_PIN);
    ledController->begin();
    
    // 3. Sensores
    Serial.println("🔧 Inicializando sensores...\n");
    
    smokeSensor = SmokeSensor::getInstance(SMOKE_SENSOR_PIN);
    smokeSensor->begin(true);  // 60 seg
    
    ch4Sensor = CH4Sensor::getInstance(CH4_SENSOR_PIN);
    ch4Sensor->begin(true);    // 180 seg
    
    envSensor = EnvironmentSensor::getInstance();
    envSensor->begin(I2C_SDA, I2C_SCL);
    
    Serial.println("\n⏳ Tiempos de calentamiento:");
    Serial.println("   • Smoke:     60 segundos");
    Serial.println("   • CH4:       180 segundos (3 min)");
    Serial.println("   • Ambiente:  Listo inmediatamente");
    Serial.println("   Las lecturas se habilitarán automáticamente\n");
    
    // 4. WiFi
    wifiManager = WiFiManager::getInstance();
    bool wifiConnected = wifiManager->begin();
    
    // 5. Web Server
    webServer = MyWebServer::getInstance();
    
    if (wifiConnected) {
        Serial.println("✓ WiFi conectado: " + wifiManager->getLocalIP());
        webServer->begin(false);
        
        if (OTA_ENABLED) {
            otaManager = OTAManager::getInstance();
            otaManager->begin(OTA_HOSTNAME, OTA_PASSWORD);
        }
    } else {
        Serial.println("⚠ Modo AP - Configura WiFi");
        wifiManager->startAccessPoint();
        webServer->begin(true);
    }
    
    Serial.println("\n✓ Sistema iniciado\n");
}

void loop() {
    // Actualizar módulos base
    ledController->update();
    wifiManager->checkConnection();
    
    if (OTA_ENABLED && wifiManager->isWiFiConnected()) {
        otaManager->handle();
    }
    
    // ========== LECTURA Y ANÁLISIS DE SENSORES ==========
    if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL) {
        lastSensorRead = millis();
        
        // Leer todos los sensores
        SmokeReading smoke = smokeSensor->read();
        CH4Reading ch4 = ch4Sensor->read();
        EnvironmentReading env = envSensor->read();
        
        // Evaluar alerta con inteligencia multi-sensor
        GlobalAlertLevel newAlert = evaluateSmartAlert();
        
        // Detectar cambio de nivel
        if (newAlert != currentAlert) {
            currentAlert = newAlert;
            Serial.println("\n⚠️ ══════ CAMBIO DE NIVEL DE ALERTA ══════ ⚠️");
        }
        
        // Mostrar estado completo
        if (DEBUG_SERIAL) {
            displayFullStatus();
        }
        
        // ========== ACCIONES SEGÚN NIVEL ==========
        
        if (currentAlert >= ALERT_FIRE_CONFIRMED) {
            Serial.println("🚨🚨🚨 ¡¡¡EMERGENCIA!!! 🚨🚨🚨");
            Serial.println("EVACUAR INMEDIATAMENTE\n");
            
            // TODO: Activar sirena máxima
            // TODO: Enviar notificación de emergencia
            // TODO: Llamada automática a emergencias
        }
        else if (currentAlert == ALERT_FIRE_SUSPECTED) {
            Serial.println("⚠️ INCENDIO SOSPECHOSO");
            Serial.println("Verificar situación y preparar evacuación\n");
            
            // TODO: Activar alarma
            // TODO: Notificación urgente
        }
        else if (currentAlert == ALERT_WARNING) {
            Serial.println("🟠 ADVERTENCIA - Múltiples sensores activados");
            Serial.println("Ventilar área inmediatamente\n");
            
            // TODO: Alarma moderada
            // TODO: Notificación de advertencia
        }
        else if (currentAlert == ALERT_COOKING) {
            Serial.println("🍳 Detección de vapor/cocina - No es peligroso\n");
            
            // Solo notificación leve
        }
    }
    
    delay(10);
}

// ============================================================
// COMANDOS DE CONTROL (OPCIONAL)
// ============================================================
//
// void processCommands() {
//     if (Serial.available()) {
//         String cmd = Serial.readStringUntil('\n');
//         cmd.trim();
//         
//         if (cmd == "status") {
//             displayFullStatus();
//         }
//         else if (cmd == "calibrate") {
//             Serial.println("Calibrando todos los sensores...");
//             smokeSensor->calibrate(300, 1000);
//             ch4Sensor->calibrate(300, 1000);
//             envSensor->calibrateBaseline();
//         }
//         else if (cmd == "baseline") {
//             Serial.println("Calibrando baseline ambiental...");
//             envSensor->calibrateBaseline();
//         }
//         else if (cmd == "test") {
//             Serial.println("Modo TEST - 30 segundos");
//             for (int i = 0; i < 30; i++) {
//                 smokeSensor->read();
//                 ch4Sensor->read();
//                 envSensor->read();
//                 evaluateSmartAlert();
//                 displayFullStatus();
//                 delay(1000);
//             }
//         }
//     }
// }