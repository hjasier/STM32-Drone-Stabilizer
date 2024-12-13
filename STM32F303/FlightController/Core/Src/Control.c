#include "Control.h"
#include "Sensor.h"
#include <math.h>
#include <stdio.h>

#define MOTOR_MAX_SPEED 200
#define MOTOR_MIN_SPEED 100

#define ALPHA 0.98  // Coeficiente del filtro complementario
#define MOVING_AVG_SIZE 5

Control_t Control;

extern int16_t ax, ay, az;
extern int16_t gx, gy, gz;
extern int16_t mx, my, mz;

float roll_filtered = 0.0;
float pitch_filtered = 0.0;
float yaw_filtered = 0.0;

float mx_buffer[MOVING_AVG_SIZE] = {0};
float my_buffer[MOVING_AVG_SIZE] = {0};
float mz_buffer[MOVING_AVG_SIZE] = {0};
int buffer_index = 0;



void Control_Init(void) {
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

void Control_ArmMotors() {
	printf("Hadoken");
    Control.motor_control.motor1_speed = MOTOR_MAX_SPEED;
    Control.motor_control.motor2_speed = MOTOR_MAX_SPEED;
    Control.motor_control.motor3_speed = MOTOR_MAX_SPEED;
    Control.motor_control.motor4_speed = MOTOR_MAX_SPEED;
    Control_SetMotorSpeeds();
    HAL_Delay(3000);
    Control.motor_control.motor1_speed = MOTOR_MIN_SPEED;
    Control.motor_control.motor2_speed = MOTOR_MIN_SPEED;
    Control.motor_control.motor3_speed = MOTOR_MIN_SPEED;
    Control.motor_control.motor4_speed = MOTOR_MIN_SPEED;
    Control_SetMotorSpeeds();
    HAL_Delay(3000);

    printData("Motors armed\n");
}

void Control_Update(void) {
	struct girodata_t giro;
	Sensor_Read(&giro);
	printf("AX: %i, AY: %i, AZ: %i, GX: %i, GY: %i, GZ: %i, MX: %i, MY: %i, MZ: %i\n", giro.ax, giro.ay, giro.az, giro.gx, giro.gy, giro.gz, giro.mx, giro.my, giro.mz);
    Control_Compute(&giro);

    Control_SetMotorSpeeds();
}

void Control_Compute(struct girodata_t* giro) {
    float rateRoll = giro->gx / 16.4;
    float ratePitch = giro->gy / 16.4;
    float rateYaw = giro->gz / 16.4;

    float desiredRateRoll = 10.0;
    float desiredRatePitch = 5.0;
    float desiredRateYaw = 0.0;

    // Error
    float errorRoll = desiredRateRoll - rateRoll;
    float errorPitch = desiredRatePitch - ratePitch;
    float errorYaw = desiredRateYaw - rateYaw;

    // PID Roll
    float pTermRoll = Control.pid_roll.Kp * errorRoll;
    Control.pid_roll.integral += errorRoll; // Acumulador integral
    float iTermRoll = Control.pid_roll.Ki * Control.pid_roll.integral;
    float dTermRoll = Control.pid_roll.Kd * (errorRoll - Control.pid_roll.prev_error);
    Control.pid_roll.prev_error = errorRoll;
    float pidOutputRoll = pTermRoll + iTermRoll + dTermRoll;

    // PID Pitch
    float pTermPitch = Control.pid_pitch.Kp * errorPitch;
    Control.pid_pitch.integral += errorPitch;
    float iTermPitch = Control.pid_pitch.Ki * Control.pid_pitch.integral;
    float dTermPitch = Control.pid_pitch.Kd * (errorPitch - Control.pid_pitch.prev_error);
    Control.pid_pitch.prev_error = errorPitch;
    float pidOutputPitch = pTermPitch + iTermPitch + dTermPitch;

    // PID Yaw
    float pTermYaw = Control.pid_yaw.Kp * errorYaw;
    Control.pid_yaw.integral += errorYaw;
    float iTermYaw = Control.pid_yaw.Ki * Control.pid_yaw.integral;
    float dTermYaw = Control.pid_yaw.Kd * (errorYaw - Control.pid_yaw.prev_error);
    Control.pid_yaw.prev_error = errorYaw;
    float pidOutputYaw = pTermYaw + iTermYaw + dTermYaw;


    int baseThrottle = Control.base_throttle;

    Control.motor_control.motor1_speed = baseThrottle + pidOutputPitch - pidOutputRoll - pidOutputYaw;
    Control.motor_control.motor2_speed = baseThrottle + pidOutputPitch + pidOutputRoll + pidOutputYaw;
    Control.motor_control.motor3_speed = baseThrottle - pidOutputPitch + pidOutputRoll - pidOutputYaw;
    Control.motor_control.motor4_speed = baseThrottle - pidOutputPitch - pidOutputRoll + pidOutputYaw;

    Control.motor_control.motor1_speed = constrain(Control.motor_control.motor1_speed, MIN_MOTOR_SPEED, MAX_MOTOR_SPEED);
    Control.motor_control.motor2_speed = constrain(Control.motor_control.motor2_speed, MIN_MOTOR_SPEED, MAX_MOTOR_SPEED);
    Control.motor_control.motor3_speed = constrain(Control.motor_control.motor3_speed, MIN_MOTOR_SPEED, MAX_MOTOR_SPEED);
    Control.motor_control.motor4_speed = constrain(Control.motor_control.motor4_speed, MIN_MOTOR_SPEED, MAX_MOTOR_SPEED);

}




void Control_SetMotorSpeeds(void) {
    int motor1_speed = (Control.motor_control.motor1_speed < 0) ? 0 : (Control.motor_control.motor1_speed > MOTOR_MAX_SPEED ? MOTOR_MAX_SPEED : Control.motor_control.motor1_speed);
    int motor2_speed = (Control.motor_control.motor2_speed < 0) ? 0 : (Control.motor_control.motor2_speed > MOTOR_MAX_SPEED ? MOTOR_MAX_SPEED : Control.motor_control.motor2_speed);
    int motor3_speed = (Control.motor_control.motor3_speed < 0) ? 0 : (Control.motor_control.motor3_speed > MOTOR_MAX_SPEED ? MOTOR_MAX_SPEED : Control.motor_control.motor3_speed);
    int motor4_speed = (Control.motor_control.motor4_speed < 0) ? 0 : (Control.motor_control.motor4_speed > MOTOR_MAX_SPEED ? MOTOR_MAX_SPEED : Control.motor_control.motor4_speed);

    TIM1->CCR1 = motor1_speed;
    TIM1->CCR4 = motor2_speed;
    TIM2->CCR1 = motor3_speed;
    TIM2->CCR2 = motor4_speed;

    printf("Motor Speeds: %d, %d, %d, %d\n", motor1_speed, motor2_speed, motor3_speed, motor4_speed);
}

void Control_SetMotorsPower(uint8_t base_power_percentage) {
    if (base_power_percentage < 0) base_power_percentage = 0;
    if (base_power_percentage > 100) base_power_percentage = 100;

    printf("Base power percentage: %d\n", base_power_percentage);

    int base_power = MOTOR_MIN_SPEED + ((MOTOR_MAX_SPEED - MOTOR_MIN_SPEED) * base_power_percentage) / 100;

    printf("Base power: %d\n", base_power);
    Control.motor_control.motor1_speed = base_power;
    Control.motor_control.motor2_speed = base_power;
    Control.motor_control.motor3_speed = base_power;
    Control.motor_control.motor4_speed = base_power;

}

void Control_SendMotorCommands(void) {
    printf("Motor 1 speed: %d\n", Control.motor_control.motor1_speed);
    printf("Motor 2 speed: %d\n", Control.motor_control.motor2_speed);
    printf("Motor 3 speed: %d\n", Control.motor_control.motor3_speed);
    printf("Motor 4 speed: %d\n", Control.motor_control.motor4_speed);
}

void Control_Stop(void) {
    Control.motor_control.motor1_speed = 0;
    Control.motor_control.motor2_speed = 0;
    Control.motor_control.motor3_speed = 0;
    Control.motor_control.motor4_speed = 0;

    printData("Motors stopped\n");
}
