/*
Funciones de utilidad:

Validación de IPs
Validación de formularios
Funciones helper
*/
#ifndef VALIDATORS_H
#define VALIDATORS_H

#include <Arduino.h>
#include <IPAddress.h>

class Validators {
public:
    /**
     * Valida el formato de una dirección IP
     * @param ipStr String con la IP a validar
     * @return true si la IP es válida, false en caso contrario
     */
    static bool isValidIP(const String& ipStr);
    
    /**
     * Valida si un SSID es válido
     * @param ssid String con el SSID a validar
     * @return true si el SSID es válido, false en caso contrario
     */
    static bool isValidSSID(const String& ssid);
    
    /**
     * Valida una contraseña WiFi
     * @param password String con la contraseña a validar
     * @return true si la contraseña es válida, false en caso contrario
     */
    static bool isValidPassword(const String& password);
    
    /**
     * Convierte un String a IPAddress
     * @param ipStr String con la IP
     * @param ip Referencia al objeto IPAddress donde guardar el resultado
     * @return true si la conversión fue exitosa
     */
    static bool stringToIP(const String& ipStr, IPAddress& ip);
};

#endif // VALIDATORS_H