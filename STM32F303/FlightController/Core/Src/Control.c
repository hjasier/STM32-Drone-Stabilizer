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
    // Filtro complementario para roll y pitch
    float roll_acc = atan2(giro->ay, giro->az) * 180.0 / M_PI;
    float pitch_acc = atan2(-giro->ax, sqrt(giro->ay * giro->ay + giro->az * giro->az)) * 180.0 / M_PI;

    roll_filtered = ALPHA * (roll_filtered + giro->gx * 0.01) + (1 - ALPHA) * roll_acc;
    pitch_filtered = ALPHA * (pitch_filtered + giro->gy * 0.01) + (1 - ALPHA) * pitch_acc;

    // Filtro de promedio móvil para yaw (usando magnetómetro)
    mx_buffer[buffer_index] = giro->mx;
    my_buffer[buffer_index] = giro->my;
    mz_buffer[buffer_index] = giro->mz;
    buffer_index = (buffer_index + 1) % MOVING_AVG_SIZE;

    float mx_avg = 0, my_avg = 0, mz_avg = 0;
    for (int i = 0; i < MOVING_AVG_SIZE; i++) {
        mx_avg += mx_buffer[i];
        my_avg += my_buffer[i];
        mz_avg += mz_buffer[i];
    }
    mx_avg /= MOVING_AVG_SIZE;
    my_avg /= MOVING_AVG_SIZE;
    mz_avg /= MOVING_AVG_SIZE;

    yaw_filtered = atan2(my_avg, mx_avg) * 180.0 / M_PI;

    // Error de cada eje
    float roll_error = 0 - roll_filtered;
    float pitch_error = 0 - pitch_filtered;
    float yaw_error = 0 - yaw_filtered;

    printf("Roll: %d, Pitch: %d, Yaw: %d\n", roll_filtered, pitch_filtered, yaw_filtered);

    Control.motor_control.motor1_speed = pidCompute(&Control.pid_roll, roll_error);
    Control.motor_control.motor2_speed = pidCompute(&Control.pid_pitch, pitch_error);
    Control.motor_control.motor3_speed = pidCompute(&Control.pid_yaw, yaw_error);
    Control.motor_control.motor4_speed = pidCompute(&Control.pid_roll, roll_error);

    printf("Motor 1 speed: %d\n", Control.motor_control.motor1_speed);
    printf("Motor 2 speed: %d\n", Control.motor_control.motor2_speed);
    printf("Motor 3 speed: %d\n", Control.motor_control.motor3_speed);

}

float pidCompute(PID* pid, float error) {
    pid->integral += error;
    float derivative = error - pid->prev_error;

    float output = pid->Kp * error + pid->Ki * pid->integral + pid->Kd * derivative;

    pid->prev_error = error;

    if (output > MOTOR_MAX_SPEED) output = MOTOR_MAX_SPEED;
    if (output < MOTOR_MIN_SPEED) output = MOTOR_MIN_SPEED;

    return output;
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
