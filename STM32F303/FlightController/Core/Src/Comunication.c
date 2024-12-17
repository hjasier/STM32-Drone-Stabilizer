#include "Comunication.h"
#include <stdio.h>

extern UART_HandleTypeDef huart1;

void UART_Init(void) {
    HAL_UART_Init(&huart1); // ya se hace en el main así que creo que no hace falta
}

void sendAngles() {
	struct girodata_t giro;
	Sensor_Read(&giro);

	float rateRoll = giro.gx * 70.0 / 1000.0;
	float ratePitch = giro.gy * 70.0 / 1000.0;
	float rateYaw = giro.gz * 70.0 / 1000.0;


    char data_msg[128];
    snprintf(data_msg, sizeof(data_msg), "Roll: %f;Pitch: %f;Yaw: %f\n", rateRoll, ratePitch, rateYaw);

    HAL_UART_Transmit(&huart1, (uint8_t*)data_msg, strlen(data_msg), HAL_MAX_DELAY);


}



// Función para enviar datos de los sensores al ESP8266
void sendSensorData(void) {
    struct girodata_t giro;


    Sensor_Read(&giro);

    char data_msg[128];

    snprintf(data_msg, sizeof(data_msg), "AX: %i, AY: %i, AZ: %i, GX: %i, GY: %i, GZ: %i, MX: %i, MY: %i, MZ: %i\n", giro.ax, giro.ay, giro.az, giro.gx, giro.gy, giro.gz, giro.mx, giro.my, giro.mz);

    //printf(data_msg);
    // Enviar los datos a través de UART
    HAL_UART_Transmit(&huart1, (uint8_t*)data_msg, strlen(data_msg), HAL_MAX_DELAY);

    char speeds[125];
    Control_GetMotorSpeeds(speeds, sizeof(speeds));

    printf("Motor speeds: %s\n", speeds);

    HAL_UART_Transmit(&huart1, (uint8_t*)speeds, strlen(speeds), HAL_MAX_DELAY);

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
