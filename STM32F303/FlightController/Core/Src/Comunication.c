#include "Comunication.h"
#include <stdio.h>

extern UART_HandleTypeDef huart1;

void UART_Init(void) {
    HAL_UART_Init(&huart1); // ya se hace en el main así que creo que no hace falta
}

// Función para enviar datos de los sensores al ESP8266
void sendSensorData(void) {
    char data_msg[128];

    int16_t ax, ay, az;
    int16_t gx, gy, gz;
    int16_t mx, my, mz;

    ADXL345_ReadData(&ax, &ay, &az);  // Leer datos acelerómetro
    ITG3205_ReadData(&gx, &gy, &gz);  // Leer datos giroscopio
    HMC5883L_ReadData(&mx, &my, &mz);  // Leer datos magnetómetro

    snprintf(data_msg, sizeof(data_msg), "ACCX:%d ACCY:%d ACCZ:%d GYX:%d GYY:%d GYZ:%d MAGX:%d MAGY:%d MAGZ:%d\r\n",
             ax, ay, az, gx, gy, gz, mx, my, mz);

    // Enviar los datos a través de UART
    HAL_UART_Transmit(&huart1, (uint8_t*)data_msg, strlen(data_msg), HAL_MAX_DELAY);
}

// Función para enviar un comando de control al ESP8266 (ejemplo: "start", "stop", "set_speed 100")
void sendControlCommand(const char* command) {
    char cmd_msg[64];
    snprintf(cmd_msg, sizeof(cmd_msg), "%s\r\n", command);  // Agregar salto de línea al final del comando
    HAL_UART_Transmit(&huart1, (uint8_t*)cmd_msg, strlen(cmd_msg), HAL_MAX_DELAY);
}

// Función para recibir comandos del ESP8266
void receiveControlCommand(void) {
    uint8_t rx_buffer[64];  // Buffer para almacenar el comando recibido
    HAL_StatusTypeDef status = HAL_UART_Receive(&huart1, rx_buffer, sizeof(rx_buffer) - 1, 1000);  // Timeout 1 segundo

    if (status == HAL_OK) {
        rx_buffer[sizeof(rx_buffer) - 1] = '\0';  // Asegurarse de que la cadena esté terminada en nulo

        // Aquí procesas el comando recibido. Ejemplo:
        if (strncmp((char*)rx_buffer, "stop", 4) == 0) {
            // Detener los motores
            sendControlCommand("Motor stopped");
        } else if (strncmp((char*)rx_buffer, "start", 5) == 0) {
            // Iniciar motores a velocidad 100
            sendControlCommand("Motor started at speed 100");
        } else if (strncmp((char*)rx_buffer, "set_speed", 9) == 0) {
            // Extraer la velocidad del comando
            int speed = atoi((char*)rx_buffer + 10);  // Asumimos que el valor de velocidad viene después de "set_speed "
            sendControlCommand("Speed set");
        }
    }
}
