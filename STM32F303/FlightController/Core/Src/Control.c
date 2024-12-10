#include "Control.h"
#include "Sensor.h"
#include <math.h>
#include <stdio.h>

#define MOTOR_MAX_SPEED 200
#define MOTOR_MIN_SPEED 100

Control_t Control;

extern int16_t ax, ay, az;
extern int16_t gx, gy, gz;
extern int16_t mx, my, mz;


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


void Control_ArmMotors(){
	Control.motor_control.motor1_speed = MOTOR_MAX_SPEED;
	Control.motor_control.motor2_speed = MOTOR_MAX_SPEED;
	Control.motor_control.motor3_speed = MOTOR_MAX_SPEED;
	Control.motor_control.motor4_speed = MOTOR_MAX_SPEED;
	Control_Update();
	HAL_Delay(3000);
	Control.motor_control.motor1_speed = 100;
	Control.motor_control.motor2_speed = 100;
	Control.motor_control.motor3_speed = 100;
	Control.motor_control.motor4_speed = 100;
	Control_Update();
	HAL_Delay(3000);

	printData("Motors armed\n");
}

void Control_Update(void) {
    //Control_Compute(ax, ay, az, gx, gy, gz, mx, my, mz);

    Control_SetMotorSpeeds();

    //Control_SendMotorCommands();
}

// Calcular la estabilización usando los datos de los sensores (acelerómetro, giroscopio, magnetómetro)
void Control_Compute(int ax, int ay, int az, int gx, int gy, int gz, int mx, int my, int mz) {
	float roll = atan2(ay, az) * 180.0 / M_PI;
    float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / M_PI;
    float yaw = atan2(my, mx) * 180.0 / M_PI;

    // error de cada eje
    float roll_error = 0 - roll;  // Queremos que el roll sea 0
    float pitch_error = 0 - pitch;  // Queremos que el pitch sea 0
    float yaw_error = 0 - yaw;  // Queremos que el yaw sea 0

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
    int motor1_speed = (Control.motor_control.motor1_speed < 0) ? 0 : (Control.motor_control.motor1_speed > MOTOR_MAX_SPEED ? MOTOR_MAX_SPEED : Control.motor_control.motor1_speed);
    int motor2_speed = (Control.motor_control.motor2_speed < 0) ? 0 : (Control.motor_control.motor2_speed > MOTOR_MAX_SPEED ? MOTOR_MAX_SPEED : Control.motor_control.motor2_speed);
    int motor3_speed = (Control.motor_control.motor3_speed < 0) ? 0 : (Control.motor_control.motor3_speed > MOTOR_MAX_SPEED ? MOTOR_MAX_SPEED : Control.motor_control.motor3_speed);
    int motor4_speed = (Control.motor_control.motor4_speed < 0) ? 0 : (Control.motor_control.motor4_speed > MOTOR_MAX_SPEED ? MOTOR_MAX_SPEED : Control.motor_control.motor4_speed);

    TIM1->CCR1 = motor1_speed;  // Motor 1 (TIM1 Channel 1)
    TIM1->CCR4 = motor2_speed;  // Motor 2 (TIM1 Channel 4)
    TIM2->CCR1 = motor3_speed;  // Motor 3 (TIM2 Channel 1)
    TIM2->CCR2 = motor4_speed;  // Motor 4 (TIM2 Channel 2)
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

    Control_Update(); //igual mejor no empezar ajustando por si acaso pero ya se quitara si no
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

	Control_Update();
	printData("Motors stopped\n");
}
