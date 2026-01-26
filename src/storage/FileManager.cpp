#include "FileManager.h"
#include "../config/Config.h"

// Inicializar instancia estática
FileManager* FileManager::instance = nullptr;

FileManager::FileManager() {
    // Constructor privado
}

FileManager* FileManager::getInstance() {
    if (instance == nullptr) {
        instance = new FileManager();
    }
    return instance;
}

bool FileManager::begin() {
    if (!LittleFS.begin(true)) {
        if (DEBUG_SERIAL) {
            Serial.println(Messages::LITTLEFS_ERROR);
        }
        return false;
    }
    
    if (DEBUG_SERIAL) {
        Serial.println(Messages::LITTLEFS_OK);
    }
    return true;
}

String FileManager::readFile(const char* path) {
    File file = LittleFS.open(path, "r");
    if (!file) {
        if (DEBUG_SERIAL) {
            Serial.printf("Error al abrir archivo: %s\n", path);
        }
        return String();
    }
    
    String content = file.readString();
    file.close();
    content.trim(); // Eliminar espacios en blanco al inicio y final
    
    if (DEBUG_SERIAL) {
        Serial.printf("Archivo leído: %s -> %s\n", path, content.c_str());
    }
    
    return content;
}

bool FileManager::writeFile(const char* path, const String& content) {
    File file = LittleFS.open(path, "w");
    if (!file) {
        if (DEBUG_SERIAL) {
            Serial.printf("Error al escribir archivo: %s\n", path);
        }
        return false;
    }
    
    size_t bytesWritten = file.print(content);
    file.close();
    
    if (DEBUG_SERIAL) {
        Serial.printf("Archivo escrito: %s -> %s (%d bytes)\n", 
                     path, content.c_str(), bytesWritten);
    }
    
    return bytesWritten > 0;
}

bool FileManager::appendFile(const char* path, const String& content) {
    File file = LittleFS.open(path, "a");
    if (!file) {
        if (DEBUG_SERIAL) {
            Serial.printf("Error al añadir a archivo: %s\n", path);
        }
        return false;
    }
    
    size_t bytesWritten = file.print(content);
    file.close();
    
    if (DEBUG_SERIAL) {
        Serial.printf("Contenido añadido a: %s (%d bytes)\n", path, bytesWritten);
    }
    
    return bytesWritten > 0;
}

bool FileManager::exists(const char* path) {
    return LittleFS.exists(path);
}

bool FileManager::deleteFile(const char* path) {
    bool result = LittleFS.remove(path);
    
    if (DEBUG_SERIAL) {
        if (result) {
            Serial.printf("Archivo eliminado: %s\n", path);
        } else {
            Serial.printf("Error al eliminar archivo: %s\n", path);
        }
    }
    
    return result;
}

bool FileManager::clearWiFiConfig() {
    bool success = true;
    
    success &= deleteFile(SSID_FILE_PATH);
    success &= deleteFile(PASS_FILE_PATH);
    success &= deleteFile(IP_FILE_PATH);
    success &= deleteFile(GATEWAY_FILE_PATH);
    success &= deleteFile(SUBNET_FILE_PATH);
    success &= deleteFile(DHCP_FILE_PATH);
    
    if (DEBUG_SERIAL) {
        if (success) {
            Serial.println("Configuración WiFi eliminada completamente");
        } else {
            Serial.println("Error al eliminar algunos archivos de configuración");
        }
    }
    
    return success;
}

size_t FileManager::getFileSize(const char* path) {
    File file = LittleFS.open(path, "r");
    if (!file) {
        return 0;
    }
    
    size_t size = file.size();
    file.close();
    
    return size;
}

void FileManager::listFiles() {
    if (!DEBUG_SERIAL) return;
    
    Serial.println("\n=== Archivos en LittleFS ===");
    
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    
    while (file) {
        Serial.printf("  %s (%d bytes)\n", file.name(), file.size());
        file = root.openNextFile();
    }
    
    Serial.println("============================\n");
}