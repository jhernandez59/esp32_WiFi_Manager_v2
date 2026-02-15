# 🔧 Guía de Configuración - Config.h vs Config_local.h

## 📁 Estructura de Archivos de Configuración

```
src/config/
├── Config.h                    ✅ Commiteado a Git
│   └── Valores por defecto y configuración pública
│
├── Config_local.h.example      ✅ Commiteado a Git
│   └── Template con instrucciones
│
└── Config_local.h             ❌ NO commiteado (en .gitignore)
    └── Tus valores personales y credenciales
```

## 🎯 Propósito de Cada Archivo

### 1️⃣ **Config.h** (Público - Commiteado)

**Contiene:**
- ✅ Configuraciones generales del proyecto
- ✅ Pines GPIO
- ✅ Timeouts e intervalos
- ✅ Rutas de archivos
- ✅ Valores por defecto seguros
- ✅ Mensajes del sistema

**NO contiene:**
- ❌ Contraseñas reales
- ❌ API keys
- ❌ Credenciales de producción

**Características:**
- Incluye automáticamente `Config_local.h` si existe
- Usa `#ifndef` para permitir sobrescritura
- Muestra mensajes de compilación informativos

### 2️⃣ **Config_local.h.example** (Template - Commiteado)

**Propósito:**
- 📝 Template de ejemplo para otros desarrolladores
- 📝 Documentación de qué configurar
- 📝 Instrucciones claras de uso

**Uso:**
```bash
# Los nuevos desarrolladores hacen:
cp Config_local.h.example Config_local.h
# Luego editan Config_local.h con sus valores
```

### 3️⃣ **Config_local.h** (Privado - NO Commiteado)

**Contiene:**
- 🔒 Contraseñas OTA reales
- 🔒 API keys
- 🔒 Tokens de servicios
- 🔒 Credenciales MQTT
- 🔒 Hostnames personalizados
- 🔒 Cualquier valor que no quieres compartir

**Características:**
- Sobrescribe valores de `Config.h`
- Usa `#undef` y `#define` para reemplazar valores
- Protegido por `.gitignore`

## 🚀 Setup Inicial (Primera Vez)

### Paso 1: Clonar Proyecto

```bash
git clone tu-repositorio.git
cd tu-repositorio
```

### Paso 2: Crear Config_local.h

```bash
cd src/config
cp Config_local.h.example Config_local.h
```

### Paso 3: Editar Config_local.h

Abre `Config_local.h` y personaliza:

```cpp
// Cambiar hostname
#undef OTA_HOSTNAME
#define OTA_HOSTNAME "ESP32-MiNombre"

// Cambiar password OTA
#undef OTA_PASSWORD
#define OTA_PASSWORD "MiPasswordSeguro123!"
```

### Paso 4: Verificar

Compila el proyecto. Deberías ver en la salida:

```
✓ Usando Config_local.h - Configuración personalizada cargada
```

Si NO creaste `Config_local.h`, verás:

```
⚠ Config_local.h no encontrado - Usando valores por defecto
  Para producción: Copia Config_local.h.example a Config_local.h
```

## 🔄 Cómo Funciona la Sobrescritura

### Ejemplo 1: Cambiar OTA_HOSTNAME

**Config.h** (valores por defecto):
```cpp
#ifndef OTA_HOSTNAME
  #define OTA_HOSTNAME "ESP32-OTA"
#endif
```

**Config_local.h** (tus valores):
```cpp
#undef OTA_HOSTNAME
#define OTA_HOSTNAME "ESP32-Cocina"
```

**Resultado:** Se usa `"ESP32-Cocina"`

### Ejemplo 2: Agregar Nueva Configuración

**Config_local.h**:
```cpp
// Esta configuración solo existe en tu entorno local
#define MQTT_SERVER "mqtt.miservidor.com"
#define MQTT_PORT 1883
```

**Usar en código:**
```cpp
#ifdef MQTT_SERVER
  mqtt.connect(MQTT_SERVER, MQTT_PORT);
#else
  Serial.println("MQTT no configurado");
#endif
```

## 📝 Patrón de Uso Recomendado

### Para Valores Seguros (Config.h)

```cpp
// Valor público que todos pueden ver
#define LED_PIN 2
#define WEB_SERVER_PORT 80
```

### Para Valores Sensibles (Config_local.h)

```cpp
// Valor privado que solo tú ves
#undef OTA_PASSWORD
#define OTA_PASSWORD "TuPasswordReal"
```

## 🔒 Buenas Prácticas de Seguridad

### ✅ HACER:

1. **Siempre usar Config_local.h para credenciales**
   ```cpp
   // En Config_local.h
   #define API_KEY "sk-abc123..."
   ```

2. **Verificar que Config_local.h esté en .gitignore**
   ```bash
   git check-ignore src/config/Config_local.h
   # Debe retornar: src/config/Config_local.h
   ```

3. **Usar passwords diferentes para desarrollo y producción**
   ```cpp
   // Desarrollo
   #define OTA_PASSWORD "dev123"
   
   // Producción
   #define OTA_PASSWORD "Pr0d_$ecure_P@ssw0rd!"
   ```

4. **Documentar en Config_local.h.example**
   ```cpp
   // Template claro con instrucciones
   // #define API_KEY "tu_api_key_aqui"
   ```

### ❌ NO HACER:

1. **NO hardcodear credenciales en Config.h**
   ```cpp
   // ❌ MAL - Esto irá a Git
   #define OTA_PASSWORD "miPasswordReal123"
   ```

2. **NO commitear Config_local.h**
   ```bash
   # Verificar antes de commit
   git status
   # No debe aparecer Config_local.h
   ```

3. **NO usar el mismo password en múltiples lugares**
   ```cpp
   // ❌ MAL
   #define OTA_PASSWORD "admin123"
   #define MQTT_PASSWORD "admin123"
   ```

## 🎨 Personalización por Entorno

### Desarrollo

**Config_local.h**:
```cpp
#undef OTA_HOSTNAME
#define OTA_HOSTNAME "ESP32-DEV"

#undef DEBUG_SERIAL
#define DEBUG_SERIAL true

#define TEST_MODE true
```

### Producción

**Config_local.h**:
```cpp
#undef OTA_HOSTNAME
#define OTA_HOSTNAME "ESP32-Prod-001"

#undef DEBUG_SERIAL
#define DEBUG_SERIAL false

#define PRODUCTION_MODE true
```

### Testing

**Config_local.h**:
```cpp
#undef OTA_HOSTNAME
#define OTA_HOSTNAME "ESP32-Test"

#define MOCK_SENSORS true
#define TEST_WIFI_SSID "TestNetwork"
```

## 🔍 Troubleshooting

### Problema: "OTA_HOSTNAME was not declared"

**Causa:** Config_local.h usa `#define` sin `#undef` primero

**Solución:**
```cpp
// ❌ MAL
#define OTA_HOSTNAME "MiNombre"

// ✅ BIEN
#undef OTA_HOSTNAME
#define OTA_HOSTNAME "MiNombre"
```

### Problema: Sigue usando valores por defecto

**Verificar:**
```bash
# ¿Existe el archivo?
ls -la src/config/Config_local.h

# ¿Está en la ruta correcta?
# Debe estar en src/config/, no en src/
```

### Problema: Valores no se sobrescriben

**Causa:** Config.h no usa `#ifndef`

**Solución en Config.h:**
```cpp
// ✅ BIEN - Permite sobrescritura
#ifndef OTA_HOSTNAME
  #define OTA_HOSTNAME "ESP32-OTA"
#endif
```

### Problema: Accidentalmente commiteé Config_local.h

**Solución inmediata:**
```bash
# 1. Remover del stage
git reset HEAD src/config/Config_local.h

# 2. Verificar que esté en .gitignore
echo "src/config/Config_local.h" >> .gitignore

# 3. Si ya hiciste commit:
git rm --cached src/config/Config_local.h
git commit -m "Remove Config_local.h from tracking"

# 4. IMPORTANTE: Cambiar todas las credenciales que exponiste
```

## 📚 Recursos

- [Git .gitignore Documentation](https://git-scm.com/docs/gitignore)
- [C++ Preprocessor Directives](https://en.cppreference.com/w/cpp/preprocessor)
- [Environment Variables Best Practices](https://12factor.net/config)

## ✅ Checklist Final

- [ ] `Config.h` contiene solo valores públicos
- [ ] `Config.h` usa `#ifndef` para valores sobrescribibles
- [ ] `Config_local.h.example` tiene instrucciones claras
- [ ] `Config_local.h` existe localmente (NO en Git)
- [ ] `Config_local.h` está en `.gitignore`
- [ ] Credenciales reales solo en `Config_local.h`
- [ ] Proyecto compila mostrando mensaje de Config_local.h
- [ ] `git status` NO muestra `Config_local.h`

---

**Resumen:** Config.h = Público (Git), Config_local.h = Privado (Local), Config_local.h.example = Template (Git)