#ifndef SENSOR_H
#define SENSOR_H


extern float sensor[9];
extern bool ADS1115_connected;
extern bool DHT11_connected;
extern float temperature;
extern float humidity;
extern float sensorMax[9];
extern float sensorMin[9];

void setupSensors(int ADC[5]);
void readSensorsADS1115();
void readLightSensor(int pin);
void readHumidityTemperature();
void readSensor();
#endif
