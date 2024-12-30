#ifndef SENSOR_H
#define SENSOR_H


extern float sensor[10];
extern bool ADS1115_connected;
extern bool DHT11_connected;
extern float temperature;

void setupSensors(int ADC[6]);
void readSensorsADS1115();
void readLightSensor(int pin);
void readHumidityTemperature();
int readSensor(int pin);
#endif
