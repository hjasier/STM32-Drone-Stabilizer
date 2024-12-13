#include "sensor.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

// Definiciones de direcciones I2C para los sensores
#define ADXL345_ADDR  0x53
#define HMC5883L_ADDR 0x1E
#define ITG3205_ADDR  0x68

// Variables globales para sensores
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;
extern I2C_HandleTypeDef hi2c1;

// Estructura para el filtro de Kalman
typedef struct {
    float q;  // Varianza del proceso
    float r;  // Varianza de la medida
    float x;  // Valor estimado
    float p;  // Varianza estimada
    float k;  // Ganancia de Kalman
} KalmanFilter;

// Inicializa el filtro de Kalman
void Kalman_Init(KalmanFilter* filter, float q, float r, float initial_value) {
    filter->q = q;
    filter->r = r;
    filter->x = initial_value;
    filter->p = 1.0;
    filter->k = 0.0;
}

// Actualiza el filtro de Kalman con un nuevo valor medido
float Kalman_Update(KalmanFilter* filter, float measurement) {
    // Predicción
    filter->p += filter->q;

    // Actualización
    filter->k = filter->p / (filter->p + filter->r);
    filter->x += filter->k * (measurement - filter->x);
    filter->p *= (1 - filter->k);

    return filter->x;
}

// Filtros de Kalman para cada eje
KalmanFilter kalman_ax, kalman_ay, kalman_az;
KalmanFilter kalman_gx, kalman_gy, kalman_gz;
KalmanFilter kalman_mx, kalman_my, kalman_mz;

void Filters_Init() {
    // Inicializamos los filtros con valores ajustados para ruido y varianza
    Kalman_Init(&kalman_ax, 0.001, 0.1, 0);
    Kalman_Init(&kalman_ay, 0.001, 0.1, 0);
    Kalman_Init(&kalman_az, 0.001, 0.1, 0);
    Kalman_Init(&kalman_gx, 0.001, 0.1, 0);
    Kalman_Init(&kalman_gy, 0.001, 0.1, 0);
    Kalman_Init(&kalman_gz, 0.001, 0.1, 0);
    Kalman_Init(&kalman_mx, 0.001, 0.1, 0);
    Kalman_Init(&kalman_my, 0.001, 0.1, 0);
    Kalman_Init(&kalman_mz, 0.001, 0.1, 0);
}

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

  // Aplicar filtro de Kalman a cada eje
//    giro->ax = Kalman_Update(&kalman_ax, giro->ax);
//    giro->ay = Kalman_Update(&kalman_ay, giro->ay);
//    giro->az = Kalman_Update(&kalman_az, giro->az);
//    giro->gx = Kalman_Update(&kalman_gx, giro->gx);
//    giro->gy = Kalman_Update(&kalman_gy, giro->gy);
//    giro->gz = Kalman_Update(&kalman_gz, giro->gz);
//    giro->mx = Kalman_Update(&kalman_mx, giro->mx);
//    giro->my = Kalman_Update(&kalman_my, giro->my);
//    giro->mz = Kalman_Update(&kalman_mz, giro->mz);

}




void printGyro() {
    struct girodata_t giro;
    Sensor_Read(&giro);

    printf("AX: %i, AY: %i, AZ: %i, GX: %i, GY: %i, GZ: %i, MX: %i, MY: %i, MZ: %i\n", giro.ax, giro.ay, giro.az, giro.gx, giro.gy, giro.gz, giro.mx, giro.my, giro.mz);
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
