#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include "config.h"

WebSocketsServer webSocket = WebSocketsServer(81);

unsigned long lastUpdate = 0;  
int number = 0;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void processMessage(String message);
void processSerialMessage(String message);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600); 

  // Inicialización de la conexión Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Conectando a Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(500); 
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);  
  }
  Serial.println("\nConectado a Wi-Fi");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
  
  // Inicializar WebSocket
  webSocket.begin(); 
  webSocket.onEvent(webSocketEvent); 

  Serial.println("Servidor WebSocket iniciado en el puerto 81");
}

void loop() {
  webSocket.loop();

  // Leer mensajes del puerto serie
  if (Serial.available() > 0) { // Comprobar si hay datos disponibles en el puerto serie
    String serialMessage = Serial.readStringUntil('\n'); // Leer hasta el final de la línea
    serialMessage.trim(); // Eliminar espacios en blanco iniciales y finales
    if (serialMessage.length() > 0) {
      processSerialMessage(serialMessage);
      //Serial.println("Mensaje recibido por Serial: " + serialMessage);
    }
  }

  // Enviar un número incremental o aleatorio cada 2 segundos
  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdate >= 2000) { 
    number++;  

    String numberStr = "Valor: " + String(number);

    // Enviar el número a todos los clientes conectados
    webSocket.broadcastTXT((uint8_t*)numberStr.c_str(), numberStr.length());
    Serial.println("Datos enviados: " + numberStr);  // Imprime el número en el puerto serial

    lastUpdate = currentMillis;
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.println("Nuevo cliente conectado");
      break;
    case WStype_DISCONNECTED:
      Serial.println("Cliente desconectado");
      break;
    case WStype_TEXT:
      Serial.println("Mensaje recibido:");
      Serial.println((char*)payload);
      processMessage((char*)payload);
      break;
    case WStype_PING:
      break;
    case WStype_PONG:
      break;
    default:
      Serial.println("Evento WebSocket no manejado.");
      break;
  }
}

void processMessage(String message) {
  if (message == "ON") {
    digitalWrite(LED_BUILTIN, LOW);
  } else if (message == "OFF") {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}


void processSerialMessage(String message) {
    // Variables para almacenar los datos extraídos
    int16_t GYX = 0, GYY = 0, GYZ = 0;
    int16_t MAGX = 0, MAGY = 0, MAGZ = 0;
    int16_t ACX = 0, ACY = 0, ACZ = 0;

    // Buscar y extraer cada dato usando índices
    int startIdx, endIdx;

    // GY85 Data
    startIdx = message.indexOf("GYX:");
    if (startIdx != -1) {
        endIdx = message.indexOf(" ", startIdx);
        GYX = message.substring(startIdx + 4, endIdx).toInt();
    }
    startIdx = message.indexOf("GYY:");
    if (startIdx != -1) {
        endIdx = message.indexOf(" ", startIdx);
        GYY = message.substring(startIdx + 4, endIdx).toInt();
    }
    startIdx = message.indexOf("GYZ:");
    if (startIdx != -1) {
        endIdx = message.indexOf("\r\n", startIdx);
        GYZ = message.substring(startIdx + 4, endIdx).toInt();
    }

    // HMC5883L Data
    startIdx = message.indexOf("MAGX:");
    if (startIdx != -1) {
        endIdx = message.indexOf(" ", startIdx);
        MAGX = message.substring(startIdx + 5, endIdx).toInt();
    }
    startIdx = message.indexOf("MAGY:");
    if (startIdx != -1) {
        endIdx = message.indexOf(" ", startIdx);
        MAGY = message.substring(startIdx + 5, endIdx).toInt();
    }
    startIdx = message.indexOf("MAGZ:");
    if (startIdx != -1) {
        endIdx = message.indexOf("\r\n", startIdx);
        MAGZ = message.substring(startIdx + 5, endIdx).toInt();
    }

    // ADXL345 Data
    startIdx = message.indexOf("ACX:");
    if (startIdx != -1) {
        endIdx = message.indexOf(" ", startIdx);
        ACX = message.substring(startIdx + 4, endIdx).toInt();
    }
    startIdx = message.indexOf("ACY:");
    if (startIdx != -1) {
        endIdx = message.indexOf(" ", startIdx);
        ACY = message.substring(startIdx + 4, endIdx).toInt();
    }
    startIdx = message.indexOf("ACZ:");
    if (startIdx != -1) {
        endIdx = message.indexOf("\r\n", startIdx);
        ACZ = message.substring(startIdx + 4, endIdx).toInt();
    }

    // Imprimir los datos procesados
    Serial.println("Datos Procesados:");
    Serial.print("GYX: "); Serial.println(GYX);
    Serial.print("GYY: "); Serial.println(GYY);
    Serial.print("GYZ: "); Serial.println(GYZ);
    Serial.print("MAGX: "); Serial.println(MAGX);
    Serial.print("MAGY: "); Serial.println(MAGY);
    Serial.print("MAGZ: "); Serial.println(MAGZ);
    Serial.print("ACX: "); Serial.println(ACX);
    Serial.print("ACY: "); Serial.println(ACY);
    Serial.print("ACZ: "); Serial.println(ACZ);
}
