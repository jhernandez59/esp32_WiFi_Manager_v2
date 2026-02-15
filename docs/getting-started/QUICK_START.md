# ⚡ Guía Rápida - 5 Minutos para Empezar

Esta guía te llevará de cero a un ESP32 funcionando en **menos de 5 minutos**.

## ✅ Pre-requisitos

- [ ] ESP32 conectado por USB
- [ ] PlatformIO IDE instalado (VSCode + extensión)
- [ ] Cable USB funcional

## 🚀 Pasos Rápidos

### 1️⃣ Clonar o Descargar Proyecto (30 seg)

```bash
git clone https://github.com/tu-repo/esp32-wifi-manager.git
cd esp32-wifi-manager
```

### 2️⃣ Abrir en VSCode (10 seg)

```bash
code .
```

O arrastra la carpeta a VSCode.

### 3️⃣ Subir Archivos HTML al ESP32 (1 min)

```bash
pio run --target uploadfs
```

⏳ Espera a que termine (aparecerá "SUCCESS")

### 4️⃣ Subir Código al ESP32 (1.5 min)

```bash
pio run --target upload
```

⏳ Espera la compilación y carga.

### 5️⃣ Abrir Monitor Serial (10 seg)

```bash
pio device monitor
```

Deberías ver:

```
╔════════════════════════════════════╗
║   ESP32 WiFi Manager - Modular     ║
╚════════════════════════════════════╝

⚠ Modo Access Point - Portal de Configuración
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
SSID AP: ESP-WIFI-MANAGER
Password: 12345678
IP: 192.168.4.1
```

### 6️⃣ Configurar WiFi (1 min)

1. **En tu teléfono/PC**, busca WiFi: `ESP-WIFI-MANAGER`
2. **Contraseña**: `12345678`
3. **Abre navegador**: `http://192.168.4.1`
4. **Completa el formulario**:
   - Tu SSID WiFi
   - Contraseña WiFi
   - ✅ Marca "DHCP" (recomendado)
5. **Click en "Guardar y Conectar"**

### 7️⃣ Encontrar tu ESP32 (30 seg)

El ESP32 se reinicia y conecta a tu WiFi. Observa el serial monitor:

```
✓ Modo Station - Conectado a WiFi
IP: 192.168.1.XXX
```

### 8️⃣ ¡Listo! Accede al Panel (10 seg)

Abre navegador: `http://192.168.1.XXX` (usa la IP que apareció)

Verás el panel de control con:
- ✅ Información de red
- ✅ Control del LED
- ✅ Configuración

## 🎉 ¡Felicitaciones!

Ya tienes tu ESP32 WiFi Manager funcionando. Total: **~5 minutos** ⏱️

## 🔄 Próximos Pasos

### Actualización OTA (sin cable USB)

1. **Configura** `platformio.ini`:
   ```ini
   upload_protocol = espressif
   upload_port = ESP32-OTA.local
   upload_flags = 
       --auth=admin123
       --port=3232
   ```

2. **Actualiza por WiFi**:
   ```bash
   pio run --target upload
   ```

🎯 Ver [Guía OTA Completa](../guides/OTA_USAGE.md)

### Personalizar

1. **Cambiar nombre del AP**: Edita `src/config/Config.h`
2. **Cambiar contraseñas**: En el mismo archivo
3. **Agregar sensores**: Ver [Módulos Personalizados](../examples/CUSTOM_MODULES.md)

## ❓ Problemas Comunes

### No compila

```bash
# Limpiar y recompilar
pio run --target clean
pio run
```

### No aparece puerto COM

- Verifica cable USB (usa uno con datos, no solo carga)
- Instala drivers CP2102 o CH340
- Reinicia VSCode

### No encuentra ESP-WIFI-MANAGER

- Espera 30-60 segundos después de encender
- Verifica que el LED parpadee
- Acércate al ESP32

### No se conecta a mi WiFi

- Verifica contraseña (sensible a mayúsculas)
- Verifica que tu WiFi sea 2.4GHz (no 5GHz)
- Revisa serial monitor para errores

## 📚 Más Información

- [Instalación Detallada](INSTALLATION.md) - Setup completo
- [Primeros Pasos](FIRST_STEPS.md) - Tutorial paso a paso
- [Solución de Problemas](../guides/TROUBLESHOOTING.md) - Problemas y soluciones

## 💡 Tips Finales

✅ **Guarda la IP del ESP32** - Anótala para acceso rápido
✅ **Configura OTA** - Actualiza sin cable desde el día 1
✅ **Usa DHCP** - Es más fácil y flexible
✅ **Cambia las contraseñas** - Por seguridad

---

**¿Todo funcionó?** ¡Excelente! Ahora explora el [Tutorial Completo](FIRST_STEPS.md) para aprender más.

**¿Tuviste problemas?** Consulta [Troubleshooting](../guides/TROUBLESHOOTING.md).