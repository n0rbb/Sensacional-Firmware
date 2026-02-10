# SENSACIONAL
Este repositorio contiene el código cargado en Sensacional 2 para las pruebas de verificación. Contiene la BSP desarrollada para facilitar la comunicación con los sensores 
integrados en Sensacional 2 además de un programa de prueba. 

MCU: ESP32 (ESP32 WROOM SoC)


Sensores soportados:
  * BME280 
  * SGP41 
  * MiCS-6814 (MCP3204)
  * FLMG (MCP3204)
  * Ondas de espín
---

## 📦 Contenido del repositorio

```
.
├── board/
│   ├── include/        
│   │   └── esp32_sensacional.h   # Headers de la BSP
│   ├──                           # Implementación de drivers de bajo nivel
│   └── esp32_sensacional.c       # Configuración de la aplicación demo
│
├── main/
│   ├── include/         
│   │   └── mqtt.h       # Headers para MQTT
│   ├── mqtt.c           # Funciones de soporte para MQTT
│   └── main.c           # Programa de demostración 
│  
└── components/ 
    ├── Wrapper_BME280/ #Implementación de funciones de bajo nivel de BME280 para ESP32
    │   ├── include/         
    │   │   └── wrapper_bme280.h       
    │   └── wrapper_bme280.c       
    ├── Wrapper_SGP41/  #Implementación de funciones de bajo nivel de SGP41 para ESP32
    │   └── sensirion_i2c_hal_esp32.c  
    ├── Sensor_BME280/  #Submódulo con el repositorio de los drivers de BME280
    └── Sensor_SGP41/   #Submódulo con el repositorio de los drivers de SGP41

```

---

## 🧩 BSP (Board Support Package)

La BSP abstrae el hardware de la PCB y proporciona una interfaz para:

- Manejo de GPIOs
- Drivers básicos (UART, I2C, SPI, etc.)
- Comunicación con los diferentes sensores integrados en la placa

---

## 📝 Notas

Este repositorio está pensado como **punto de partida** para desarrollo y validación de plataformas de la familia SENSACIONAL. No pretende ser un framework completo, sino una base sólida y comprensible.

