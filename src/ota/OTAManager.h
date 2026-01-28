#ifndef OTAMANAGER_H
#define OTAMANAGER_H

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include "../led/LEDController.h"

// Estados de la actualización OTA
enum class OTAState {
    IDLE,           // Esperando
    STARTING,       // Iniciando actualización
    PROGRESS,       // Actualizando
    COMPLETED,      // Completado
    ERROR           // Error
};

class OTAManager {
private:
    static OTAManager* instance;
    LEDController* ledController;
    String hostname;
    String password;
    bool enabled;
    OTAState currentState;
    unsigned int lastProgress;
    
    OTAManager(); // Constructor privado
    
    /**
     * Configura los callbacks de ArduinoOTA
     */
    void setupCallbacks();
    
public:
    /**
     * Obtiene la instancia única de OTAManager (Singleton)
     * @return Puntero a la instancia de OTAManager
     */
    static OTAManager* getInstance();
    
    /**
     * Inicializa el servicio OTA
     * @param deviceHostname Nombre del dispositivo en la red
     * @param otaPassword Contraseña para OTA (opcional)
     * @return true si se inicializó correctamente
     */
    bool begin(const String& deviceHostname = "ESP32", const String& otaPassword = "");
    
    /**
     * Maneja las actualizaciones OTA (llamar en loop)
     */
    void handle();
    
    /**
     * Habilita o deshabilita OTA
     * @param enable true para habilitar, false para deshabilitar
     */
    void setEnabled(bool enable);
    
    /**
     * Verifica si OTA está habilitado
     * @return true si está habilitado
     */
    bool isEnabled() const;
    
    /**
     * Obtiene el estado actual de OTA
     * @return Estado actual
     */
    OTAState getState() const;
    
    /**
     * Obtiene el progreso de la actualización (0-100)
     * @return Porcentaje de progreso
     */
    unsigned int getProgress() const;
    
    /**
     * Obtiene el hostname configurado
     * @return Hostname del dispositivo
     */
    String getHostname() const;
    
    /**
     * Establece un nuevo hostname
     * @param newHostname Nuevo nombre
     */
    void setHostname(const String& newHostname);
    
    /**
     * Establece una nueva contraseña OTA
     * @param newPassword Nueva contraseña
     */
    void setPassword(const String& newPassword);
};

#endif // OTAMANAGER_H