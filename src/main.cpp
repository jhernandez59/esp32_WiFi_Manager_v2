#include <Arduino.h>
#include "config/Config.h"
#include "storage/FileManager.h"
#include "wifi/WiFiManager.h"
#include "led/LEDController.h"
#include "web/MyWebServer.h"

// Instancias de los módulos
FileManager* fileManager;
WiFiManager* wifiManager;
LEDController* ledController;
MyWebServer* webServer;

void setup() {
    // Inicializar puerto serial
    if (DEBUG_SERIAL) {
        Serial.begin(SERIAL_BAUD_RATE);
        delay(1000);
        Serial.println("\n\n╔════════════════════════════════════╗");
        Serial.println("║   ESP32 WiFi Manager - Modular   ║");
        Serial.println("╚════════════════════════════════════╝\n");
    }
    
    // 1. Inicializar sistema de archivos
    fileManager = FileManager::getInstance();
    if (!fileManager->begin()) {
        if (DEBUG_SERIAL) {
            Serial.println("❌ Error crítico: LittleFS no pudo iniciarse");
        }
        return;
    }
    
    // Opcional: Listar archivos en LittleFS
    if (DEBUG_SERIAL) {
        fileManager->listFiles();
    }
    
    // 2. Inicializar control del LED
    ledController = LEDController::getInstance(LED_PIN);
    ledController->begin();
    
    // 3. Inicializar WiFi Manager
    wifiManager = WiFiManager::getInstance();
    bool wifiConnected = wifiManager->begin();
    
    // 4. Inicializar servidor web
    webServer = MyWebServer::getInstance();
    
    if (wifiConnected) {
        // ========== MODO STATION - Conectado a WiFi ==========
        if (DEBUG_SERIAL) {
            Serial.println("\n✓ Modo Station - Conectado a WiFi");
            Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
            Serial.println("SSID: " + wifiManager->getConfig().ssid);
            Serial.println("IP: " + wifiManager->getLocalIP());
            Serial.println("Gateway: " + wifiManager->getGateway());
            Serial.println("Subnet: " + wifiManager->getSubnet());
            Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
        }
        
        webServer->begin(false); // Modo Station
        ledController->setState(LEDState::ON);
        
    } else {
        // ========== MODO ACCESS POINT - Portal de Configuración ==========
        if (DEBUG_SERIAL) {
            Serial.println("\n⚠ Modo Access Point - Portal de Configuración");
            Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
            Serial.println("SSID AP: " + String(AP_SSID));
            Serial.println("Password: " + String(AP_PASSWORD));
            Serial.println("IP: 192.168.4.1");
            Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
            Serial.println("Conéctate al AP y visita: http://192.168.4.1");
            Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
        }
        
        wifiManager->startAccessPoint();
        webServer->begin(true); // Modo AP
        ledController->setState(LEDState::AP_MODE);
    }
    
    if (DEBUG_SERIAL) {
        Serial.println("✓ Sistema iniciado correctamente\n");
    }
}

void loop() {
    // Actualizar estado del LED
    ledController->update();
    
    // Verificar conexión WiFi periódicamente
    wifiManager->checkConnection();
    
    // Agregar pequeño delay para no saturar el CPU
    delay(10);
}

/*
 * NOTAS DE DESARROLLO:
 * 
 * - Este archivo main.cpp ahora es mucho más simple y legible
 * - Toda la lógica compleja está en módulos especializados
 * - Fácil de extender: solo crea nuevos módulos siguiendo el mismo patrón
 * 
 * PARA AGREGAR NUEVAS FUNCIONALIDADES:
 * 1. Crea un nuevo módulo en su propia carpeta (ej: sensors/)
 * 2. Sigue el patrón Singleton si solo necesitas una instancia
 * 3. Incluye el .h en main.cpp
 * 4. Inicializa en setup() y actualiza en loop() si es necesario
 * 
 * EJEMPLO - Agregar sensor DHT22:
 * 
 * #include "sensors/DHTSensor.h"
 * 
 * DHTSensor* dhtSensor;
 * 
 * void setup() {
 *     // ... código existente ...
 *     dhtSensor = DHTSensor::getInstance(DHT_PIN);
 *     dhtSensor->begin();
 * }
 * 
 * void loop() {
 *     // ... código existente ...
 *     dhtSensor->update();
 * }
 */