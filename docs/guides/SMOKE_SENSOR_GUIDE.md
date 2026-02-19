# 🔥 Guía de Uso - Sensor de Humo MEMS (Fermion)

## 📋 Especificaciones del Sensor

**Sensor:** Fermion MEMS Smoke Gas Sensor  
**Tipo de salida:** Analógica (0-3.3V)  
**Alimentación:** 3.3V  
**Tiempo de calentamiento:** 60 segundos recomendado  
**Rango de detección:** Variable según concentración de humo  

## 🔌 Conexión Hardware

### Pines del Sensor

```
Fermion MEMS Smoke:
┌──────────┐
│ A   VCC  GND │
└──────────┘
  │    │    │
  │    │    └──→ GND ESP32
  │    └───────→ 3.3V ESP32
  └────────────→ GPIO36 (VP) ESP32
```

### Diagrama de Conexión

```
ESP32 DevKit           Fermion MEMS Smoke
┌─────────────┐        ┌──────────────┐
│             │        │              │
│  3.3V   ────┼────────┼──→ VCC       │
│             │        │              │
│  GND    ────┼────────┼──→ GND       │
│             │        │              │
│  GPIO36 ←───┼────────┼──→ A         │
│  (VP)       │        │              │
└─────────────┘        └──────────────┘
```

**⚠️ IMPORTANTE:**
- Usar GPIO36 (VP) o GPIO39 (VN) - Son ADC1
- NO usar GPIO 0, 2, 4, 12-15, 25-27 (ADC2 - conflicto con WiFi)

## 🚀 Instalación del Módulo

### 1. Crear Archivos del Módulo

```bash
# Crear carpeta de sensores
mkdir -p src/sensors

# Crear archivos
touch src/sensors/SmokeSensor.h
touch src/sensors/SmokeSensor.cpp
```

### 2. Copiar Código

Copia el contenido de los artifacts:
- `SmokeSensor.h` → `src/sensors/SmokeSensor.h`
- `SmokeSensor.cpp` → `src/sensors/SmokeSensor.cpp`

### 3. Actualizar Config.h

Ya está actualizado con:
```cpp
#define SMOKE_SENSOR_PIN 36  // GPIO36 (VP)
```

## 📝 Uso Básico

### Inicialización Simple

```cpp
#include "sensors/SmokeSensor.h"

SmokeSensor* smokeSensor;

void setup() {
    Serial.begin(115200);
    
    // Inicializar sensor (con warmup de 60 seg)
    smokeSensor = SmokeSensor::getInstance(SMOKE_SENSOR_PIN);
    smokeSensor->begin(true);
}

void loop() {
    // Leer sensor cada 5 segundos
    static unsigned long lastRead = 0;
    if (millis() - lastRead >= 5000) {
        lastRead = millis();
        
        SmokeReading reading = smokeSensor->read();
        
        Serial.printf("Estado: %s\n", smokeSensor->getStateString().c_str());
        Serial.printf("PPM: %d\n", reading.ppm);
        Serial.printf("Porcentaje: %d%%\n", reading.percentage);
    }
}
```

## 🎯 Calibración del Sensor

### ¿Cuándo Calibrar?

- ✅ Primera vez que usas el sensor
- ✅ Después de cambiar de ubicación
- ✅ Si las lecturas parecen incorrectas
- ✅ Cada 1-3 meses en ambientes estables

### Proceso de Calibración

**REQUISITO:** Ambiente con AIRE LIMPIO (sin humo, cocina, incienso, etc)

```cpp
void setup() {
    Serial.begin(115200);
    
    smokeSensor = SmokeSensor::getInstance(SMOKE_SENSOR_PIN);
    smokeSensor->begin(true);  // Esperar warmup
    
    // Esperar a que termine el warmup (60 seg)
    while (!smokeSensor->isReady()) {
        delay(1000);
        Serial.println("Calentando sensor...");
    }
    
    Serial.println("¡ATENCIÓN! Calibración en 5 segundos");
    Serial.println("Asegúrate de tener AIRE LIMPIO");
    delay(5000);
    
    // Calibrar: 300 muestras, 1 segundo entre muestras (5 minutos total)
    smokeSensor->calibrate(300, 1000);
    
    Serial.println("✓ Calibración completada y guardada");
}
```

### Calibración Rápida (Testing)

```cpp
// Solo para pruebas - 30 segundos
smokeSensor->calibrate(30, 1000);
```

### Calibración Completa (Producción)

```cpp
// Para producción - 10 minutos
smokeSensor->calibrate(600, 1000);
```

## 📊 Interpretación de Lecturas

### Estructura SmokeReading

```cpp
struct SmokeReading {
    int rawValue;        // Valor ADC (0-4095)
    float voltage;       // Voltaje (0-3.3V)
    int percentage;      // Porcentaje 0-100%
    int ppm;            // Partes por millón
    SmokeState state;   // Estado actual
    unsigned long timestamp;
};
```

### Estados del Sensor

| Estado | Descripción | Acción |
|--------|-------------|--------|
| `INITIALIZING` | Calentando | Esperar 60 seg |
| `NORMAL` | Aire limpio | Monitoreo normal |
| `DETECTED` | Humo presente | Alerta de precaución |
| `CRITICAL` | Nivel peligroso | ¡EVACUAR! |
| `ERROR` | Fallo sensor | Verificar conexión |

### Umbrales Típicos (Post-Calibración)

```cpp
// Estos valores se calculan automáticamente al calibrar
Baseline Avg:     250  (aire limpio)
Threshold Caution: 400  (humo ligero)
Threshold Warning: 800  (humo moderado)
Threshold Alarm:  1500  (humo denso)
```

## 🔍 Ejemplos de Uso Avanzado

### Ejemplo 1: Sistema de Alerta con LED

```cpp
void loop() {
    SmokeReading reading = smokeSensor->read();
    
    switch (reading.state) {
        case SmokeState::NORMAL:
            digitalWrite(GREEN_LED, HIGH);
            digitalWrite(YELLOW_LED, LOW);
            digitalWrite(RED_LED, LOW);
            break;
            
        case SmokeState::DETECTED:
            digitalWrite(GREEN_LED, LOW);
            digitalWrite(YELLOW_LED, HIGH);
            digitalWrite(RED_LED, LOW);
            Serial.println("⚠️ Humo detectado");
            break;
            
        case SmokeState::CRITICAL:
            digitalWrite(GREEN_LED, LOW);
            digitalWrite(YELLOW_LED, LOW);
            digitalWrite(RED_LED, HIGH);
            tone(BUZZER_PIN, 2000); // Alarma
            Serial.println("🚨 ¡NIVEL CRÍTICO!");
            break;
    }
}
```

### Ejemplo 2: Notificación por WiFi

```cpp
void checkSmokeAndNotify() {
    SmokeReading reading = smokeSensor->read();
    
    if (smokeSensor->isCritical()) {
        // Enviar notificación urgente
        sendMQTTAlert("CRITICAL", reading.ppm);
        sendPushNotification("¡HUMO DETECTADO!");
        activateSiren();
    }
    else if (smokeSensor->isSmokeDetected()) {
        // Notificación de advertencia
        sendMQTTAlert("WARNING", reading.ppm);
    }
}
```

### Ejemplo 3: Logging de Datos

```cpp
void logSmokeData() {
    SmokeReading reading = smokeSensor->read();
    
    String logEntry = String(millis()) + "," +
                     String(reading.rawValue) + "," +
                     String(reading.ppm) + "," +
                     smokeSensor->getStateString() + "\n";
    
    FileManager::getInstance()->appendFile("/smoke_log.csv", logEntry);
}
```

### Ejemplo 4: Promedio Móvil (Reducir Falsos Positivos)

```cpp
class SmokeMonitor {
private:
    static const int HISTORY_SIZE = 5;
    int history[HISTORY_SIZE];
    int index = 0;
    
public:
    bool checkForSmoke() {
        SmokeReading reading = smokeSensor->read();
        
        // Guardar en historial
        history[index] = reading.rawValue;
        index = (index + 1) % HISTORY_SIZE;
        
        // Calcular promedio
        int sum = 0;
        for (int i = 0; i < HISTORY_SIZE; i++) {
            sum += history[i];
        }
        int avg = sum / HISTORY_SIZE;
        
        // Alarma solo si el promedio supera umbral
        return avg > smokeSensor->getCalibration().thresholdAlarm;
    }
};
```

## 🐛 Troubleshooting

### Problema: Sensor siempre en INITIALIZING

**Causa:** No ha pasado el tiempo de warmup (60 seg)

**Solución:**
```cpp
while (!smokeSensor->isReady()) {
    delay(1000);
}
```

### Problema: Lecturas erráticas o muy altas

**Causa:** Sensor no calibrado o cerca de fuentes de humo

**Solución:**
1. Alejar de cocina, velas, incienso
2. Recalibrar en aire limpio
3. Verificar conexiones

### Problema: Siempre muestra estado NORMAL

**Causa:** Umbrales muy altos o sensor defectuoso

**Solución:**
```cpp
// Ver valores actuales
Serial.printf("Raw: %d\n", reading.rawValue);
Serial.printf("Threshold: %d\n", smokeSensor->getCalibration().thresholdWarning);

// Si raw < 100, verificar conexión
// Si raw no cambia con humo, sensor defectuoso
```

### Problema: ERROR state

**Causa:** Valor ADC fuera de rango (0-4095)

**Solución:**
1. Verificar conexión física
2. Verificar alimentación 3.3V
3. Verificar pin GPIO correcto

### Problema: Calibración no se guarda

**Causa:** Error en LittleFS

**Solución:**
```cpp
// Verificar que LittleFS esté montado
FileManager::getInstance()->begin();

// Verificar espacio disponible
// Manualmente guardar:
smokeSensor->saveCalibration();
```

## ⚙️ Configuración Avanzada

### Cambiar Pin del Sensor

```cpp
// En Config.h
#define SMOKE_SENSOR_PIN 39  // Cambiar a GPIO39 (VN)

// En código
smokeSensor = SmokeSensor::getInstance(39);
```

### Ajustar Tiempo de Warmup

```cpp
// En SmokeSensor.h, cambiar:
static const unsigned long WARMUP_TIME = 30000; // 30 seg en lugar de 60
```

### Ajustar Sensibilidad

```cpp
// Después de calibrar, ajustar manualmente umbrales
SmokeCalibration cal = smokeSensor->getCalibration();
cal.thresholdWarning = cal.baselineAvg + 500;  // Más sensible
cal.thresholdAlarm = cal.baselineAvg + 1000;
smokeSensor->saveCalibration();
```

### Cambiar Tamaño de Promediado

```cpp
// En SmokeSensor.h, cambiar:
static const int SAMPLE_SIZE = 20;  // Más suave (por defecto 10)
```

## 📈 Optimización de Rendimiento

### Lectura Asíncrona

```cpp
// No usar delay() bloqueante
unsigned long lastRead = 0;

void loop() {
    if (millis() - lastRead >= 5000) {
        lastRead = millis();
        SmokeReading reading = smokeSensor->read();
        // Procesar...
    }
    
    // Otras tareas
}
```

### Reducir Consumo de Energía

```cpp
// Leer menos frecuentemente cuando está normal
unsigned long interval = (smokeSensor->isSmokeDetected()) ? 1000 : 10000;
```

## 📚 API Reference Rápida

```cpp
// Inicialización
SmokeSensor* sensor = SmokeSensor::getInstance(pin);
sensor->begin(warmup);
sensor->calibrate(samples, delayMs);

// Lectura
SmokeReading reading = sensor->read();
bool ready = sensor->isReady();

// Estado
SmokeState state = sensor->getState();
String stateStr = sensor->getStateString();
bool detected = sensor->isSmokeDetected();
bool critical = sensor->isCritical();

// Valores
int ppm = sensor->getPPM();
int percentage = sensor->getPercentage();

// Calibración
SmokeCalibration cal = sensor->getCalibration();
bool loaded = sensor->loadCalibration();
bool saved = sensor->saveCalibration();
sensor->resetCalibration();
```

## 🎯 Checklist de Implementación

- [ ] Conectar sensor a GPIO36 (VP)
- [ ] Verificar alimentación 3.3V
- [ ] Crear archivos SmokeSensor.h/cpp en src/sensors/
- [ ] Actualizar main.cpp con ejemplo
- [ ] Compilar y verificar sin errores
- [ ] Subir código al ESP32
- [ ] Esperar warmup (60 seg)
- [ ] Calibrar en aire limpio
- [ ] Probar con humo (vela, incienso)
- [ ] Verificar alertas funcionan
- [ ] Guardar calibración

---

**¿Problemas?** Revisa la sección Troubleshooting o verifica las conexiones físicas.