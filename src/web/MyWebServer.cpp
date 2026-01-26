#include "MyWebServer.h"
#include "../config/Config.h"
#include "../utils/Validators.h"
#include <LittleFS.h>
#include <WiFi.h>

// Inicializar instancia estática
MyWebServer* MyWebServer::instance = nullptr;

MyWebServer::MyWebServer() {
    server = new AsyncWebServer(WEB_SERVER_PORT);
    wifiManager = WiFiManager::getInstance();
    ledController = LEDController::getInstance();
    fileManager = FileManager::getInstance();
}

MyWebServer* MyWebServer::getInstance() {
    if (instance == nullptr) {
        instance = new MyWebServer();
    }
    return instance;
}

String MyWebServer::processor(const String& var) {
    MyWebServer* ws = getInstance();
    
    if (var == "STATE") {
        return ws->ledController->getStateString();
    }
    if (var == "SSID") {
        return ws->wifiManager->getConfig().ssid;
    }
    if (var == "IP") {
        return ws->wifiManager->getLocalIP();
    }
    if (var == "GATEWAY") {
        return ws->wifiManager->getGateway();
    }
    if (var == "SUBNET") {
        return ws->wifiManager->getSubnet();
    }
    
    return String();
}

void MyWebServer::setupStationRoutes() {
    // Servir archivos estáticos (esto ya maneja "/" automáticamente)
    server->serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    
    // Encender LED
    server->on("/on", HTTP_GET, [](AsyncWebServerRequest *request) {
        MyWebServer* ws = getInstance();
        ws->ledController->turnOn();
        request->send(LittleFS, "/index.html", "text/html", false, processor);
    });
    
    // Apagar LED
    server->on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
        MyWebServer* ws = getInstance();
        ws->ledController->turnOff();
        request->send(LittleFS, "/index.html", "text/html", false, processor);
    });
    
    // Reset configuración WiFi
    server->on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = 
            "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
            "<style>body{font-family:Arial;text-align:center;margin-top:50px;}</style></head>"
            "<body><h1>Restableciendo Configuración WiFi...</h1>"
            "<p>El ESP32 se reiniciará en modo Access Point.</p>"
            "<p>Conéctate a 'ESP-WIFI-MANAGER' para reconfigurar.</p></body></html>";
        
        request->send(200, "text/html", html);
        
        delay(2000);
        MyWebServer* ws = getInstance();
        ws->wifiManager->resetConfig();
    });
    
    if (DEBUG_SERIAL) {
        Serial.println("Rutas del modo Station configuradas");
    }
}

void MyWebServer::setupAPRoutes() {
    // Servir página de configuración en la raíz
    server->serveStatic("/", LittleFS, "/").setDefaultFile("wifimanager.html");
    
    // Manejar POST del formulario de configuración en /save
    server->on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
        getInstance()->handleConfigPost(request);
    });
    
    if (DEBUG_SERIAL) {
        Serial.println("Rutas del modo AP configuradas");
    }
}

void MyWebServer::handleConfigPost(AsyncWebServerRequest *request) {
    WiFiConfig newConfig;
    bool configValid = true;
    String errorMsg = "";
    
    // Procesar parámetros del formulario
    int params = request->params();
    for (int i = 0; i < params; i++) {
        const AsyncWebParameter* p = request->getParam(i);
        if (p->isPost()) {
            String paramName = p->name();
            String paramValue = p->value();
            
            if (paramName == PARAM_SSID) {
                newConfig.ssid = paramValue;
                if (!Validators::isValidSSID(newConfig.ssid)) {
                    configValid = false;
                    errorMsg = "SSID inválido";
                }
            }
            else if (paramName == PARAM_PASS) {
                newConfig.password = paramValue;
                if (!Validators::isValidPassword(newConfig.password)) {
                    configValid = false;
                    errorMsg = "Contraseña inválida (debe tener al menos 8 caracteres)";
                }
            }
            else if (paramName == PARAM_DHCP) {
                newConfig.useDHCP = (paramValue == "true");
            }
            else if (paramName == PARAM_IP) {
                newConfig.ip = paramValue;
            }
            else if (paramName == PARAM_GATEWAY) {
                newConfig.gateway = paramValue;
            }
            else if (paramName == PARAM_SUBNET) {
                newConfig.subnet = paramValue;
                if (newConfig.subnet == "") {
                    newConfig.subnet = DEFAULT_SUBNET;
                }
            }
        }
    }
    
    // Validar IPs si no se usa DHCP
    if (!newConfig.useDHCP && configValid) {
        if (!Validators::isValidIP(newConfig.ip)) {
            configValid = false;
            errorMsg = "Formato de IP inválido";
        }
        else if (!Validators::isValidIP(newConfig.gateway)) {
            configValid = false;
            errorMsg = "Formato de Gateway inválido";
        }
        else if (!Validators::isValidIP(newConfig.subnet)) {
            configValid = false;
            errorMsg = "Formato de Subnet inválido";
        }
    }
    
    if (configValid) {
        // Guardar configuración
        if (wifiManager->saveConfig(newConfig)) {
            String responseHTML = 
                "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
                "<style>body{font-family:Arial;text-align:center;margin-top:50px;background:#f0f0f0;}"
                "h1{color:#4CAF50;}p{font-size:18px;}</style></head>"
                "<body><h1>✓ Configuración Guardada!</h1>"
                "<p>ESP32 se reiniciará y se conectará a tu red WiFi.</p>";
            
            if (newConfig.useDHCP) {
                responseHTML += "<p>Usando DHCP - La IP se asignará automáticamente</p>";
            } else {
                responseHTML += "<p>IP Estática: " + newConfig.ip + "</p>";
            }
            
            responseHTML += 
                "<p>Por favor espera 10 segundos y conecta tu dispositivo a la red WiFi.</p>"
                "<p style='color:#888;'>Si la conexión falla, el dispositivo volverá al modo AP.</p>"
                "</body></html>";
            
            request->send(200, "text/html", responseHTML);
            
            delay(3000);
            wifiManager->restart();
        } else {
            errorMsg = "Error al guardar la configuración";
            configValid = false;
        }
    }
    
    if (!configValid) {
        String errorHTML = 
            "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
            "<style>body{font-family:Arial;text-align:center;margin-top:50px;}"
            "h1{color:#f44336;}</style></head>"
            "<body><h1>✗ Error de Configuración</h1>"
            "<p>" + errorMsg + "</p>"
            "<p><a href='/'>Volver</a></p></body></html>";
        
        request->send(400, "text/html", errorHTML);
    }
}

void MyWebServer::begin(bool isAPMode) {
    if (isAPMode) {
        setupAPRoutes();
    } else {
        setupStationRoutes();
    }
    
    server->begin();
    
    if (DEBUG_SERIAL) {
        Serial.println(Messages::SERVER_STARTED);
    }
}

void MyWebServer::stop() {
    server->end();
    
    if (DEBUG_SERIAL) {
        Serial.println("Servidor web detenido");
    }
}