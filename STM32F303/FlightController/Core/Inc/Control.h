#ifndef CONTROL_H
#define CONTROL_H

#include "main.h"
#include "Sensor.h"

// Estructura PID
typedef struct {
    float Kp;
    float Ki;
    float Kd;
    float prev_error;
    float integral;
} PID;

// Estructura para controlar los motores
typedef struct {
    PID pid_roll;
    PID pid_pitch;
    PID pid_yaw;

    struct {
        int motor1_speed;
        int motor2_speed;
        int motor3_speed;
        int motor4_speed;
    } motor_control;

} Control_t;

extern Control_t Control;

void Control_Init(void);
void Control_Update(void);
void Control_Compute(int ax, int ay, int az, int gx, int gy, int gz, int mx, int my, int mz);
void Control_SetMotorSpeeds(void);
void Control_SendMotorCommands(void);
void Control_SetMotorsPower(int base_power_percentage);
float pidCompute(PID* pid, float error);

#endif // CONTROL_H
