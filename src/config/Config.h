#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==================== CONFIGURACIÓN DE PINES ====================
#define LED_PIN 2                    // GPIO del LED de estado

// ==================== PINES DE SENSORES ====================
// Sensores I2C
#define I2C_SDA 21                   // Pin SDA para I2C
#define I2C_SCL 22                   // Pin SCL para I2C

// Sensores Analógicos (ADC1 - Compatible con WiFi)
#define SMOKE_SENSOR_PIN 36          // VP - Sensor de humo MEMS
#define CH4_SENSOR_PIN 39            // VN - Sensor de metano MEMS

// Actuadores
#define RGB_LED_PIN 23               // LED RGB WS2812B (NeoPixel)
#define BUZZER_PIN 32                // Buzzer activo o pasivo
#define BUTTON_PIN 34                // Botón de reset de alarma

// ==================== CONFIGURACIÓN DE SENSORES ====================
#define SENSOR_READ_INTERVAL 5000    // Intervalo de lectura de sensores (ms)
#define CH4_LEL_THRESHOLD 5.0        // % LEL para alarma crítica (5% = explosivo)

// ==================== CONFIGURACIÓN DE RED ====================
#define AP_SSID "ESP-WIFI-MANAGER"   // Nombre del Access Point
#define AP_PASSWORD "12345678"       // Contraseña del AP (mínimo 8 caracteres)
#define WEB_SERVER_PORT 80           // Puerto del servidor web

// ==================== CONFIGURACIÓN OTA ====================
// IMPORTANTE: Estas son configuraciones por defecto para desarrollo
// Para producción, crea Config_local.h con tus valores personalizados
#ifndef OTA_HOSTNAME
  #define OTA_HOSTNAME "ESP32-OTA"     // Nombre del dispositivo para OTA
#endif

#ifndef OTA_PASSWORD
  #define OTA_PASSWORD "admin123"      // Contraseña para OTA (CAMBIAR EN PRODUCCIÓN)
#endif

#define OTA_PORT 3232                // Puerto OTA (por defecto 3232)
#define OTA_ENABLED true             // Habilitar OTA por defecto

// ==================== TIMEOUTS E INTERVALOS ====================
#define WIFI_CONNECT_TIMEOUT 10000   // Timeout para conectar WiFi (ms)
#define WIFI_CHECK_INTERVAL 30000    // Intervalo para verificar conexión WiFi (ms)
#define LED_BLINK_INTERVAL 500       // Intervalo de parpadeo del LED (ms)

// ==================== RUTAS DE ARCHIVOS EN LITTLEFS ====================
#define SSID_FILE_PATH "/ssid.txt"
#define PASS_FILE_PATH "/pass.txt"
#define IP_FILE_PATH "/ip.txt"
#define GATEWAY_FILE_PATH "/gateway.txt"
#define SUBNET_FILE_PATH "/subnet.txt"
#define DHCP_FILE_PATH "/dhcp.txt"

// ==================== NOMBRES DE PARÁMETROS HTTP ====================
#define PARAM_SSID "ssid"
#define PARAM_PASS "pass"
#define PARAM_IP "ip"
#define PARAM_GATEWAY "gateway"
#define PARAM_SUBNET "subnet"
#define PARAM_DHCP "dhcp"

// ==================== VALORES POR DEFECTO ====================
#define DEFAULT_SUBNET "255.255.255.0"

// ==================== CONFIGURACIÓN DE DEBUG ====================
#define DEBUG_SERIAL true            // Habilitar mensajes por Serial
#define SERIAL_BAUD_RATE 115200      // Velocidad del puerto serial

// ==================== MENSAJES DEL SISTEMA ====================
namespace Messages {
    const char* const WIFI_CONNECTING = "Conectando a WiFi...";
    const char* const WIFI_CONNECTED = "Conectado! IP: ";
    const char* const WIFI_FAILED = "Fallo al conectar";
    const char* const WIFI_LOST = "Conexión WiFi perdida";
    const char* const AP_MODE = "Modo Access Point iniciado";
    const char* const LITTLEFS_ERROR = "Error montando LittleFS";
    const char* const LITTLEFS_OK = "LittleFS montado exitosamente";
    const char* const SERVER_STARTED = "Servidor HTTP iniciado";
    const char* const INVALID_IP = "Formato de IP inválido";
    const char* const CONFIG_SAVED = "Configuración guardada";
}

// ==================== INCLUIR CONFIGURACIÓN LOCAL ====================
// Si existe Config_local.h, se incluirá y sobrescribirá valores de arriba
#if __has_include("Config_local.h")
  #include "Config_local.h"
  #pragma message("✓ Usando Config_local.h - Configuración personalizada cargada")
#else
  #pragma message("⚠ Config_local.h no encontrado - Usando valores por defecto")
  #pragma message("  Para producción: Copia Config_local.h.example a Config_local.h")
#endif

#endif // CONFIG_H