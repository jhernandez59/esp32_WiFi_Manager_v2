#ifndef MY_WEBSERVER_H
#define MY_WEBSERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include "../wifi/WiFiManager.h"
#include "../led/LEDController.h"
#include "../storage/FileManager.h"

class MyWebServer {
private:
    static MyWebServer* instance;
    AsyncWebServer* server;
    WiFiManager* wifiManager;
    LEDController* ledController;
    FileManager* fileManager;
    
    MyWebServer(); // Constructor privado
    
    /**
     * Procesa placeholders en páginas HTML
     */
    static String processor(const String& var);
    
    /**
     * Configura las rutas para el modo Station (conectado)
     */
    void setupStationRoutes();
    
    /**
     * Configura las rutas para el modo Access Point (configuración)
     */
    void setupAPRoutes();
    
    /**
     * Maneja el POST del formulario de configuración WiFi
     */
    void handleConfigPost(AsyncWebServerRequest *request);
    
public:
    /**
     * Obtiene la instancia única de MyWebServer (Singleton)
     * @return Puntero a la instancia de MyWebServer
     */
    static MyWebServer* getInstance();
    
    /**
     * Inicializa el servidor web
     * @param isAPMode true si está en modo Access Point
     */
    void begin(bool isAPMode);
    
    /**
     * Detiene el servidor web
     */
    void stop();
};

#endif // MY_WEBSERVER_H