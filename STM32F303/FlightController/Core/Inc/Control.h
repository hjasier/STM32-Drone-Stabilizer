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
    int base_throttle;


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
void Control_Compute(struct girodata_t* giro);
void Control_SetMotorSpeeds(void);
void Control_SendMotorCommands(void);
void Control_SetMotorsPower(uint8_t base_power_percentage);
void Control_ArmMotors();
void Control_Stop(void);
float pidCompute(PID* pid, float error);

void Control_GetMotorSpeeds(char* buffer, size_t buffer_size);

#endif // CONTROL_H
