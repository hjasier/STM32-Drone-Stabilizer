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
void receiveControlCommand(uint8_t *rx_buffer, uint16_t length) {

	if (strncmp((char *)rx_buffer, "Dat", 3) == 0 || strncmp((char *)rx_buffer, "Men", 3) == 0) {
	    memset(rx_buffer, 0, length);  // Limpiar el buffer
	    return;
	}

    printData("Comando recibido\n");

    if (strncmp((char *)rx_buffer, "LED_ON", 6) == 0) {
        // Encender el LED (PB3)
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
    } else if (strncmp((char *)rx_buffer, "LED_OFF", 7) == 0) {
        // Apagar el LED (PB3)
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
    }

    else if (strncmp((char*) rx_buffer, "ARM", 3) == 0) {
		Control_ArmMotors();
	} else if (strncmp((char*) rx_buffer, "STOP", 4) == 0) {
		Control_Stop();
	} else if (strncmp((char*) rx_buffer, "PWR", 3) == 0) {
		uint8_t speed = atoi((char*) &rx_buffer[3]);
		Control_SetMotorsPower(speed);
	}


    // Limpiar el buffer después de procesar
    memset(rx_buffer, 0, length);
}
