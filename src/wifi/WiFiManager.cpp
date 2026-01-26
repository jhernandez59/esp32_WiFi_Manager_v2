#include "WiFiManager.h"
#include "../config/Config.h"
#include "../utils/Validators.h"

// Inicializar instancia estática
WiFiManager* WiFiManager::instance = nullptr;

WiFiManager::WiFiManager() 
    : lastCheckTime(0), isConnected(false) {
    fileManager = FileManager::getInstance();
    ledController = LEDController::getInstance();
}

WiFiManager* WiFiManager::getInstance() {
    if (instance == nullptr) {
        instance = new WiFiManager();
    }
    return instance;
}

void WiFiManager::loadConfig() {
    config.ssid = fileManager->readFile(SSID_FILE_PATH);
    config.password = fileManager->readFile(PASS_FILE_PATH);
    config.ip = fileManager->readFile(IP_FILE_PATH);
    config.gateway = fileManager->readFile(GATEWAY_FILE_PATH);
    config.subnet = fileManager->readFile(SUBNET_FILE_PATH);
    
    String dhcpStr = fileManager->readFile(DHCP_FILE_PATH);
    config.useDHCP = (dhcpStr == "true");
    
    // Valor por defecto para subnet
    if (config.subnet == "") {
        config.subnet = DEFAULT_SUBNET;
    }
    
    if (DEBUG_SERIAL) {
        Serial.println("\n=== Configuración WiFi Cargada ===");
        Serial.println("SSID: " + config.ssid);
        Serial.println("IP: " + config.ip);
        Serial.println("Gateway: " + config.gateway);
        Serial.println("Subnet: " + config.subnet);
        Serial.println("DHCP: " + String(config.useDHCP ? "Sí" : "No"));
        Serial.println("==================================\n");
    }
}

bool WiFiManager::connectToWiFi() {
    if (config.ssid == "") {
        if (DEBUG_SERIAL) {
            Serial.println("SSID no configurado");
        }
        return false;
    }
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    ledController->setState(LEDState::CONNECTING);
    
    if (config.useDHCP || config.ip == "" || config.ip == "0.0.0.0") {
        // Usar DHCP
        if (DEBUG_SERIAL) {
            Serial.println("Usando DHCP...");
        }
        WiFi.begin(config.ssid.c_str(), config.password.c_str());
    } else {
        // Usar IP estática
        IPAddress localIP, gateway, subnet;
        
        if (!Validators::stringToIP(config.ip, localIP) ||
            !Validators::stringToIP(config.gateway, gateway) ||
            !Validators::stringToIP(config.subnet, subnet)) {
            if (DEBUG_SERIAL) {
                Serial.println(Messages::INVALID_IP);
            }
            return false;
        }
        
        if (!WiFi.config(localIP, gateway, subnet)) {
            if (DEBUG_SERIAL) {
                Serial.println("Error al configurar IP estática");
            }
            return false;
        }
        
        if (DEBUG_SERIAL) {
            Serial.println("Usando IP estática: " + config.ip);
        }
        
        WiFi.begin(config.ssid.c_str(), config.password.c_str());
    }
    
    if (DEBUG_SERIAL) {
        Serial.println(Messages::WIFI_CONNECTING);
    }
    
    unsigned long startTime = millis();
    
    // Esperar conexión
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime >= WIFI_CONNECT_TIMEOUT) {
            if (DEBUG_SERIAL) {
                Serial.println(Messages::WIFI_FAILED);
            }
            return false;
        }
        ledController->update(); // Parpadear LED mientras conecta
        delay(100);
    }
    
    isConnected = true;
    ledController->setState(LEDState::ON);
    
    if (DEBUG_SERIAL) {
        Serial.print(Messages::WIFI_CONNECTED);
        Serial.println(WiFi.localIP());
    }
    
    return true;
}

bool WiFiManager::begin() {
    loadConfig();
    return connectToWiFi();
}

void WiFiManager::startAccessPoint() {
    if (DEBUG_SERIAL) {
        Serial.println(Messages::AP_MODE);
    }
    
    ledController->setState(LEDState::AP_MODE);
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    
    IPAddress apIP = WiFi.softAPIP();
    
    if (DEBUG_SERIAL) {
        Serial.print("AP IP: ");
        Serial.println(apIP);
        Serial.print("AP Password: ");
        Serial.println(AP_PASSWORD);
    }
}

void WiFiManager::checkConnection() {
    // Solo verificar en modo Station
    if (WiFi.getMode() != WIFI_STA) {
        return;
    }
    
    // Verificar cada WIFI_CHECK_INTERVAL
    if (millis() - lastCheckTime < WIFI_CHECK_INTERVAL) {
        return;
    }
    
    lastCheckTime = millis();
    
    if (WiFi.status() != WL_CONNECTED) {
        if (DEBUG_SERIAL) {
            Serial.println(Messages::WIFI_LOST);
        }
        
        isConnected = false;
        ledController->setState(LEDState::CONNECTING);
        
        // Intentar reconectar
        if (!connectToWiFi()) {
            if (DEBUG_SERIAL) {
                Serial.println("Reconexión fallida. Reiniciando...");
            }
            delay(2000);
            restart();
        }
    }
}

bool WiFiManager::saveConfig(const WiFiConfig& newConfig) {
    // Validar configuración
    if (!Validators::isValidSSID(newConfig.ssid)) {
        if (DEBUG_SERIAL) {
            Serial.println("SSID inválido");
        }
        return false;
    }
    
    if (!Validators::isValidPassword(newConfig.password)) {
        if (DEBUG_SERIAL) {
            Serial.println("Contraseña inválida");
        }
        return false;
    }
    
    if (!newConfig.useDHCP) {
        if (!Validators::isValidIP(newConfig.ip) ||
            !Validators::isValidIP(newConfig.gateway) ||
            !Validators::isValidIP(newConfig.subnet)) {
            if (DEBUG_SERIAL) {
                Serial.println(Messages::INVALID_IP);
            }
            return false;
        }
    }
    
    // Guardar en archivos
    bool success = true;
    success &= fileManager->writeFile(SSID_FILE_PATH, newConfig.ssid);
    success &= fileManager->writeFile(PASS_FILE_PATH, newConfig.password);
    success &= fileManager->writeFile(IP_FILE_PATH, newConfig.ip);
    success &= fileManager->writeFile(GATEWAY_FILE_PATH, newConfig.gateway);
    success &= fileManager->writeFile(SUBNET_FILE_PATH, newConfig.subnet);
    success &= fileManager->writeFile(DHCP_FILE_PATH, newConfig.useDHCP ? "true" : "false");
    
    if (success) {
        config = newConfig;
        if (DEBUG_SERIAL) {
            Serial.println(Messages::CONFIG_SAVED);
        }
    }
    
    return success;
}

WiFiConfig WiFiManager::getConfig() const {
    return config;
}

bool WiFiManager::isWiFiConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getLocalIP() const {
    return WiFi.localIP().toString();
}

String WiFiManager::getGateway() const {
    return WiFi.gatewayIP().toString();
}

String WiFiManager::getSubnet() const {
    return WiFi.subnetMask().toString();
}

void WiFiManager::restart() {
    if (DEBUG_SERIAL) {
        Serial.println("Reiniciando ESP32...");
    }
    delay(1000);
    ESP.restart();
}

void WiFiManager::resetConfig() {
    fileManager->clearWiFiConfig();
    restart();
}