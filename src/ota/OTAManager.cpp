#include "OTAManager.h"
#include "../config/Config.h"

// Inicializar instancia estática
OTAManager* OTAManager::instance = nullptr;

OTAManager::OTAManager() 
    : enabled(false), 
      currentState(OTAState::IDLE),
      lastProgress(0) {
    ledController = LEDController::getInstance();
}

OTAManager* OTAManager::getInstance() {
    if (instance == nullptr) {
        instance = new OTAManager();
    }
    return instance;
}

void OTAManager::setupCallbacks() {
    // Callback cuando comienza la actualización
    ArduinoOTA.onStart([this]() {
        currentState = OTAState::STARTING;
        String type;
        
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS o U_FS
            type = "filesystem";
        }
        
        if (DEBUG_SERIAL) {
            Serial.println("\n╔═══════════════════════════════════╗");
            Serial.println("║    INICIANDO ACTUALIZACIÓN OTA    ║");
            Serial.println("╚═══════════════════════════════════╝");
            Serial.println("Tipo: " + type);
        }
        
        // Apagar LED durante actualización para indicar que está ocupado
        ledController->turnOff();
    });
    
    // Callback de progreso
    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
        currentState = OTAState::PROGRESS;
        lastProgress = (progress / (total / 100));
        
        if (DEBUG_SERIAL) {
            // Mostrar progreso cada 10%
            if (lastProgress % 10 == 0 && lastProgress != 0) {
                Serial.printf("Progreso: %u%%\n", lastProgress);
            }
        }
        
        // Parpadear LED durante actualización
        if (millis() % 200 < 100) {
            ledController->turnOn();
        } else {
            ledController->turnOff();
        }
    });
    
    // Callback cuando termina la actualización
    ArduinoOTA.onEnd([this]() {
        currentState = OTAState::COMPLETED;
        
        if (DEBUG_SERIAL) {
            Serial.println("\n╔═══════════════════════════════════╗");
            Serial.println("║   ACTUALIZACIÓN COMPLETADA ✓      ║");
            Serial.println("╚═══════════════════════════════════╝");
            Serial.println("Reiniciando...\n");
        }
        
        ledController->turnOn(); // LED encendido al finalizar
    });
    
    // Callback de error
    ArduinoOTA.onError([this](ota_error_t error) {
        currentState = OTAState::ERROR;
        
        if (DEBUG_SERIAL) {
            Serial.printf("\n╔═══════════════════════════════════╗\n");
            Serial.printf("║   ERROR EN ACTUALIZACIÓN OTA      ║\n");
            Serial.printf("╚═══════════════════════════════════╝\n");
            Serial.printf("Error[%u]: ", error);
            
            if (error == OTA_AUTH_ERROR) {
                Serial.println("Autenticación fallida");
            } else if (error == OTA_BEGIN_ERROR) {
                Serial.println("Error al comenzar");
            } else if (error == OTA_CONNECT_ERROR) {
                Serial.println("Error de conexión");
            } else if (error == OTA_RECEIVE_ERROR) {
                Serial.println("Error al recibir datos");
            } else if (error == OTA_END_ERROR) {
                Serial.println("Error al finalizar");
            }
            Serial.println();
        }
        
        // Parpadear rápido en caso de error
        for (int i = 0; i < 10; i++) {
            ledController->toggle();
            delay(100);
        }
    });
}

bool OTAManager::begin(const String& deviceHostname, const String& otaPassword) {
    if (WiFi.status() != WL_CONNECTED) {
        if (DEBUG_SERIAL) {
            Serial.println("⚠ OTA: WiFi no conectado, no se puede iniciar OTA");
        }
        return false;
    }
    
    hostname = deviceHostname;
    password = otaPassword;
    
    // Configurar hostname
    ArduinoOTA.setHostname(hostname.c_str());
    
    // Configurar contraseña si se proporcionó
    if (password.length() > 0) {
        ArduinoOTA.setPassword(password.c_str());
    }
    
    // Configurar puerto (por defecto 3232)
    ArduinoOTA.setPort(3232);
    
    // Configurar callbacks
    setupCallbacks();
    
    // Iniciar servicio OTA
    ArduinoOTA.begin();
    
    enabled = true;
    currentState = OTAState::IDLE;
    
    if (DEBUG_SERIAL) {
        Serial.println("\n╔═══════════════════════════════════╗");
        Serial.println("║      OTA INICIALIZADO ✓           ║");
        Serial.println("╚═══════════════════════════════════╝");
        Serial.println("Hostname: " + hostname);
        Serial.println("Puerto: 3232");
        Serial.println("IP: " + WiFi.localIP().toString());
        if (password.length() > 0) {
            Serial.println("Contraseña: Configurada");
        } else {
            Serial.println("Contraseña: No configurada (⚠ No seguro)");
        }
        Serial.println("════════════════════════════════════\n");
    }
    
    return true;
}

void OTAManager::handle() {
    if (enabled && WiFi.status() == WL_CONNECTED) {
        ArduinoOTA.handle();
    }
}

void OTAManager::setEnabled(bool enable) {
    enabled = enable;
    
    if (DEBUG_SERIAL) {
        if (enable) {
            Serial.println("✓ OTA habilitado");
        } else {
            Serial.println("⚠ OTA deshabilitado");
        }
    }
}

bool OTAManager::isEnabled() const {
    return enabled;
}

OTAState OTAManager::getState() const {
    return currentState;
}

unsigned int OTAManager::getProgress() const {
    return lastProgress;
}

String OTAManager::getHostname() const {
    return hostname;
}

void OTAManager::setHostname(const String& newHostname) {
    hostname = newHostname;
    ArduinoOTA.setHostname(hostname.c_str());
    
    if (DEBUG_SERIAL) {
        Serial.println("Hostname OTA actualizado: " + hostname);
    }
}

void OTAManager::setPassword(const String& newPassword) {
    password = newPassword;
    if (password.length() > 0) {
        ArduinoOTA.setPassword(password.c_str());
    }
    
    if (DEBUG_SERIAL) {
        if (password.length() > 0) {
            Serial.println("Contraseña OTA actualizada");
        } else {
            Serial.println("⚠ Contraseña OTA eliminada (no seguro)");
        }
    }
}