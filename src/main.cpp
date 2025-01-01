#include <Arduino.h>
#include "wifi_mqtt.h"
#include "sensor.h"
#include "control.h"
#include "time_sync.h"



int ADC[5] = {32, 33, 34, 35, 36}; // dùng làm input, 4 cái của ADS1115 nữa là 9. giao tiếp I2C
int RL[10] = {14, 4, 5, 27, 17, 18, 19, 23, 2, 26}; // 9 cặp và chân ở vị trí số 9 RL[9] dành cho quạt

unsigned long current_time;
unsigned long time1=0;
unsigned long time2=0;
unsigned long time3=0;
unsigned long time4=0;
unsigned long time5=0;


int t[9];
int ActivationTime = 60; // thời gian chờ trước khi bật lại
bool count_status[9] = {false, false, false, false, false, false, false, false, false};


// với kiểu dữ liệu unsigned long: 10 - 4294967295 = 11 nên không lo tràn số ở hàm millis
int Timer(unsigned long *time, int wait){
    current_time = millis();
    if (current_time-*time>wait){
        *time = current_time;
        return 1;
    }
    else{
        return 0;
    }
}

void updateStatus() {
    for (int i = 0; i < 9; i++) {
        if (!isActive[i]){ // nếu thiết bị không dùng thì bỏ qua, và đặt lại chế độ là tắt.
            if (status[i]){
                status[i] = false;
            }
            continue;
        }
        if (!isAuto[i]) {
            // Nếu không ở chế độ tự động, bỏ qua relay này
            continue;
        }
        if (t[i]<0){
            status[i] = false;
        }

        if (sensor[i] <= lower_limit[i]) {
            if (i>2){ // chỉ với tự động tưới. relay máy bơm từ 3 đến 8
                if (!count_status[i]){
                    count_status[i] = true;
                    status[i] = true;
                }
            }
        } else if (sensor[i] >= upper_limit[i]) {
            status[i] = false; // Không tưới, chiếu sáng
        } else {
            if (timer_variable[i]) {
                if (!count_status[i]){
                    count_status[i] = true;
                    status[i] = true;
                }
                timer_variable[i] = false; // Reset lại bộ hẹn giờ
            }
        }
    }
    if (!isActive[9]){
        if (status[9]){
            status[9] = false;
        }
        return;
    }
    if (!isAuto[9]) {
        // Nếu không ở chế độ tự động, bỏ qua relay này
        return;
    }
    if (temperature > maxTemperature){
        status[9] = true; // nhiệt độ quá cao thì bật quạt (maxTemperature do setup của người dùng)
    }else{
        status[9] = false; 
    }
}

void config_sensor(){  // tính năng dành cho nhà phát triển. người dùng k dùng đến
    for (int i = 0; i < 9; i++) {
        if (ssMin[i] >= 0){
            sensorMin[i] = ssMin[i];
            ssMin[i] = -1;
        }
        if (ssMax[i] >=0){
            sensorMax[i] = ssMax[i];
            ssMax[i] = -1;
        }
    }
}

void setup() {
    Serial.begin(115200);
    setupRelay(RL);
    initializeTimers();
    setupWiFi();                  // Cấu hình WiFi
    setupMQTT();                 // Cấu hình MQTT
    setupSensors(ADC);       // Cấu hình cảm biến
    setupTimeSync();

    for (int i; i<9; i++){
        t[i] = max_time[i];
    }
}



// logic tưới cấy:

//     có hai chế độ là tự động và điều khiển bằng tay
//     nếu mất kết nối thì tự động chuyển về chế độ tự động
//     khi ở chế độ điều khiển bằng tay, người dùng tự bật lên thì tự tắt đi
//     khi ở chế độ tự động, người dùng lập lịch tưới hàng ngày
//     có hai giá trị độ ẩm là mix và max. ví dụ min=60 và max=90
//     nếu độ ẩm bé hơn min thì tưới cây, nếu độ ẩm lớn hơn max thì không tưới, 
//     nếu độ ẩm nằm giữa min và max thì tưới theo lịch
//     
//     khi relay được bật thì nó được tưới tối đa trong khoảng thời gian được lưu trong mảng max_time[10] (auto)
//     khi relay tắt thì nó phải chờ một khoảng thời gian là ActivationTime mới bật lên lại được (auto)
//     logic trên được triển khải bằng ActivationTime, t[10], count_status[10]

// logic điều chỉnh đổ sáng:
//     Giống hệt logic tưới cây. ánh sáng đo độ sáng xong giá trị adc được chuyển về %. mức thấp nhất đo được ở môi trường tự nhiên ứng với 0%, mức cao nhất là 100%
//     đối với những loại cây trồng cần lên lịch chiếu sáng thì nằm giữa 2 giá trị min, max thì bật tắt đèn theo lịch. nếu quá sáng thì thôi

// logic làm mát không khí bằng quạt:
//     Khi nhiệt độ không khí quá cao, hơn ngưỡng người dùng đặt thì quạt được bật. khi dưới ngưỡng thì tắt (cập nhật sau mỗi phút)


void loop() {
    handleMQTT();                 // Xử lý kết nối MQTT
    if (Timer(&time1, 3000)){ // kết nối lại mqtt, wifi (mếu mất kết nối)
        if(!mqtt_connected){
            connect_MQTT();
        }
    }
    if (Timer(&time2,1000)){ // đọc, gửi, in giá trị cảm biến
        readSensor();                
        for (int i = 0; i < 9; i++) {
            if (!isActive[i]){
                continue;
            }
            String message = String(sensor[i]);
            String ss = "ss"+String(i);
            publishData(ss.c_str(), message.c_str());
        }
        String relay_status;
        for (int i=0; i<10; i++){
            relay_status += String(status[i]); // trạng thái relay. bật hay tắt
        }
        publishData("PS", relay_status.c_str());
        config_sensor();
        if (DHT11_connected){
            if (isnan(humidity) || isnan(temperature)) {
                Serial.println("Failed to read from DHT sensor!");
                return;
            }
            publishData("h", String(humidity).c_str());
            publishData("t", String(temperature).c_str());
        }
    }
    if (Timer(&time3,500)){ // thực thi bật tắt relay
        manageRelay(RL, status);
        // for (int i=0; i<10; i++){
        //     Serial.println(status[i]);
        // }
    }
    if (Timer(&time4,998)){ // thay đổi trạng thái
        updateStatus(); // thay đổi trạng thái relay
        for (int i=0; i<9; i++){ // kiểm soát thời gian tưới tối đa và thời gian tối thiểu từ khi tắt đến khi bật (chế độ auto)
            if (count_status[i]){
                t[i]--; // bắt buộc để timer ở đây là 1 giây để hoạt động bình thường
            }
            if (t[i]<-ActivationTime){
                count_status[i]=false;
                t[i]=max_time[i];
            }
        }
    }
    if (Timer(&time5,991)){ // hẹn giờ
        ProcessTimerString(mqttMessage); // hẹn giờ hoạt động
        checkAndActivateTimers();  // kích hoạt các relay đã hẹn giờ
    }
}
