# 📡 ESP32 WiFi Manager - Versión Modular

Sistema modular de gestión WiFi para ESP32 con portal web de configuración.

## 📚 Documentación

### 🚀 Inicio Rápido
- **[⚡ Guía Rápida (5 min)](docs/getting-started/QUICK_START.md)** - Empieza aquí
- [Instalación Completa](docs/getting-started/INSTALLATION.md)
- [Primeros Pasos](docs/getting-started/FIRST_STEPS.md)

### 📖 Guías
- [🔄 Actualización OTA](docs/guides/OTA_USAGE.md) - Actualizar firmware por WiFi
- [📡 Configuración WiFi](docs/guides/WIFI_SETUP.md)
- [🔧 Solución de Problemas](docs/guides/TROUBLESHOOTING.md)
- [🔄 Migración de Código](docs/guides/MIGRATION_GUIDE.md)

### 🔧 Referencia API
- [Resumen de Módulos](docs/api/MODULES_API.md)
- [WiFiManager API](docs/api/WIFIMANAGER_API.md)
- [OTAManager API](docs/api/OTAMANAGER_API.md)
- [FileManager API](docs/api/FILEMANAGER_API.md)
- [Ver todos →](docs/api/)

### 💡 Ejemplos
- [Uso Básico](docs/examples/BASIC_USAGE.md)
- [Uso Avanzado](docs/examples/ADVANCED_USAGE.md)
- [Crear Módulos Personalizados](docs/examples/CUSTOM_MODULES.md)

**📚 [Ver documentación completa →](docs/)**

- ✅ **Portal de Configuración WiFi** - Interfaz web amigable
- ✅ **Soporte DHCP y IP Estática** - Máxima flexibilidad
- ✅ **Reconexión Automática** - Recuperación ante pérdida de conexión
- ✅ **Indicadores LED** - Estados visuales del sistema
- ✅ **Almacenamiento Persistente** - Configuración guardada en LittleFS
- ✅ **Arquitectura Modular** - Código organizado y escalable
- ✅ **Control Remoto** - Interfaz web para controlar el dispositivo
- ✅ **OTA Updates** - Actualización de firmware por WiFi

## 📁 Estructura del Proyecto

```
ESP32_WiFiManager/
├── src/
│   ├── main.cpp                    # Archivo principal (80 líneas)
│   ├── config/
│   │   └── Config.h                # Configuraciones globales
│   ├── wifi/
│   │   ├── WiFiManager.h
│   │   └── WiFiManager.cpp         # Gestión WiFi
│   ├── storage/
│   │   ├── FileManager.h
│   │   └── FileManager.cpp         # Sistema de archivos
│   ├── web/
│   │   ├── MyWebServer.h
│   │   └── MyWebServer.cpp         # Servidor HTTP
│   ├── led/
│   │   ├── LEDController.h
│   │   └── LEDController.cpp       # Control del LED
│   ├── ota/
│   │   ├── OTAManager.h
│   │   └── OTAManager.cpp          # Actualización OTA
│   └── utils/
│       ├── Validators.h
│       └── Validators.cpp          # Validaciones
├── data/
│   ├── index.html                  # Panel de control
│   └── wifimanager.html            # Portal de configuración
├── platformio.ini
└── README.md
```

## 🚀 Inicio Rápido

### Requisitos

- PlatformIO IDE (VSCode extension recomendada)
- ESP32 Development Board
- Cable USB

### Instalación

1. **Clonar o descargar el proyecto**

2. **Subir archivos HTML al ESP32**
   ```bash
   pio run --target uploadfs
   ```

3. **Compilar y subir el código**
   ```bash
   pio run --target upload
   ```

4. **Monitor serial** (opcional)
   ```bash
   pio device monitor
   ```

## 📖 Uso

### Primera Configuración

1. **El ESP32 inicia en modo AP**
   - SSID: `ESP-WIFI-MANAGER`
   - Password: `12345678`

2. **Conectarse al AP desde tu dispositivo**

3. **Abrir navegador y visitar**
   ```
   http://192.168.4.1
   ```

4. **Completar formulario de configuración**
   - Nombre de tu red WiFi (SSID)
   - Contraseña WiFi
   - Elegir DHCP o IP estática

5. **Guardar y esperar**
   - El ESP32 se reiniciará
   - Se conectará a tu red WiFi

### Uso Normal

Una vez configurado:

1. **Encontrar la IP del ESP32**
   - En el router (DHCP)
   - O usar la IP estática configurada

2. **Acceder al panel de control**
   ```
   http://[IP_DEL_ESP32]
   ```

3. **Controlar el LED**
   - Botón Encender
   - Botón Apagar

4. **Ver información de red**
   - SSID conectado
   - Dirección IP
   - Gateway
   - Subnet

### Restablecer Configuración

Desde el panel de control web:
- Botón "Restablecer Configuración WiFi"
- El ESP32 volverá al modo AP

## 🔄 Actualización OTA

Una vez conectado a WiFi, puedes actualizar el firmware sin cable USB.

### Configuración

En `src/config/Config.h`:
```cpp
#define OTA_HOSTNAME "ESP32-OTA"
#define OTA_PASSWORD "admin123"  // ⚠️ CAMBIA ESTO
#define OTA_ENABLED true
```

### Uso con PlatformIO

Agrega a `platformio.ini`:
```ini
upload_protocol = espressif
upload_port = ESP32-OTA.local
upload_flags = 
    --auth=admin123
    --port=3232
```

Luego simplemente:
```bash
pio run --target upload
```

### Uso con Arduino IDE

1. Tools → Port → Selecciona `ESP32-OTA at [IP]`
2. Click en Upload

**📖 Ver [OTA_USAGE.md](OTA_USAGE.md) para guía completa**

## 🎨 Personalización

### Cambiar Credenciales del AP

En `src/config/Config.h`:

```cpp
#define AP_SSID "MI-ESP32"
#define AP_PASSWORD "MiPassword123"
```

### Cambiar Pin del LED

```cpp
#define LED_PIN 2  // Cambiar al GPIO deseado
```

### Cambiar Timeouts

```cpp
#define WIFI_CONNECT_TIMEOUT 10000   // 10 segundos
#define WIFI_CHECK_INTERVAL 30000    // 30 segundos
```

### Desactivar Debug Serial

```cpp
#define DEBUG_SERIAL false
```

## 🏗️ Arquitectura

### Patrón de Diseño: Singleton

Todos los módulos principales usan el patrón Singleton:

```cpp
FileManager* fm = FileManager::getInstance();
WiFiManager* wm = WiFiManager::getInstance();
LEDController* led = LEDController::getInstance();
WebServer* ws = WebServer::getInstance();
```

### Flujo de Inicialización

```
main.cpp
   │
   ├─> FileManager::begin()
   │   └─> Monta LittleFS
   │
   ├─> LEDController::begin()
   │   └─> Configura GPIO
   │
   ├─> WiFiManager::begin()
   │   ├─> Lee configuración
   │   └─> Intenta conectar
   │       ├─> ✓ Conectado → Modo Station
   │       └─> ✗ Fallo → Modo AP
   │
   └─> WebServer::begin(isAPMode)
       ├─> Modo Station: Rutas de control
       └─> Modo AP: Rutas de configuración
```

### Estados del LED

| Estado | Significado |
|--------|------------|
| Apagado | Sistema inactivo |
| Parpadeando | Conectando a WiFi / Modo AP |
| Encendido fijo | Conectado a WiFi |

## 🔧 Agregar Nuevas Funcionalidades

### Ejemplo: Agregar un Sensor

1. **Crear módulo**
   ```
   src/sensors/
   ├── DHTSensor.h
   └── DHTSensor.cpp
   ```

2. **Implementar clase**
   ```cpp
   class DHTSensor {
   private:
       static DHTSensor* instance;
       DHTSensor(int pin);
   
   public:
       static DHTSensor* getInstance(int pin);
       void begin();
       void update();
       float getTemperature();
       float getHumidity();
   };
   ```

3. **Usar en main.cpp**
   ```cpp
   #include "sensors/DHTSensor.h"
   
   DHTSensor* dht;
   
   void setup() {
       // ... código existente ...
       dht = DHTSensor::getInstance(DHT_PIN);
       dht->begin();
   }
   
   void loop() {
       // ... código existente ...
       dht->update();
   }
   ```

## 📚 API de Módulos

### WiFiManager

```cpp
bool begin()                              // Inicializa WiFi
void startAccessPoint()                   // Inicia AP
void checkConnection()                    // Verifica conexión
bool saveConfig(WiFiConfig& config)       // Guarda configuración
WiFiConfig getConfig()                    // Obtiene configuración
String getLocalIP()                       // IP local
void restart()                            // Reinicia ESP32
void resetConfig()                        // Borra configuración
```

### FileManager

```cpp
bool begin()                              // Monta LittleFS
String readFile(const char* path)         // Lee archivo
bool writeFile(path, content)             // Escribe archivo
bool exists(const char* path)             // Verifica existencia
bool deleteFile(const char* path)         // Elimina archivo
void listFiles()                          // Lista archivos
```

### LEDController

```cpp
void begin()                              // Inicializa LED
void update()                             // Actualiza estado (en loop)
void setState(LEDState state)             // Establece estado
void turnOn()                             // Enciende
void turnOff()                            // Apaga
void toggle()                             // Alterna
String getStateString()                   // "ON" o "OFF"
```

## 🐛 Troubleshooting

### El ESP32 no aparece como AP

- Verifica que `AP_PASSWORD` tenga al menos 8 caracteres
- Revisa el monitor serial para ver errores
- Asegúrate de que WiFi no esté deshabilitado por hardware

### No se pueden subir los archivos HTML

```bash
# Verificar que LittleFS esté habilitado en platformio.ini
board_build.filesystem = littlefs

# Subir nuevamente
pio run --target uploadfs
```

### El LED no funciona

- Verifica el pin GPIO en `Config.h`
- Algunos ESP32 tienen el LED interno en GPIO 2
- Revisa conexiones si usas LED externo

### No se guarda la configuración

- Verifica que LittleFS se haya montado correctamente
- Revisa el monitor serial para mensajes de error
- Puede que la partición esté llena (poco probable)

## 📊 Uso de Memoria

Aproximado para ESP32:
- **Flash**: ~300 KB (código + librerías)
- **LittleFS**: ~256 KB (archivos HTML + configuración)
- **RAM**: ~40 KB en tiempo de ejecución

## 🤝 Contribuir

Este proyecto está diseñado para ser extensible. Ideas para mejorar:

- [ ] Soporte para múltiples redes WiFi
- [ ] OTA (Over The Air) updates
- [ ] Integración MQTT
- [ ] Autenticación en panel web
- [ ] API REST completa
- [ ] Integración con Home Assistant

## 📄 Licencia

Este proyecto es de código abierto. Úsalo, modifícalo y compártelo libremente.

## 👨‍💻 Autor

Desarrollado con ❤️ para la comunidad ESP32

---

**¿Necesitas ayuda?** Abre un issue en el repositorio del proyecto.