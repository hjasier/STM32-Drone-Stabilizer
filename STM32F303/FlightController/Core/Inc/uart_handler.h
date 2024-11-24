#ifndef UART_HANDLER_H
#define UART_HANDLER_H

#include "main.h"

/* Declaración de la función para recibir datos por UART */
void uart_receive_data(UART_HandleTypeDef *huart, uint8_t *buffer, uint16_t length);

#endif /* UART_HANDLER_H */
