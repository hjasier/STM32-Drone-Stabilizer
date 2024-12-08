#include "uart_handler.h"
#include <string.h>

void uart_receive_data(UART_HandleTypeDef *huart, uint8_t *buffer, uint16_t length) {
    // Recibir datos por UART
    HAL_UART_Receive(huart, buffer, length, HAL_MAX_DELAY);
    // Limpiar el búfer
    memset(buffer, 0, length);
}
