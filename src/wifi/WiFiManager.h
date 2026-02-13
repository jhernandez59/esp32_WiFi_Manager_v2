/*
Gestión completa de WiFi:

Conexión a red
Modo Access Point
Reconexión automática
Validación de credenciales
*/
#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <IPAddress.h>
#include "../storage/FileManager.h"
#include "../led/LEDController.h"

struct WiFiConfig {
    String ssid;
    String password;
    String ip;
    String gateway;
    String subnet;
    bool useDHCP;   // true para DHCP, false para IP estática
};

class WiFiManager {
private:
    static WiFiManager* instance;
    FileManager* fileManager;
    LEDController* ledController;
    WiFiConfig config;
    unsigned long lastCheckTime;
    bool isConnected;
    
    WiFiManager(); // Constructor privado
    
    /**
     * Carga la configuración desde los archivos
     */
    void loadConfig();
    
    /**
     * Intenta conectar a WiFi con la configuración actual
     * @return true si se conectó exitosamente
     */
    bool connectToWiFi();
    
public:
    /**
     * Obtiene la instancia única de WiFiManager (Singleton)
     * @return Puntero a la instancia de WiFiManager
     */
    static WiFiManager* getInstance();
    
    /**
     * Inicializa el WiFiManager
     * @return true si se inicializó y conectó correctamente
     */
    bool begin();
    
    /**
     * Inicia el modo Access Point
     */
    void startAccessPoint();
    
    /**
     * Verifica periódicamente la conexión WiFi (llamar en loop)
     */
    void checkConnection();
    
    /**
     * Guarda nueva configuración WiFi
     * @param newConfig Nueva configuración
     * @return true si se guardó correctamente
     */
    bool saveConfig(const WiFiConfig& newConfig);
    
    /**
     * Obtiene la configuración actual
     * @return Configuración WiFi actual
     */
    WiFiConfig getConfig() const;
    
    /**
     * Verifica si está conectado a WiFi
     * @return true si está conectado
     */
    bool isWiFiConnected() const;
    
    /**
     * Obtiene la dirección IP local
     * @return String con la IP
     */
    String getLocalIP() const;
    
    /**
     * Obtiene el Gateway
     * @return String con el Gateway
     */
    String getGateway() const;
    
    /**
     * Obtiene la máscara de subred
     * @return String con la subnet
     */
    String getSubnet() const;
    
    /**
     * Reinicia el ESP32
     */
    void restart();
    
    /**
     * Resetea la configuración WiFi
     */
    void resetConfig();
};

#endif // WIFIMANAGER_H