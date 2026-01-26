#include "Validators.h"

bool Validators::isValidIP(const String& ipStr) {
    // Permitir vacío o 0.0.0.0 para DHCP
    if (ipStr == "" || ipStr == "0.0.0.0") {
        return true;
    }
    
    IPAddress testIP;
    return testIP.fromString(ipStr);
}

bool Validators::isValidSSID(const String& ssid) {
    // SSID no puede estar vacío y debe tener máximo 32 caracteres
    if (ssid.length() == 0 || ssid.length() > 32) {
        return false;
    }
    return true;
}

bool Validators::isValidPassword(const String& password) {
    // La contraseña puede estar vacía (red abierta)
    // Si no está vacía, debe tener al menos 8 caracteres para WPA2
    if (password.length() == 0) {
        return true; // Red abierta
    }
    
    if (password.length() < 8 || password.length() > 63) {
        return false;
    }
    
    return true;
}

bool Validators::stringToIP(const String& ipStr, IPAddress& ip) {
    return ip.fromString(ipStr);
}