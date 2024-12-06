#ifndef CONTROL_H
#define CONTROL_H

#include "main.h"
#include "Sensor.h"

// Definir el tipo de datos para las señales de control de los motores
typedef struct {
    int motor1_speed;
    int motor2_speed;
    int motor3_speed;
    int motor4_speed;
} MotorControl;

// Estructura para los parámetros PID
typedef struct {
    float Kp;  // Coeficiente Proporcional
    float Ki;  // Coeficiente Integral
    float Kd;  // Coeficiente Derivativo
    float prev_error;  // Error anterior
    float integral;  // Integral de los errores
} PID;

// Clase Control
typedef struct {
    PID pid_roll;  // PID para el control de roll (inclinación en el eje X)
    PID pid_pitch;  // PID para el control de pitch (inclinación en el eje Y)
    PID pid_yaw;  // PID para el control de yaw (giro en el eje Z)
    MotorControl motor_control;  // Control de los motores

    // Métodos
    void (*init)(void);  // Inicializar PID y otros parámetros
    void (*update)(void);  // Actualizar controles (se llamará periódicamente)
    void (*computeControl)(int ax, int ay, int az, int gx, int gy, int gz, int mx, int my, int mz);  // Calcular la estabilización del dron
    void (*setMotorSpeeds)(void);  // Asignar las velocidades a los motores
    void (*sendMotorCommands)(void);  // Enviar comandos a los motores
} Control;

// Prototipos de las funciones
void Control_Init(void);
void Control_Update(void);
void Control_Compute(int ax, int ay, int az, int gx, int gy, int gz, int mx, int my, int mz);
void Control_SetMotorSpeeds(void);
void Control_SendMotorCommands(void);

#endif /* CONTROL_H */
