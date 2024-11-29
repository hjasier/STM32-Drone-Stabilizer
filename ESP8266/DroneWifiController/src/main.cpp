#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include "config.h"

WebSocketsServer webSocket = WebSocketsServer(81);

unsigned long lastUpdate = 0;  
int number = 0;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void processMessage(String message);

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
      Serial.println("Mensaje recibido por Serial: " + serialMessage);
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
