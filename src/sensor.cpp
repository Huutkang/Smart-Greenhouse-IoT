#include "sensor.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ADS1115_WE.h>


const float Vref = 3.3; // Điện áp tham chiếu
const float RL = 1000; // Điện trở cố định trong mạch phân áp

bool ADS1115_connected = true;
bool DHT11_connected = true;

#define DHTPIN 4    // Chân kết nối tín hiệu của DHT11 với ESP32

// sensor/ADC
ADS1115_WE adc(0x48);

// sensor/DHT11
DHT dht(DHTPIN, DHT11);


float humidity;
float temperature;
int arr_ADC[6];

// mảng lưu giá trị của cảm biến chuyển về dạng %
float sensor[10] = {100, 100, 100, 100, 100, 100, 100, 100, 100, 100};

// giá trị đo max của cảm biến ở môi trường thực tế (cần test trước để hiệu chỉnh). 4 cái sau là của ADS1115_WE
float sensorMax[10] = {2760, 2680, 4095, 4095, 4095, 4095, 32767, 32767, 32767, 32767};
// giá trị đo min của cảm biến ở môi trường thực tế (cần test trước để hiệu chỉnh). hiện tại mới dùng 2 cái đầu tiên cho ánh sáng và độ ẩm đất
float sensorMin[10] = {1460, 1210, 0, 0, 0, 0, 0, 0, 0, 0};


// có 10 cảm biến là 6 chân ADC được chọn trên esp32 và 4 chân bổ sung từ ADS1115_WE.
// ADS1115_WE dành cho những việc đo giá trị có khoảng cách min->max nhỏ hơn nhiều so với thang đo. hoặc đo ở khoảng cách xa và gửi dữ liệu về

// Khởi tạo các cảm biến
void setupSensors(int ADC[6]) {
    for (int i = 0; i < 6; i++) {
        arr_ADC[i] = ADC[i];
    }
    dht.begin();
    Wire.begin(SCL, SDA);
    if (!adc.init()) {
        Serial.println("ADS1115 not connected!");
        ADS1115_connected = false;
    }
    for (int i = 0; i < 6; i++) {
        pinMode(ADC[i], INPUT);
    }
}

// Đọc giá trị từ kênh ADC
float readChannel(ADS1115_MUX channel) {
    float voltage = 0.0;
    adc.setCompareChannels(channel);
    adc.startSingleMeasurement();
    voltage = adc.getResult_mV();
    return voltage;
}


// Đọc giá trị từ các cảm biến
void readSensorsADS1115() {
    float reading[4];

    // Đọc giá trị thô từ các kênh
    reading[0] = readChannel(ADS1115_COMP_0_GND);
    reading[1] = readChannel(ADS1115_COMP_1_GND);
    reading[2] = readChannel(ADS1115_COMP_2_GND);
    reading[3] = readChannel(ADS1115_COMP_3_GND);

    // Chuyển đổi thành phần trăm, lọc và ràng buộc giá trị
    for (int i = 0; i < 4; i++) {
        reading[i] = map(reading[i], sensorMax[i], sensorMin[i], 0, 100);
        sensor[i] = constrain(reading[i], 0, 100);
    }

}

void readLightSensor(int pin) {
    int adcValue = analogRead(arr_ADC[pin]);
    int value = map(adcValue, sensorMax[pin], sensorMin[pin], 100, 0);
    sensor[0] = constrain(value, 0, 100);
}

void readSoilMoisture(int pin) {
    int adcValue = analogRead(arr_ADC[pin]);
    int value = map(adcValue, sensorMax[pin], sensorMin[pin], 0, 100);
    sensor[0] = constrain(value, 0, 100);
}

void readSensor() {
    for (int i = 0; i <6; i++){
        if (i<3){
            readLightSensor(i);
        }else{
            readSoilMoisture(i);
        }
    }
    if (ADS1115_connected){
        readSensorsADS1115();
    }
    readHumidityTemperature();
}

void readHumidityTemperature(){
    // Đọc giá trị nhiệt độ và độ ẩm
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();

    // Kiểm tra nếu việc đọc dữ liệu bị lỗi
    if (isnan(humidity) || isnan(temperature)) {
        Serial.println(F("Lỗi: Không đọc được dữ liệu từ cảm biến DHT11"));
        DHT11_connected = false;
        return;
    }else{
        DHT11_connected = true;
    }
}