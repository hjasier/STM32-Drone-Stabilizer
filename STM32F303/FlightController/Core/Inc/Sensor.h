#ifndef SENSOR_H
#define SENSOR_H

#include "main.h"
#include <stdint.h>


struct girodata_t {
	int16_t ax, ay, az;  // Acelerómetro
	int16_t gx, gy, gz;  // Giroscopio
	int16_t mx, my, mz;  // Magnetómetro
};

void GY85_Init(void);
void Sensor_Read(struct girodata_t* giro);
void ADXL345_ReadData(int16_t *x, int16_t *y, int16_t *z);
void HMC5883L_ReadData(int16_t *x, int16_t *y, int16_t *z);
void ITG3205_ReadData(int16_t *x, int16_t *y, int16_t *z);
void printMagnetometro(void);
void printGiroscopio(void);
void printAcelerometro(void);
void sendGY85Data(void);

#endif /* SENSOR_H */
