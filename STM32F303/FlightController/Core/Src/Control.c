#include "Control.h"
#include "Sensor.h"
#include <math.h>
#include <stdio.h>

#define MOTOR_MAX_SPEED 1000  // Velocidad máxima del motor
#define MOTOR_MIN_SPEED 0    // Velocidad mínima del motor

// Variables globales para almacenar los datos de los sensores
extern int16_t ax, ay, az;
extern int16_t gx, gy, gz;
extern int16_t mx, my, mz;

// Inicialización de la clase Control
void Control_Init(void) {
    // Inicializar PID con valores de ejemplo (ajustar según sea necesario)
    Control.pid_roll.Kp = 1.0;
    Control.pid_roll.Ki = 0.0;
    Control.pid_roll.Kd = 0.0;
    Control.pid_roll.prev_error = 0.0;
    Control.pid_roll.integral = 0.0;

    Control.pid_pitch.Kp = 1.0;
    Control.pid_pitch.Ki = 0.0;
    Control.pid_pitch.Kd = 0.0;
    Control.pid_pitch.prev_error = 0.0;
    Control.pid_pitch.integral = 0.0;

    Control.pid_yaw.Kp = 1.0;
    Control.pid_yaw.Ki = 0.0;
    Control.pid_yaw.Kd = 0.0;
    Control.pid_yaw.prev_error = 0.0;
    Control.pid_yaw.integral = 0.0;
}

// Actualizar el control, este método se llama periódicamente
void Control_Update(void) {
    // Calcular el control con los datos de los sensores
    Control_Compute(ax, ay, az, gx, gy, gz, mx, my, mz);

    // Asignar las velocidades a los motores en función de los cálculos
    Control_SetMotorSpeeds();

    // Enviar los comandos a los motores (por UART o GPIO)
    Control_SendMotorCommands();
}

// Calcular la estabilización usando los datos de los sensores (acelerómetro, giroscopio, magnetómetro)
void Control_Compute(int ax, int ay, int az, int gx, int gy, int gz, int mx, int my, int mz) {
    // Calcular los ángulos roll, pitch, yaw usando los sensores (esto es un ejemplo, puedes usar un filtro complementario o un algoritmo de fusión de sensores)
    float roll = atan2(ay, az) * 180.0 / M_PI;
    float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / M_PI;
    float yaw = atan2(my, mx) * 180.0 / M_PI;

    // Calculamos el error en cada eje
    float roll_error = 0 - roll;  // Queremos que el roll sea 0
    float pitch_error = 0 - pitch;  // Queremos que el pitch sea 0
    float yaw_error = 0 - yaw;  // Queremos que el yaw sea 0

    // Aplicamos el control PID para cada eje (roll, pitch, yaw)
    Control.motor_control.motor1_speed = pidCompute(&Control.pid_roll, roll_error);
    Control.motor_control.motor2_speed = pidCompute(&Control.pid_pitch, pitch_error);
    Control.motor_control.motor3_speed = pidCompute(&Control.pid_yaw, yaw_error);
}

// Método PID para calcular el control basado en el error
float pidCompute(PID* pid, float error) {
    pid->integral += error;
    float derivative = error - pid->prev_error;

    float output = pid->Kp * error + pid->Ki * pid->integral + pid->Kd * derivative;

    pid->prev_error = error;

    // Limitar el valor de salida entre el rango de velocidad de los motores
    if (output > MOTOR_MAX_SPEED) output = MOTOR_MAX_SPEED;
    if (output < MOTOR_MIN_SPEED) output = MOTOR_MIN_SPEED;

    return output;
}

// Asignar las velocidades de los motores basados en el PID
void Control_SetMotorSpeeds(void) {
    // Ajustar las velocidades de los motores según el cálculo de PID
    // Ejemplo: motor1_speed para el control de roll, motor2_speed para el control de pitch, etc.
    // En un sistema real, estos valores deben combinarse adecuadamente para el control de los 4 motores.
}

// Enviar los comandos de los motores (esto se hará mediante UART o GPIO)
void Control_SendMotorCommands(void) {
    // Este método enviará las velocidades calculadas a los motores
    // Ejemplo: enviar los valores de las velocidades a través de UART al controlador de los motores
    // O utilizar PWM en los pines de salida del STM32
    printf("Motor 1 speed: %d\n", Control.motor_control.motor1_speed);
    printf("Motor 2 speed: %d\n", Control.motor_control.motor2_speed);
    printf("Motor 3 speed: %d\n", Control.motor_control.motor3_speed);
    printf("Motor 4 speed: %d\n", Control.motor_control.motor4_speed);
}
