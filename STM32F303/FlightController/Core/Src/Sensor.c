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
    // Inicializar el acelerómetro (ADXL345)
    uint8_t data = 0x08; // Encender el sensor
    HAL_I2C_Mem_Write(&hi2c1, (ADXL345_ADDR << 1), 0x2D, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

    // Inicializar el magnetómetro (HMC5883L)
    data = 0x70; // Configuración estándar
    HAL_I2C_Mem_Write(&hi2c1, (HMC5883L_ADDR << 1), 0x00, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

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


void Sensor_Read(void) {
    ADXL345_ReadData(&ax, &ay, &az);  // Acelerómetro
    ITG3205_ReadData(&gx, &gy, &gz);  // Giroscopio
    HMC5883L_ReadData(&mx, &my, &mz);  // Magnetómetro
}

void printMagnetometro() {
    int16_t x, y, z;
    char data_msg[64];
    HMC5883L_ReadData(&x, &y, &z);
    snprintf(data_msg, sizeof(data_msg), "MAGX:%d MAGY:%d MAGZ:%d\r\n", x, y, z);
    // Supongamos que tienes una función `printData()` que envía datos a través de UART
    printData(data_msg);
}

void printGiroscopio() {
    int16_t x, y, z;
    char data_msg[64];
    ITG3205_ReadData(&x, &y, &z);
    snprintf(data_msg, sizeof(data_msg), "GYX:%d GYY:%d GYZ:%d\r\n", x, y, z);
    printData(data_msg);
}

void printAcelerometro() {
    int16_t x, y, z;
    char data_msg[64];
    ADXL345_ReadData(&x, &y, &z);
    snprintf(data_msg, sizeof(data_msg), "ACCX:%d ACCY:%d ACCZ:%d\r\n", x, y, z);
    printData(data_msg);
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

	    printData(data_msg);
	    sendData(data_msg);
}
