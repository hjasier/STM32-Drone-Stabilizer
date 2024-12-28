#include <Arduino.h>
#include <WiFi.h>
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
  Serial.println("Esperando Datos por UART:");
}

void loop() {
  webSocket.loop();

  // Leer mensajes del puerto serie
  if (Serial.available() > 0) { // Comprobar si hay datos disponibles en el puerto serie
    String serialMessage = Serial.readStringUntil('\n');  // Cambié el delimitador a '\n'
    serialMessage.trim(); // Eliminar espacios en blanco iniciales y finales
    if (serialMessage.length() > 0) {
      //Serial.println("Mensaje recibido por Serial: " + serialMessage);
      webSocket.broadcastTXT(serialMessage);
    }
  }

  
  // // Enviar un número incremental o aleatorio cada 2 segundos
  // unsigned long currentMillis = millis();
  // if (currentMillis - lastUpdate >= 2000) { 
  //   number++;  

  //   String numberStr = "Valor: " + String(number);

  //   // Enviar el número a todos los clientes conectados
  //   webSocket.broadcastTXT((uint8_t*)numberStr.c_str(), numberStr.length());
  //   Serial.println("Datos enviados: " + numberStr);  // Imprime el número en el puerto serial

  //   lastUpdate = currentMillis;
  // }
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


int extractValue(String message, String key) {
    int startIdx = message.indexOf(key);
    if (startIdx != -1) {
        int endIdx = message.indexOf(" ", startIdx);
        if (endIdx == -1) {
            endIdx = message.indexOf("\r\n", startIdx);
            if (endIdx == -1) {
                endIdx = message.length();
            }
        }
        return message.substring(startIdx + key.length(), endIdx).toInt();
    }
    return 0;
}

void processSerialMessage(String message) {
    // Variables para almacenar los datos extraídos
    int16_t GYX = 0, GYY = 0, GYZ = 0;
    int16_t MAGX = 0, MAGY = 0, MAGZ = 0;
    int16_t ACX = 0, ACY = 0, ACZ = 0;

    // GY85 Data
    GYX = extractValue(message, "Roll:");
    GYY = extractValue(message, "Pitch:");
    GYZ = extractValue(message, "Yaw:");

    Serial.println("Roll: " + String(GYX) + " Pitch: " + String(GYY) + " Yaw: " + String(GYZ));
    webSocket.broadcastTXT(message.c_str(), message.length());
}