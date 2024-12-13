#include "sensor.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

// Definiciones de direcciones I2C para los sensores
#define ADXL345_ADDR  0x53
#define HMC5883L_ADDR 0x1E
#define ITG3205_ADDR  0x68

#define FIXED_DT 0.001f

// Variables globales para sensores
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;
extern I2C_HandleTypeDef hi2c1;

static Kalman_t kalman_pitch, kalman_roll; // Instancias del filtro Kalman
static float pitch = 0.0f, roll = 0.0f;



void GY85_Init() {
    uint8_t data;

    // Inicializar el acelerómetro (ADXL345)
    data = 0x08; // Encender el sensor
    HAL_I2C_Mem_Write(&hi2c1, (ADXL345_ADDR << 1), 0x2D, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
    data = 0x01; // Configurar rango ±4g
    HAL_I2C_Mem_Write(&hi2c1, (ADXL345_ADDR << 1), 0x31, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

    // Inicializar el magnetómetro (HMC5883L)
    data = 0x70; // Configurar promedio y frecuencia
    HAL_I2C_Mem_Write(&hi2c1, (HMC5883L_ADDR << 1), 0x00, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
    data = 0x00; // Modo continuo
    HAL_I2C_Mem_Write(&hi2c1, (HMC5883L_ADDR << 1), 0x02, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

    // Inicializar el giroscopio (ITG-3205)
    data = 0x03; // Usar PLL con eje Z
    HAL_I2C_Mem_Write(&hi2c1, (ITG3205_ADDR << 1), 0x3E, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
    data = 0x18; // Configurar rango ±2000°/s
    HAL_I2C_Mem_Write(&hi2c1, (ITG3205_ADDR << 1), 0x16, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

    CalibrateAccelerometer();
    CalibrateGyroscope();
    CalibrateMagnetometer();
}


void ADXL345_ReadData(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t buffer[6];
    HAL_I2C_Mem_Read(&hi2c1, (ADXL345_ADDR << 1), 0x32, I2C_MEMADD_SIZE_8BIT, buffer, 6, HAL_MAX_DELAY);
    *x = (int16_t)((buffer[1] << 8) | buffer[0]);
    *y = (int16_t)((buffer[3] << 8) | buffer[2]);
    *z = (int16_t)((buffer[5] << 8) | buffer[4]);
}

void HMC5883L_ReadData(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t buffer[6];
    HAL_I2C_Mem_Read(&hi2c1, (HMC5883L_ADDR << 1), 0x03, I2C_MEMADD_SIZE_8BIT, buffer, 6, HAL_MAX_DELAY);
    *x = (int16_t)((buffer[0] << 8) | buffer[1]);
    *z = (int16_t)((buffer[2] << 8) | buffer[3]);
    *y = (int16_t)((buffer[4] << 8) | buffer[5]);
}

void ITG3205_ReadData(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t buffer[6];
    HAL_I2C_Mem_Read(&hi2c1, (ITG3205_ADDR << 1), 0x1D, I2C_MEMADD_SIZE_8BIT, buffer, 6, HAL_MAX_DELAY);
    *x = (int16_t)((buffer[0] << 8) | buffer[1]);
    *y = (int16_t)((buffer[2] << 8) | buffer[3]);
    *z = (int16_t)((buffer[4] << 8) | buffer[5]);
}

void Sensor_Read(struct girodata_t* giro) {
    // Leer datos crudos de sensores
    ADXL345_ReadData(&giro->ax, &giro->ay, &giro->az);
    ITG3205_ReadData(&giro->gx, &giro->gy, &giro->gz);
    HMC5883L_ReadData(&giro->mx, &giro->my, &giro->mz);

    // Compensar datos
    CompensateAccelerometer(&giro->ax, &giro->ay, &giro->az);
    CompensateGyroscope(&giro->gx, &giro->gy, &giro->gz);
    CompensateMagnetometer(&giro->mx, &giro->my, &giro->mz);

}

// Variables globales para compensación
float accel_offset_x = 0, accel_offset_y = 0, accel_offset_z = 0;
float gyro_offset_x = 0, gyro_offset_y = 0, gyro_offset_z = 0;
// Variables globales para compensación
float mag_min_x = 0, mag_max_x = 0;
float mag_min_y = 0, mag_max_y = 0;
float mag_min_z = 0, mag_max_z = 0;

void CalibrateMagnetometer() {
    mag_min_x = mag_min_y = mag_min_z = 32767;
    mag_max_x = mag_max_y = mag_max_z = -32768;

    for (int i = 0; i < 1000; i++) {
        int16_t x, y, z;
        HMC5883L_ReadData(&x, &y, &z);

        if (x < mag_min_x) mag_min_x = x;
        if (x > mag_max_x) mag_max_x = x;
        if (y < mag_min_y) mag_min_y = y;
        if (y > mag_max_y) mag_max_y = y;
        if (z < mag_min_z) mag_min_z = z;
        if (z > mag_max_z) mag_max_z = z;

        HAL_Delay(10);
    }
}

void CompensateMagnetometer(int16_t *x, int16_t *y, int16_t *z) {
    *x = 2 * (*x - mag_min_x) / (mag_max_x - mag_min_x) - 1;
    *y = 2 * (*y - mag_min_y) / (mag_max_y - mag_min_y) - 1;
    *z = 2 * (*z - mag_min_z) / (mag_max_z - mag_min_z) - 1;
}


void CalibrateAccelerometer() {
    int32_t sum_x = 0, sum_y = 0, sum_z = 0;
    int samples = 1000;

    for (int i = 0; i < samples; i++) {
        int16_t x, y, z;
        ADXL345_ReadData(&x, &y, &z);
        sum_x += x;
        sum_y += y;
        sum_z += z;
        HAL_Delay(5);  // Esperar un poco entre lecturas
    }

    accel_offset_x = sum_x / samples;
    accel_offset_y = sum_y / samples;
    accel_offset_z = (sum_z / samples) - 16384; // Compensar gravedad (1g ≈ 16384 en ±2g)
}

void CompensateAccelerometer(int16_t *x, int16_t *y, int16_t *z) {
    *x -= accel_offset_x;
    *y -= accel_offset_y;
    *z -= accel_offset_z;
}



void CalibrateGyroscope() {
    int32_t sum_x = 0, sum_y = 0, sum_z = 0;
    int samples = 1000;

    for (int i = 0; i < samples; i++) {
        int16_t x, y, z;
        ITG3205_ReadData(&x, &y, &z);
        sum_x += x;
        sum_y += y;
        sum_z += z;
        HAL_Delay(5);
    }

    gyro_offset_x = sum_x / samples;
    gyro_offset_y = sum_y / samples;
    gyro_offset_z = sum_z / samples;
}

void CompensateGyroscope(int16_t *x, int16_t *y, int16_t *z) {
    *x -= gyro_offset_x;
    *y -= gyro_offset_y;
    *z -= gyro_offset_z;
}




void Kalman_Init(Kalman_t *kalman) {
    kalman->angle = 0.0f;
    kalman->bias = 0.0f;
    kalman->rate = 0.0f;

    kalman->P[0][0] = 0.0f;
    kalman->P[0][1] = 0.0f;
    kalman->P[1][0] = 0.0f;
    kalman->P[1][1] = 0.0f;

    kalman->Q_angle = 0.001f;
    kalman->Q_bias = 0.003f;
    kalman->R_measure = 0.03f;
}


float Kalman_GetAngle(Kalman_t *kalman, float newAngle, float newRate, float dt) {
    // Paso de predicción
    kalman->rate = newRate - kalman->bias;
    kalman->angle += dt * kalman->rate;

    kalman->P[0][0] += dt * (dt * kalman->P[1][1] - kalman->P[0][1] - kalman->P[1][0] + kalman->Q_angle);
    kalman->P[0][1] -= dt * kalman->P[1][1];
    kalman->P[1][0] -= dt * kalman->P[1][1];
    kalman->P[1][1] += kalman->Q_bias * dt;

    // Paso de corrección
    float S = kalman->P[0][0] + kalman->R_measure; // Innovación
    float K[2];                                   // Ganancia de Kalman
    K[0] = kalman->P[0][0] / S;
    K[1] = kalman->P[1][0] / S;

    float y = newAngle - kalman->angle;           // Error de medida
    kalman->angle += K[0] * y;
    kalman->bias += K[1] * y;

    float P00_temp = kalman->P[0][0];
    float P01_temp = kalman->P[0][1];

    kalman->P[0][0] -= K[0] * P00_temp;
    kalman->P[0][1] -= K[0] * P01_temp;
    kalman->P[1][0] -= K[1] * P00_temp;
    kalman->P[1][1] -= K[1] * P01_temp;

    return kalman->angle;
}


void Sensor_GetAngles(struct girodata_t* giro, Kalman_t* kalman_pitch, Kalman_t* kalman_roll, float* pitch, float* roll, float dt) {
    // Calcular ángulos con el acelerómetro
    float pitch_acc = atan2f((float)giro->ay, (float)giro->az) * 180 / M_PI;
    float roll_acc = atan2f((float)-giro->ax, sqrtf((float)(giro->ay * giro->ay + giro->az * giro->az))) * 180 / M_PI;

    // Obtener la velocidad angular del giroscopio
    float ratePitch = (float)giro->gx / 131.0f; // Sensibilidad típica del giroscopio
    float rateRoll = (float)giro->gy / 131.0f;

    // Aplicar el filtro Kalman
    *pitch = Kalman_GetAngle(kalman_pitch, pitch_acc, ratePitch, dt);
    *roll = Kalman_GetAngle(kalman_roll, roll_acc, rateRoll, dt);
}



void printKalman(struct girodata_t* giro) {
    // Leer datos de los sensores
    Sensor_Read(giro);

    // Calcular ángulos usando el filtro Kalman
    Sensor_GetAngles(giro, &kalman_pitch, &kalman_roll, &pitch, &roll, FIXED_DT);

    // Imprimir los resultados
    printf("Pitch: %.2d, Roll: %.2d\n", pitch, roll);
}
void printGyro() {
    struct girodata_t giro;
    Sensor_Read(&giro);

    //printf("AX: %i, AY: %i, AZ: %i, GX: %i, GY: %i, GZ: %i, MX: %i, MY: %i, MZ: %i\n", giro.ax, giro.ay, giro.az, giro.gx, giro.gy, giro.gz, giro.mx, giro.my, giro.mz);
    // Calcular los ángulos de inclinación (roll, pitch) a partir de los datos del acelerómetro
    float roll = atan2f(giro.ay, giro.az) * 180.0 / M_PI;
    float pitch = atan2f(-giro.ax, sqrtf(giro.ay * giro.ay + giro.az * giro.az)) * 180.0 / M_PI;

    // Calcular el rumbo (yaw) a partir del magnetómetro
    float yaw = atan2f((float)giro.my, (float)giro.mx) * 180.0 / M_PI;
    if (yaw < 0) yaw += 360.0;

    // Imprimir los valores calculados
    char msg[128];
    snprintf(msg, sizeof(msg), "Roll: %.2d, Pitch: %.2d, Yaw: %.2d\r\n", roll, pitch, yaw);
    printf(msg);
}
