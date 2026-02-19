# 💨 Guía de Uso - Sensor de Metano CH4 MEMS (Fermion)

## ⚠️ ADVERTENCIA DE SEGURIDAD

**METANO (CH4) ES UN GAS INFLAMABLE Y EXPLOSIVO**

- 🔥 LEL (Lower Explosive Limit) = **5%** de CH4 en aire
- 💥 UEL (Upper Explosive Limit) = **15%** de CH4 en aire
- ⚡ Puede explotar con chispa, llama o alta temperatura
- 🚫 NO usar llamas cerca del sensor
- 🌬️ Requiere ventilación adecuada
- 🏭 Para uso industrial/residencial de monitoreo solamente

## 📋 Especificaciones del Sensor

**Sensor:** Fermion MEMS CH4 Gas Sensor  
**Gas detectado:** Metano (CH4) - Gas natural  
**Tipo de salida:** Analógica (0-3.3V)  
**Alimentación:** 3.3V  
**Tiempo de calentamiento:** **180 segundos (3 minutos)** ⚠️ MÁS LARGO  
**Rango de detección:** 0 - 10,000+ PPM  
**LEL del CH4:** 50,000 PPM (5% en aire)  

## 🔌 Conexión Hardware

### Pines del Sensor

```
Fermion MEMS CH4:
┌──────────┐
│ A   VCC  GND │
└──────────┘
  │    │    │
  │    │    └──→ GND ESP32
  │    └───────→ 3.3V ESP32
  └────────────→ GPIO39 (VN) ESP32
```

### Diagrama de Conexión

```
ESP32 DevKit           Fermion MEMS CH4
┌─────────────┐        ┌──────────────┐
│             │        │              │
│  3.3V   ────┼────────┼──→ VCC       │
│             │        │              │
│  GND    ────┼────────┼──→ GND       │
│             │        │              │
│  GPIO39 ←───┼────────┼──→ A         │
│  (VN)       │        │              │
└─────────────┘        └──────────────┘
```

**⚠️ CRÍTICO:**
- Usar GPIO39 (VN) - ADC1 compatible con WiFi
- **NO compartir pin** con sensor de humo

## 🚀 Instalación del Módulo

### 1. Crear Archivos

```bash
# Ya debes tener la carpeta
cd src/sensors

# Crear archivos CH4
touch CH4Sensor.h
touch CH4Sensor.cpp
```

### 2. Copiar Código

Copia el contenido de los artifacts:
- `CH4Sensor.h` → `src/sensors/CH4Sensor.h`
- `CH4Sensor.cpp` → `src/sensors/CH4Sensor.cpp`

### 3. Config.h

Ya está actualizado con:
```cpp
#define CH4_SENSOR_PIN 39  // GPIO39 (VN)
```

## 📝 Uso Básico

### Inicialización

```cpp
#include "sensors/CH4Sensor.h"

CH4Sensor* ch4Sensor;

void setup() {
    Serial.begin(115200);
    
    // Inicializar sensor (con warmup de 3 min)
    ch4Sensor = CH4Sensor::getInstance(CH4_SENSOR_PIN);
    ch4Sensor->begin(true);
    
    Serial.println("⏳ Esperando 3 minutos de calentamiento...");
    
    while (!ch4Sensor->isReady()) {
        delay(1000);
        Serial.print(".");
    }
    
    Serial.println("\n✓ Sensor listo!");
}

void loop() {
    static unsigned long lastRead = 0;
    if (millis() - lastRead >= 5000) {
        lastRead = millis();
        
        CH4Reading reading = ch4Sensor->read();
        
        Serial.printf("Estado: %s\n", ch4Sensor->getStateString().c_str());
        Serial.printf("PPM: %d\n", reading.ppm);
        Serial.printf("LEL: %.2f%%\n", reading.lel);
        
        if (ch4Sensor->isExplosive()) {
            Serial.println("💥 ¡NIVEL EXPLOSIVO! ¡EVACUAR!");
        }
    }
}
```

## 🎯 Calibración

### ⚠️ REQUISITOS CRÍTICOS:

1. **Ambiente EXTERIOR o BIEN VENTILADO**
2. **Aire COMPLETAMENTE LIMPIO** (sin fugas de gas)
3. **NO encender llamas, cigarrillos, chispas**
4. **Sensor CALENTADO** (3 minutos completos)
5. **Ventanas/puertas ABIERTAS**

### Proceso de Calibración

```cpp
void calibrateCH4() {
    Serial.println("╔════════════════════════════════════╗");
    Serial.println("║    CALIBRACIÓN SENSOR CH4          ║");
    Serial.println("╚════════════════════════════════════╝");
    Serial.println();
    Serial.println("⚠️  VERIFICAR ANTES DE CONTINUAR:");
    Serial.println("   [✓] Ventanas y puertas abiertas");
    Serial.println("   [✓] NO hay fugas de gas");
    Serial.println("   [✓] NO hay llamas cerca");
    Serial.println("   [✓] Sensor calentado (3 min)");
    Serial.println("   [✓] Ambiente exterior o ventilado");
    Serial.println();
    Serial.println("Presiona 'y' para continuar o 'n' para cancelar:");
    
    while (!Serial.available()) delay(100);
    char confirm = Serial.read();
    
    if (confirm != 'y' && confirm != 'Y') {
        Serial.println("❌ Calibración cancelada");
        return;
    }
    
    Serial.println("\n✓ Iniciando calibración en 5 segundos...");
    delay(5000);
    
    // Calibración: 300 muestras, 1 segundo = 5 minutos
    ch4Sensor->calibrate(300, 1000);
    
    Serial.println("✓ Calibración completada y guardada");
}
```

## 📊 Interpretación de Lecturas

### Estructura CH4Reading

```cpp
struct CH4Reading {
    int rawValue;        // Valor ADC (0-4095)
    float voltage;       // Voltaje (0-3.3V)
    int percentage;      // Porcentaje 0-100%
    int ppm;            // Partes por millón
    float lel;          // % de LEL (Lower Explosive Limit)
    CH4State state;     // Estado actual
    unsigned long timestamp;
};
```

### Estados del Sensor

| Estado | LEL | PPM | Descripción | Acción |
|--------|-----|-----|-------------|--------|
| `INITIALIZING` | - | - | Calentando | Esperar 3 min |
| `NORMAL` | < 0.5% | < 250 | Aire limpio | Monitoreo normal |
| `DETECTED` | 0.5-2% | 250-1000 | Gas presente | Investigar fuente |
| `CRITICAL` | 2-4% | 1000-2000 | Nivel peligroso | Ventilar inmediatamente |
| `EXPLOSIVE` | > 5% | > 2500 | **EXPLOSIVO** | **¡EVACUAR!** |
| `ERROR` | - | - | Fallo sensor | Verificar conexión |

### Niveles de LEL Explicados

**LEL (Lower Explosive Limit) = Límite Inferior de Explosividad**

```
0%      - Sin CH4 (aire limpio)
0.5%    - Umbral de precaución
1%      - Nivel detectable
2%      - Advertencia - Ventilar
4%      - Crítico - Evacuar área
5%      - EXPLOSIVO - Riesgo de explosión
15%     - UEL (Upper Explosive Limit)
100%    - CH4 puro (no explosivo, sofocante)
```

**Cálculo:**
```
LEL% = (PPM / 50,000) × 100

Ejemplo:
2,500 PPM = (2,500 / 50,000) × 100 = 5% LEL
```

## 🔥 Diferencias con SmokeSensor

| Característica | SmokeSensor | CH4Sensor |
|----------------|-------------|-----------|
| Warmup | 60 seg | **180 seg (3 min)** |
| Muestras | 10 | **15** (más ruido) |
| Estados | 5 | **6** (+ EXPLOSIVE) |
| Umbral crítico | Humo denso | **5% LEL explosivo** |
| Peligro | Incendio | **Explosión** |
| Ventilación | Recomendada | **OBLIGATORIA** |

## 🔍 Ejemplos de Uso

### Ejemplo 1: Sistema de Alarma Completo

```cpp
void checkCH4AndAlert() {
    CH4Reading reading = ch4Sensor->read();
    
    if (ch4Sensor->isExplosive()) {
        // MÁXIMA PRIORIDAD
        activateMaxAlarm();
        sendEmergencyNotification("EXPLOSIVO");
        cutGasSupply();  // Si tienes válvula automática
        Serial.println("💥 EVACUAR INMEDIATAMENTE");
    }
    else if (ch4Sensor->isCritical()) {
        activateAlarm();
        openVentilation();  // Si tienes ventilación automática
        sendAlert("CRÍTICO");
        Serial.println("🚨 Ventilar y evacuar");
    }
    else if (ch4Sensor->isCH4Detected()) {
        playWarningBeep();
        sendNotification("CH4 detectado");
        Serial.println("⚠️ Investigar fuente");
    }
}
```

### Ejemplo 2: Logging para Análisis

```cpp
void logCH4Data() {
    CH4Reading reading = ch4Sensor->read();
    
    String logEntry = String(millis()) + "," +
                     String(reading.ppm) + "," +
                     String(reading.lel, 2) + "," +
                     ch4Sensor->getStateString() + "\n";
    
    FileManager::getInstance()->appendFile("/ch4_log.csv", logEntry);
    
    // También enviar a servidor si hay WiFi
    if (WiFi.status() == WL_CONNECTED) {
        sendToServer(reading);
    }
}
```

### Ejemplo 3: Tendencia y Predicción

```cpp
class CH4TrendMonitor {
private:
    static const int HISTORY_SIZE = 10;
    int history[HISTORY_SIZE];
    int index = 0;
    
public:
    void addReading(int ppm) {
        history[index] = ppm;
        index = (index + 1) % HISTORY_SIZE;
    }
    
    bool isRising() {
        // Verificar si los últimos 5 valores están subiendo
        for (int i = 1; i < 5; i++) {
            int prev = history[(index - i - 1 + HISTORY_SIZE) % HISTORY_SIZE];
            int curr = history[(index - i + HISTORY_SIZE) % HISTORY_SIZE];
            if (curr <= prev) return false;
        }
        return true;
    }
    
    void checkTrend() {
        if (isRising()) {
            Serial.println("📈 TENDENCIA CRECIENTE - Posible fuga");
            // Activar pre-alarma
        }
    }
};
```

### Ejemplo 4: Integración MQTT

```cpp
void publishCH4ToMQTT() {
    CH4Reading reading = ch4Sensor->read();
    
    String topic = "home/safety/ch4";
    String payload = "{";
    payload += "\"ppm\":" + String(reading.ppm) + ",";
    payload += "\"lel\":" + String(reading.lel, 2) + ",";
    payload += "\"state\":\"" + ch4Sensor->getStateString() + "\"";
    payload += "}";
    
    mqttClient.publish(topic.c_str(), payload.c_str());
    
    // Para Home Assistant
    if (ch4Sensor->isExplosive()) {
        mqttClient.publish("home/alarm/gas", "EXPLOSIVE");
    }
}
```

## 🐛 Troubleshooting

### Problema: Lecturas siempre altas

**Causa:** Cerca de fuente de gas (estufa, calentador)

**Solución:**
1. Alejar de cocina/calentadores
2. Ventilar el área completamente
3. Recalibrar en aire limpio exterior
4. Verificar no hay fugas reales

### Problema: Sensor tarda mucho en inicializar

**Causa:** Normal - CH4 requiere 3 minutos

**Solución:**
```cpp
// Mostrar progreso
while (!ch4Sensor->isReady()) {
    int elapsed = (millis() - startTime) / 1000;
    int remaining = 180 - elapsed;
    Serial.printf("Calentando... %d seg restantes\n", remaining);
    delay(5000);
}
```

### Problema: Estado EXPLOSIVE en aire limpio

**Causa:** Sensor no calibrado o defectuoso

**Solución:**
1. Verificar conexión física
2. Recalibrar en aire exterior
3. Verificar umbral: `getCalibration()`
4. Si persiste, sensor defectuoso

### Problema: No detecta gas conocido

**Causa:** 
- Sensor no calentado
- Concentración muy baja
- Sensor defectuoso

**Solución:**
```cpp
// Test con lectura cruda
int raw = analogRead(CH4_SENSOR_PIN);
Serial.printf("Raw: %d\n", raw);

// Si raw no cambia con gas presente, verificar:
// - Conexión del pin A
// - Alimentación 3.3V
// - Reemplazar sensor
```

## ⚙️ Configuración Avanzada

### Ajustar Sensibilidad de Umbrales

```cpp
// Después de calibrar
CH4Calibration cal = ch4Sensor->getCalibration();

// Más conservador (alertar antes)
cal.thresholdCaution = cal.baselineAvg + 300;
cal.thresholdWarning = cal.baselineAvg + 700;
cal.thresholdAlarm = cal.baselineAvg + 1200;

// Guardar cambios
ch4Sensor->saveCalibration();
```

### Cambiar Tiempo de Warmup

```cpp
// En CH4Sensor.h (NO RECOMENDADO, mínimo 3 min)
static const unsigned long WARMUP_TIME = 180000;
```

### Aumentar Promediado (Reduce Ruido)

```cpp
// En CH4Sensor.h
static const int SAMPLE_SIZE = 20;  // Por defecto 15
```

## 🏭 Aplicaciones Reales

### 1. Hogar/Cocina
- Detectar fugas de gas natural/LP
- Prevenir explosiones
- Alerta temprana

### 2. Industrial
- Monitoreo de líneas de gas
- Plantas de tratamiento
- Refinerías

### 3. Minería
- Detectar grisú (metano en minas)
- Seguridad de mineros

### 4. Agricultura
- Biodigestores
- Granjas (descomposición orgánica)

## 📈 Calibración Periódica

**Recomendación:**
- Primera vez: Inmediatamente
- Producción: Cada 3 meses
- Post-incidente: Inmediatamente
- Cambio ubicación: Inmediatamente

## 🔐 Integración con Sistema de Seguridad

```cpp
// Sistema completo
if (ch4Sensor->isExplosive() || smokeSensor->isCritical()) {
    // 1. Cortar electricidad (evitar chispas)
    cutPower();
    
    // 2. Cerrar válvula de gas
    closeGasValve();
    
    // 3. Activar ventilación
    openWindows();
    activateFans();
    
    // 4. Notificar
    sendEmergencyCall();
    sendSMS("EMERGENCIA: Gas/Humo detectado");
    
    // 5. Alarma visual/sonora
    activateMaxAlarm();
}
```

## 📚 API Reference Rápida

```cpp
// Inicialización
CH4Sensor* sensor = CH4Sensor::getInstance(pin);
sensor->begin(warmup);
sensor->calibrate(samples, delayMs);

// Lectura
CH4Reading reading = sensor->read();
bool ready = sensor->isReady();

// Estado
CH4State state = sensor->getState();
String stateStr = sensor->getStateString();
bool detected = sensor->isCH4Detected();
bool critical = sensor->isCritical();
bool explosive = sensor->isExplosive();

// Valores
int ppm = sensor->getPPM();
float lel = sensor->getLEL();
int percentage = sensor->getPercentage();

// Calibración
CH4Calibration cal = sensor->getCalibration();
sensor->loadCalibration();
sensor->saveCalibration();
sensor->resetCalibration();
```

## ✅ Checklist de Implementación

- [ ] Conectar sensor a GPIO39 (VN)
- [ ] Verificar alimentación 3.3V
- [ ] Crear archivos CH4Sensor.h/cpp
- [ ] Actualizar main.cpp
- [ ] Compilar sin errores
- [ ] Subir al ESP32
- [ ] Esperar warmup completo (3 min)
- [ ] Calibrar en EXTERIOR/ventilado
- [ ] Probar con fuente de gas controlada
- [ ] Verificar umbrales funcionan
- [ ] Configurar alertas/notificaciones
- [ ] Documentar ubicación de válvula de gas

---

**⚠️ RECORDATORIO FINAL:** Este sensor es para MONITOREO solamente. No reemplaza detectores certificados de gas ni sistemas profesionales de seguridad. Siempre mantén detectores certificados como respaldo.