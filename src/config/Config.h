/*
Centraliza todas las constantes y configuraciones:

Pines GPIO
Nombres de archivos
Parámetros de red
Timeouts e intervalos
Credenciales de AP
*/
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==================== CONFIGURACIÓN DE PINES ====================
#define LED_PIN 2                    // GPIO del LED de estado

// ==================== CONFIGURACIÓN DE RED ====================
#define AP_SSID "ESP-WIFI-MANAGER"   // Nombre del Access Point
#define AP_PASSWORD "12345678"       // Contraseña del AP (mínimo 8 caracteres)
#define WEB_SERVER_PORT 80           // Puerto del servidor web

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

#endif // CONFIG_H