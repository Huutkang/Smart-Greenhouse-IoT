#ifndef WIFI_MQTT_H
#define WIFI_MQTT_H

#include <Arduino.h>


extern bool isAuto[10];
extern bool status[10];
extern bool mqtt_connected;
extern int lower_limit[9];
extern int upper_limit[9];
extern int max_time[9];
extern int maxTemperature;
extern bool isActive[10];
extern int ssMin[9];
extern int ssMax[9];
extern String mqttMessage;

void setupWiFi();
void setupMQTT();
void connect_MQTT();
bool publishData(const char* topic, const char* payload);
void ProcessTimerString(const String& input);
void handleMQTT();


#endif
