# 🔄 Guía de Migración - De Código Monolítico a Modular

Esta guía te ayudará a migrar de tu código actual en un solo archivo a la arquitectura modular.

## 📋 Checklist de Migración

- [ ] Crear estructura de carpetas
- [ ] Crear archivos de módulos
- [ ] Copiar y adaptar código
- [ ] Configurar platformio.ini
- [ ] Subir archivos HTML
- [ ] Compilar y probar
- [ ] Verificar funcionalidad

## 🗂️ Paso 1: Crear Estructura de Carpetas

En la raíz de tu proyecto:

```bash
mkdir -p src/config
mkdir -p src/wifi
mkdir -p src/storage
mkdir -p src/web
mkdir -p src/led
mkdir -p src/utils
mkdir -p data
```

O manualmente crea esta estructura:
```
src/
├── config/
├── wifi/
├── storage/
├── web/
├── led/
└── utils/
data/
```

## 📝 Paso 2: Crear Archivos de los Módulos

### 2.1 Config.h
**Ubicación**: `src/config/Config.h`

Copia el contenido del artefacto "Config.h" que te proporcioné.

Este archivo reemplaza todas las constantes que tenías dispersas en main.cpp.

### 2.2 Validators
**Ubicación**: 
- `src/utils/Validators.h`
- `src/utils/Validators.cpp`

Contiene la función `isValidIP()` que tenías y agrega validaciones adicionales.

### 2.3 FileManager
**Ubicación**:
- `src/storage/FileManager.h`
- `src/storage/FileManager.cpp`

Reemplaza tus funciones:
- `readFile()` → `FileManager::getInstance()->readFile()`
- `writeFile()` → `FileManager::getInstance()->writeFile()`
- `deleteFile()` → `FileManager::getInstance()->deleteFile()`
- `fileExists()` → `FileManager::getInstance()->exists()`

### 2.4 LEDController
**Ubicación**:
- `src/led/LEDController.h`
- `src/led/LEDController.cpp`

Reemplaza tu manejo del LED:
- `digitalWrite(ledPin, HIGH)` → `ledController->turnOn()`
- `digitalWrite(ledPin, LOW)` → `ledController->turnOff()`
- Estados de parpadeo ahora manejados automáticamente

### 2.5 WiFiManager
**Ubicación**:
- `src/wifi/WiFiManager.h`
- `src/wifi/WiFiManager.cpp`

Reemplaza tu función `initWiFi()` y toda la lógica WiFi.

### 2.6 WebServer
**Ubicación**:
- `src/web/WebServer.h`
- `src/web/WebServer.cpp`

Maneja todas las rutas HTTP que antes estaban en `setup()`.

### 2.7 main.cpp
**Ubicación**: `src/main.cpp`

Reemplaza completamente tu archivo main.cpp actual con el nuevo (mucho más simple).

## 🔄 Paso 3: Mapeo de Código

### Tu código actual → Código modular

| Código Actual | Nuevo Código Modular |
|---------------|---------------------|
| `String ssid = readFile(ssidPath)` | `config.ssid = fileManager->readFile(SSID_FILE_PATH)` |
| `writeFile(ssidPath, ssid.c_str())` | `fileManager->writeFile(SSID_FILE_PATH, ssid)` |
| `if (!LittleFS.begin())` | `if (!fileManager->begin())` |
| `WiFi.begin(ssid, pass)` | Manejado por `wifiManager->begin()` |
| `digitalWrite(ledPin, HIGH)` | `ledController->turnOn()` |
| `server.on("/", ...)` | Manejado por `webServer->begin(isAPMode)` |

## 🚀 Paso 4: Proceso de Migración

### 4.1 Respaldo

```bash
# Crear backup de tu código actual
cp src/main.cpp src/main.cpp.backup
```

### 4.2 Copiar Módulos

Copia todos los archivos .h y .cpp de los artefactos a sus respectivas carpetas.

### 4.3 Copiar HTML

Los archivos HTML (`index.html` y `wifimanager.html`) van en la carpeta `data/`.

### 4.4 Actualizar platformio.ini

Reemplaza o actualiza tu `platformio.ini` con el que te proporcioné.

### 4.5 Reemplazar main.cpp

Reemplaza tu `main.cpp` con el nuevo archivo modular.

## ⚙️ Paso 5: Configuración

### 5.1 Ajustar Config.h

Edita `src/config/Config.h` y ajusta:

```cpp
// Cambiar si tu LED está en otro GPIO
#define LED_PIN 2

// Cambiar credenciales del AP
#define AP_SSID "TU-NOMBRE-AP"
#define AP_PASSWORD "TuPassword123"

// Ajustar timeouts si es necesario
#define WIFI_CONNECT_TIMEOUT 10000
```

### 5.2 Verificar Dependencias

En `platformio.ini`, asegúrate de tener:

```ini
lib_deps = 
    ESP Async WebServer@^1.2.3
    AsyncTCP@^1.1.1
```

## 🔨 Paso 6: Compilar

```bash
# Limpiar build anterior
pio run --target clean

# Compilar
pio run

# Si hay errores, verifica que todos los archivos estén en su lugar
```

### Errores Comunes

**Error**: `Config.h: No such file or directory`
- **Solución**: Verifica que Config.h esté en `src/config/Config.h`

**Error**: `undefined reference to FileManager::getInstance()`
- **Solución**: Asegúrate de tener tanto .h como .cpp de cada módulo

**Error**: Problemas con AsyncWebServer
- **Solución**: Verifica las dependencias en platformio.ini

## 📤 Paso 7: Subir al ESP32

### 7.1 Subir Sistema de Archivos (HTML)

```bash
pio run --target uploadfs
```

Esto sube `index.html` y `wifimanager.html` al ESP32.

### 7.2 Subir Código

```bash
pio run --target upload
```

### 7.3 Monitor Serial

```bash
pio device monitor
```

Deberías ver algo como:
```
╔════════════════════════════════════╗
║   ESP32 WiFi Manager - Modular   ║
╚════════════════════════════════════╝

LittleFS mounted successfully
...
```

## ✅ Paso 8: Verificación

### 8.1 Modo Access Point

Si es la primera vez o no hay configuración:

1. Busca la red WiFi: `ESP-WIFI-MANAGER`
2. Contraseña: `12345678`
3. Conecta y ve a: `http://192.168.4.1`
4. Deberías ver el formulario de configuración

### 8.2 Modo Station

Si ya configuraste WiFi:

1. Encuentra la IP del ESP32 (en tu router o serial monitor)
2. Ve a: `http://[IP_DEL_ESP32]`
3. Deberías ver el panel de control
4. Prueba encender/apagar el LED

### 8.3 Funciones a Verificar

- [ ] Portal de configuración WiFi funciona
- [ ] Checkbox DHCP muestra/oculta campos de IP
- [ ] Validación de IPs funciona
- [ ] ESP32 se conecta al WiFi configurado
- [ ] Panel de control es accesible
- [ ] Botones de LED funcionan
- [ ] LED indica estados correctamente
- [ ] Botón de reset funciona
- [ ] Reconexión automática funciona

## 🐛 Troubleshooting

### El código no compila

1. **Verifica estructura de carpetas**
   ```
   src/
   ├── config/Config.h
   ├── utils/
   │   ├── Validators.h
   │   └── Validators.cpp
   ├── storage/
   │   ├── FileManager.h
   │   └── FileManager.cpp
   ...
   ```

2. **Verifica que todos los .cpp tengan su .h correspondiente**

3. **Revisa includes en main.cpp**:
   ```cpp
   #include "config/Config.h"
   #include "storage/FileManager.h"
   #include "wifi/WiFiManager.h"
   #include "led/LEDController.h"
   #include "web/WebServer.h"
   ```

### No encuentra los archivos HTML

```bash
# Asegúrate de que estén en data/
ls data/
# Deberías ver: index.html  wifimanager.html

# Sube nuevamente
pio run --target uploadfs
```

### La configuración no se guarda

- Verifica en serial monitor que LittleFS se montó correctamente
- Prueba listar archivos (debería hacerlo automáticamente si DEBUG_SERIAL está activo)

## 📊 Comparación: Antes vs Después

### Antes (Monolítico)
```
main.cpp: ~250 líneas
- Todo mezclado
- Difícil de mantener
- Difícil de testear
- Difícil de extender
```

### Después (Modular)
```
main.cpp: ~80 líneas
Config.h: ~50 líneas
Validators: ~60 líneas
FileManager: ~150 líneas
LEDController: ~100 líneas
WiFiManager: ~200 líneas
WebServer: ~200 líneas
──────────────────────
Total: ~840 líneas en módulos organizados
```

**Beneficios**:
- ✅ Código organizado y mantenible
- ✅ Fácil agregar nuevas funcionalidades
- ✅ Cada módulo se puede testear independientemente
- ✅ Reutilizable en otros proyectos
- ✅ Más fácil de entender y documentar

## 🎯 Próximos Pasos

Una vez migrado exitosamente, puedes:

1. **Agregar MQTT** - Crear módulo `mqtt/MQTTClient`
2. **Agregar Sensores** - Crear módulo `sensors/`
3. **Agregar OTA** - Crear módulo `ota/OTAManager`
4. **Agregar Display** - Crear módulo `display/`

Cada uno siguiendo el mismo patrón modular.

## 💡 Consejos

1. **Migra gradualmente** - Puedes ir módulo por módulo si prefieres
2. **Usa control de versiones** - Git es tu amigo
3. **Prueba cada módulo** - Asegúrate de que funciona antes de continuar
4. **Lee los comentarios** - Cada módulo tiene documentación inline
5. **Personaliza** - Adapta los módulos a tus necesidades específicas

---

**¿Problemas con la migración?** Revisa el README.md o consulta los comentarios en el código.