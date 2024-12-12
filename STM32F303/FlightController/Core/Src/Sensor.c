#include "sensor.h"

// Definiciones de direcciones I2C para los sensores
#define ADXL345_ADDR  0x53
#define HMC5883L_ADDR 0x1E
#define ITG3205_ADDR  0x68

int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;


extern I2C_HandleTypeDef hi2c1;


void GY85_Init() {
    uint8_t data;

    // Inicializar el acelerómetro (ADXL345)
    data = 0x08; // Encender el sensor
    HAL_I2C_Mem_Write(&hi2c1, (ADXL345_ADDR << 1), 0x2D, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

    // Inicializar el magnetómetro (HMC5883L)
    // Configurar el Configuration Register A (0x00)
    data = 0x70; // 8 muestras promedio, salida a 15 Hz
    HAL_I2C_Mem_Write(&hi2c1, (HMC5883L_ADDR << 1), 0x00, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

    // Configurar el Mode Register (0x02) para modo continuo
    data = 0x00; // Modo continuo
    HAL_I2C_Mem_Write(&hi2c1, (HMC5883L_ADDR << 1), 0x02, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

    // Inicializar el giroscopio (ITG-3205)
    data = 0x00; // Configuración estándar
    HAL_I2C_Mem_Write(&hi2c1, (ITG3205_ADDR << 1), 0x3E, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
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
    ADXL345_ReadData(&giro->ax, &giro->ay, &giro->az);  // Acelerómetro
    ITG3205_ReadData(&giro->gx, &giro->gy, &giro->gz);  // Giroscopio
    HMC5883L_ReadData(&giro->mx, &giro->my, &giro->mz);  // Magnetómetro
}

void printMagnetometro() {
    int16_t x, y, z;
    char data_msg[64];
    HMC5883L_ReadData(&x, &y, &z);
    snprintf(data_msg, sizeof(data_msg), "MAGX:%d MAGY:%d MAGZ:%d\r\n", x, y, z);
    printf(data_msg);
}

void printGiroscopio() {
    int16_t x, y, z;
    char data_msg[64];
    ITG3205_ReadData(&x, &y, &z);
    snprintf(data_msg, sizeof(data_msg), "GYX:%d GYY:%d GYZ:%d\r\n", x, y, z);
    printf(data_msg);
}

void printAcelerometro() {
    int16_t x, y, z;
    char data_msg[64];
    ADXL345_ReadData(&x, &y, &z);
    snprintf(data_msg, sizeof(data_msg), "ACCX:%d ACCY:%d ACCZ:%d\r\n", x, y, z);
    printf(data_msg);
}


void I2C_Scan() {
    char msg[64];
    for (uint8_t i = 0; i < 128; i++) {
        if (HAL_I2C_IsDeviceReady(&hi2c1, i << 1, 1, HAL_MAX_DELAY) == HAL_OK) {
            snprintf(msg, sizeof(msg), "Dispositivo encontrado en: 0x%02X\r\n", i);
            printf(msg);
        }
    }
}



void sendGY85Data() {

	    int16_t x, y, z, u, v, w, a, b, c;
	    char data_msg[2 << 8]; // 512 bytes

	    // Read data
	    ITG3205_ReadData(&x, &y, &z);
	    HMC5883L_ReadData(&u, &v, &w);
	    ADXL345_ReadData(&a, &b, &c);
	    snprintf(data_msg, sizeof(data_msg), "GYX:%d GYY:%d GYZ:%d MAGX:%d MAGY:%d MAGZ:%d ACX:%d ACY:%d ACZ:%d", x, y, z, u, v, w, a, b, c);
	    data_msg[strlen(data_msg)] = '\0';

	    printf(data_msg);
	    sendData(data_msg);
}
