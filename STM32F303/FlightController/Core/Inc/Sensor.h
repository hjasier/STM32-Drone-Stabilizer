#ifndef SENSOR_H
#define SENSOR_H

#include "main.h"
#include <stdint.h>

struct girodata_t {
    int16_t ax, ay, az;  // Acelerómetro
    int16_t gx, gy, gz;  // Giroscopio
    int16_t mx, my, mz;  // Magnetómetro
};

//KALMAN
typedef struct {
    float angle;       // Ángulo estimado
    float bias;        // Sesgo estimado
    float rate;        // Tasa de giro sin sesgo
    float P[2][2];     // Matriz de covarianza de error
    float Q_angle;     // Varianza del ruido del proceso para el ángulo
    float Q_bias;      // Varianza del ruido del proceso para el sesgo
    float R_measure;   // Varianza del ruido de la medida
} Kalman_t;


void GY85_Init(void);
void Sensor_Read(struct girodata_t* giro);
void ADXL345_ReadData(int16_t *x, int16_t *y, int16_t *z);
void HMC5883L_ReadData(int16_t *x, int16_t *y, int16_t *z);
void ITG3205_ReadData(int16_t *x, int16_t *y, int16_t *z);
void sendGY85Data(void);
void printKalman(struct girodata_t* giro);

void Kalman_Init(Kalman_t *kalman);
float Kalman_GetAngle(Kalman_t *kalman, float newAngle, float newRate, float dt);
void Sensor_GetAngles(struct girodata_t* giro, Kalman_t* kalman_pitch, Kalman_t* kalman_roll, float* pitch, float* roll, float dt);

#endif /* SENSOR_H */
