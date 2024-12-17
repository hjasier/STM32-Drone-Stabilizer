#ifndef COMUNICATION_H
#define COMUNICATION_H

#include "main.h"

// Inicialización de UART
void UART_Init(void);

// Funciones para enviar datos a ESP8266
void sendSensorData(void);  // Enviar datos de los sensores
void sendControlCommand(const char* command);  // Enviar comandos de control al ESP8266

// Función para recibir comandos del ESP8266
void receiveControlCommand(uint8_t *rx_buffer, uint16_t length);  // Recibir comandos del ESP8266 para controlar los motores

#endif /* COMUNICACION_H */
