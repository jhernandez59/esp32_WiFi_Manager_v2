# 🔄 Guía de Uso - OTA (Over-The-Air Updates)

## ¿Qué es OTA?

OTA (Over-The-Air) te permite actualizar el firmware de tu ESP32 a través de WiFi, **sin necesidad de conectar el cable USB**. Esto es especialmente útil cuando:

- El ESP32 está montado en un lugar de difícil acceso
- Quieres actualizar múltiples dispositivos remotamente
- Desarrollas y necesitas actualizar rápidamente

## 🚀 Configuración Inicial

### 1. Configurar en Config.h

Edita `src/config/Config.h`:

```cpp
#define OTA_HOSTNAME "ESP32-OTA"     // Nombre único para tu dispositivo
#define OTA_PASSWORD "admin123"      // Contraseña (IMPORTANTE por seguridad)
#define OTA_ENABLED true             // Habilitar OTA
```

**Importante:** 
- ⚠️ **Siempre usa contraseña** para evitar actualizaciones no autorizadas
- 📝 Si tienes múltiples ESP32, usa nombres únicos (ej: "ESP32-Salon", "ESP32-Cocina")

### 2. Primera Carga (Por USB)

La primera vez **DEBES** cargar el firmware por USB:

```bash
pio run --target upload
```

Después de esto, todas las actualizaciones pueden ser por OTA.

## 📡 Métodos de Actualización OTA

### Método 1: PlatformIO (Recomendado)

#### Paso 1: Descubrir dispositivo

PlatformIO detecta automáticamente dispositivos OTA en tu red.

#### Paso 2: Configurar platformio.ini

Agrega esto a tu `platformio.ini`:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

; Configuración OTA
upload_protocol = espressif
upload_port = ESP32-OTA.local
upload_flags = 
    --auth=admin123
    --port=3232
```

#### Paso 3: Subir código

```bash
# Por línea de comandos
pio run --target upload

# O en VSCode: Click en el botón "Upload" de PlatformIO
```

### Método 2: Arduino IDE

#### Paso 1: Instalar Python

OTA en Arduino IDE requiere Python. Descarga desde [python.org](https://www.python.org/downloads/)

#### Paso 2: Seleccionar puerto OTA

1. Abre Arduino IDE
2. Ve a: **Tools → Port**
3. Verás algo como: `ESP32-OTA at 192.168.1.XXX`
4. Selecciona ese puerto

#### Paso 3: Subir sketch

Simplemente haz click en "Upload" como siempre.

### Método 3: Usando espota.py (Avanzado)

Desde la terminal:

```bash
cd ~/.platformio/packages/framework-arduinoespressif32/tools

python espota.py -i 192.168.1.XXX -p 3232 -f firmware.bin -a admin123
```

Donde:
- `192.168.1.XXX` es la IP de tu ESP32
- `3232` es el puerto OTA
- `firmware.bin` es tu archivo compilado
- `admin123` es tu contraseña

## 🔍 Encontrar tu ESP32 en la Red

### Opción 1: Revisar router

Ve a la configuración de tu router y busca el dispositivo con nombre `ESP32-OTA`

### Opción 2: Monitor Serial

Al iniciar, el ESP32 muestra:

```
╔═══════════════════════════════════╗
║      OTA INICIALIZADO ✓           ║
╚═══════════════════════════════════╝
Hostname: ESP32-OTA
Puerto: 3232
IP: 192.168.1.XXX
```

### Opción 3: mDNS (Bonjour)

Puedes acceder usando: `http://ESP32-OTA.local`

### Opción 4: Escaneo de red

En Linux/Mac:
```bash
# Escanear red
arp -a | grep esp

# O usar nmap
nmap -sn 192.168.1.0/24
```

En Windows:
```powershell
arp -a
```

## 📊 Monitoreo de Actualización

Durante la actualización OTA, verás en el Monitor Serial:

```
╔═══════════════════════════════════╗
║    INICIANDO ACTUALIZACIÓN OTA    ║
╚═══════════════════════════════════╝
Tipo: sketch
Progreso: 10%
Progreso: 20%
Progreso: 30%
...
Progreso: 100%
╔═══════════════════════════════════╗
║   ACTUALIZACIÓN COMPLETADA ✓      ║
╚═══════════════════════════════════╝
Reiniciando...
```

El LED también indica el estado:
- 🔴 **Apagado al inicio** - Preparando actualización
- 💛 **Parpadeando** - Actualizando
- 🟢 **Encendido fijo** - Completado
- 🔴 **Parpadeo rápido** - Error

## ⚠️ Problemas Comunes

### Error: "No answer from device"

**Causa:** El ESP32 no responde

**Soluciones:**
1. Verifica que el ESP32 esté encendido y conectado a WiFi
2. Comprueba la IP en el router o serial monitor
3. Verifica que el puerto 3232 no esté bloqueado por firewall
4. Asegúrate de estar en la misma red WiFi

### Error: "Authentication failed"

**Causa:** Contraseña incorrecta

**Solución:** Verifica que la contraseña en `upload_flags` coincida con `OTA_PASSWORD`

### Error: "espota.py not found"

**Causa:** Python no instalado o no encontrado

**Soluciones:**
1. Instala Python desde [python.org](https://www.python.org/downloads/)
2. En Windows, marca "Add Python to PATH" durante instalación
3. Reinicia Arduino IDE/VSCode después de instalar Python

### El dispositivo no aparece en puertos

**Soluciones:**
1. Espera 30-60 segundos después de encender el ESP32
2. Verifica que esté conectado a WiFi (revisa serial monitor)
3. Reinicia Arduino IDE/VSCode
4. Verifica que mDNS funcione: `ping ESP32-OTA.local`

### Actualización se queda en 0%

**Causas posibles:**
1. Firmware demasiado grande para la partición
2. Conexión WiFi inestable
3. Memoria insuficiente

**Soluciones:**
1. Reduce el tamaño del código
2. Acércate al router WiFi
3. Verifica particiones en `platformio.ini`

## 🔒 Seguridad

### ⚠️ MUY IMPORTANTE

**NUNCA dejes OTA sin contraseña en producción**

Alguien en tu red podría:
- Subir código malicioso
- Bloquear tu dispositivo
- Robar información

### Buenas Prácticas

1. ✅ **Usa contraseñas fuertes**
   ```cpp
   #define OTA_PASSWORD "MiPassword$egur0_2024!"
   ```

2. ✅ **Cambia el hostname por defecto**
   ```cpp
   #define OTA_HOSTNAME "MiProyecto-001"
   ```

3. ✅ **Deshabilita OTA en producción si no es necesario**
   ```cpp
   #define OTA_ENABLED false
   ```

4. ✅ **Usa una red WiFi dedicada para tus dispositivos IoT**

5. ✅ **Considera agregar autenticación adicional en tu código**

## 🎯 Tips y Trucos

### Actualización Condicional

Puedes habilitar/deshabilitar OTA dinámicamente:

```cpp
// Deshabilitar OTA temporalmente
otaManager->setEnabled(false);

// Habilitarlo nuevamente
otaManager->setEnabled(true);
```

### Cambiar Hostname Dinámicamente

```cpp
String uniqueName = "ESP32-" + WiFi.macAddress();
otaManager->setHostname(uniqueName);
```

### Actualizar Solo en Horarios Específicos

```cpp
void loop() {
    // Solo permitir OTA entre 2am y 5am
    int hour = // obtener hora actual
    if (hour >= 2 && hour <= 5) {
        otaManager->handle();
    }
}
```

## 📦 Actualizar LittleFS (Archivos)

También puedes actualizar el sistema de archivos por OTA:

### PlatformIO

```bash
pio run --target uploadfs
```

### Arduino IDE

Usa el ESP32 Sketch Data Upload plugin

## 🔄 Rollback (Volver a Versión Anterior)

Si una actualización falla, el ESP32 NO se actualizará (seguridad).

Para volver a una versión anterior:
1. Ten el código anterior guardado en Git
2. Haz checkout del commit anterior
3. Sube por OTA o USB

**Tip:** Usa Git tags para versiones estables:
```bash
git tag -a v1.0.0 -m "Versión estable 1.0.0"
```

## 📝 Checklist Pre-Actualización

Antes de cada actualización OTA, verifica:

- [ ] El código compila sin errores
- [ ] Probaste los cambios en un dispositivo de prueba
- [ ] El ESP32 objetivo está conectado a WiFi
- [ ] Tienes la IP o hostname correcto
- [ ] La contraseña es correcta
- [ ] El LED está respondiendo (indica que está vivo)
- [ ] Tienes acceso físico al ESP32 por si falla

## 🆘 Plan de Emergencia

Si una actualización OTA sale mal:

1. **Espera 2-3 minutos** - El ESP32 podría estar reiniciando
2. **Revisa el LED** - ¿Está parpadeando? Está actualizando
3. **Conecta por USB** y revisa el serial monitor
4. **Carga firmware anterior** por USB si es necesario
5. **Considera agregar un watchdog** para auto-recovery

## 📚 Recursos Adicionales

- [Documentación Arduino OTA](https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html)
- [ESP32 OTA Updates](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/ota.html)
- [PlatformIO OTA Upload](https://docs.platformio.org/en/latest/platforms/espressif32.html#over-the-air-ota-update)

---

**¿Tienes problemas?** Revisa el serial monitor primero, suele dar pistas claras del problema.