#include "wifi_mqtt.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>




// Cấu hình HiveMQ Broker
const char* mqtt_server = "271915439423479da7c90ddd3723cd0c.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "huuthang";
const char* mqtt_pass = "123456aA@";

// Chủ đề MQTT (lắng nghe)
const char* control_topic = "control";
const char* config_topic = "config";

// Chứng chỉ Root CA
const char* ca_cert = R"~~~(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)~~~";


bool isActive[10] = {true, true, true, true, true, true, true, true, true, true}; //  trong bản demo dùng 2 cảm biến ánh sáng và nhiệt độ nên các cái khác được tắt đi

bool isAuto[10] = {true, true, true, true, true, true, true, true, true, true};  // true: AUTO, false: not AUTO.

bool status[10] = {false, false, false, false, false, false, false, false, false, false};

bool mqtt_connected = false;

int count_connect_wifi = 0;
int lower_limit[9] = {0, 0, 0, 60, 60, 60, 60, 60, 60}; // 9 cảm biến tối đa  (5 + 4 ads1115)
int upper_limit[9] = {10, 10, 10, 90, 90, 90, 90, 90, 90};
int maxTemperature = 40;
int max_time[9] = {18000,18000, 18000, 600, 600, 600, 600, 600};

int ssMin[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1}; // mảng đặt lại giá trị max, min cho cảm biến
int ssMax[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1}; // chỉ dùng cho người lắp đặt sản phẩm, k dành cho người dùng phổ thông

// Biến lưu trữ chuỗi MQTT nhận được
String mqttMessage = "";


// Định nghĩa MQTT và Wi-Fi
WiFiClientSecure espClient;          // Đối tượng WiFiClient
PubSubClient mqttClient(espClient);  // Đối tượng MQTT client




void setupWiFi() {
    WiFiManager wifiManager;
    wifiManager.setTimeout(300);
    // Tự động kết nối hoặc tạo Access Point
    wifiManager.autoConnect("ESP32_AP");
    // nếu không kết nối được wifi lúc khởi động thì sẽ tạo điểm truy cập cho người dùng nhập mật khẩu qua web của WiFiManager (khi di chuyển k phải nạp lại code)
    // phát điểm truy cập ra tối đa 5 phút.
    // sẽ có 2 trường hợp xảy ra. nếu người dùng di chuyển thiết bị từ nơi này đến nơi khác, mới mua về ... thì người dùng sẽ có 5 phút để nhập mật khẩu
    // nếu trong trường hợp mất điện. cả wifi và thiết bị đều bị tắt và khởi động lại lúc có điện. thiết bị chưa kịp bắt wifi thì đã nhảy vào hàm phát wifi trên
    // thì khi này set timeout 5 phút trên kia có tác dụng. thiết bị sẽ tự kết nối lại được wifi (trong hàm loop gọi hàm connect_MQTT -> WiFi.reconnect())
}


void callback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    if (String(topic) == control_topic) { // người dùng bật relay số index
        Serial.println(message);
        if (message.startsWith("ON")) {
            int relayIndex = message.substring(2).toInt();
            if (relayIndex >= 0 && relayIndex < 10) {
                isAuto[relayIndex] = false;
                status[relayIndex] = true;
            }
        } else if (message.startsWith("OFF")) { // người dùng tắt relay số index
            int relayIndex = message.substring(3).toInt();
            if (relayIndex >= 0 && relayIndex < 10) {
                isAuto[relayIndex] = false;
                status[relayIndex] = false;
            }
        } else if (message.startsWith("AUTO")) { // người dùng đặt chế độ auto cho relay số index
            int relayIndex = message.substring(4).toInt();
            if (relayIndex >= 0 && relayIndex < 10) {
                isAuto[relayIndex] = true;
                status[relayIndex] = false;
            }
        }
    } else if (String(topic) == config_topic) {
        if (message.startsWith("AC")) { // tính năng chỉ dành cho người lắp đặt sản phẩm. để bật tắt cấu hình lại cảm biến, relay
            int index = message.substring(2, 3).toInt();
            int iAc = message.substring(4, 5).toInt();
            if (index >= 0 && index < 9) {
                if (iAc == 1){
                    isActive[index] = true;
                } else if (iAc == 0){
                    isActive[index] = false; // tắt cảm biến, relay ở vị trí tương ứng. thường là trong trường hợp thừa, không dùng hết số chân
                }
            }
        }else if (message.startsWith("ssmin")) { // giúp người thiết lập, chuẩn hóa lại giá trị cảm biến
            int index = message.substring(5, 6).toInt();
            int newMin = message.substring(7).toInt();
            if (index >= 0 && index < 9) {
                ssMin[index] = newMin;
            }
        } else if (message.startsWith("ssmax")) { // giúp người thiết lập, chuẩn hóa lại giá trị cảm biến
            int index = message.substring(5, 6).toInt();
            int newMax = message.substring(7).toInt();
            if (index >= 0 && index < 9) {
                ssMax[index] = newMax;
            }
        }else if (message.startsWith("MIN")) { // người dùng đặt ngưỡng dưới của cảm biến
            int relayIndex = message.substring(3, 4).toInt();
            int newMin = message.substring(5).toInt();
            if (relayIndex >= 0 && relayIndex < 9 && newMin > 0 && newMin <= upper_limit[relayIndex]) {
                lower_limit[relayIndex] = newMin;
            }
        } else if (message.startsWith("MAX")) { // người dùng đặt ngưỡng trên của cảm biến
            int relayIndex = message.substring(3, 4).toInt();
            int newMax = message.substring(5).toInt();
            if (relayIndex >= 0 && relayIndex < 9 && newMax > lower_limit[relayIndex] && newMax <= 100) {
                upper_limit[relayIndex] = newMax;
            }
        } else if (message.startsWith("TM")) { // người dùng đặt giá trị thời gian hoạt động tối đa cho thiết bị số ...
            int relayIndex = message.substring(2, 3).toInt();
            int newMaxTime = message.substring(4).toInt();
            if (relayIndex >= 0 && relayIndex < 9 && newMaxTime > 0) {
                max_time[relayIndex] = newMaxTime;
            }
        } else if (message.startsWith("MT")) {
            maxTemperature = message.substring(3).toInt();
        }else{
            mqttMessage = message; // Lưu lại toàn bộ chuỗi nhận được, phục vụ cho hẹn giờ ở file khác
        }
    }
}

void connect_MQTT() {
    // Thử kết nối MQTT
    if (!mqttClient.connected()) {
        Serial.print("Đang kết nối MQTT...");
        if (mqttClient.connect("ESP32Client", mqtt_user, mqtt_pass)) {
            mqtt_connected = true;
            // Serial.println("Đã kết nối MQTT!");
            mqttClient.subscribe(control_topic);
            mqttClient.subscribe(config_topic);
        } else {
            Serial.print("Lỗi MQTT: ");
            // Serial.println(mqttClient.state());
            if (WiFi.status() != WL_CONNECTED || count_connect_wifi > 5) {
                count_connect_wifi = 0;
                // Serial.println("Lỗi wifi");
                WiFi.reconnect();
            } else {
                count_connect_wifi++;
            }
            for (int i = 0; i < 10; i++) {
                isAuto[i] = true;
            }
        }
    }
}


void setupMQTT() {
    espClient.setInsecure();
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(callback);
    connect_MQTT();
}

bool publishData(const char* topic, const char* payload) {
    mqttClient.loop();
    if (mqttClient.connected()) {
        mqttClient.publish(topic, payload);
        return true;
    } else {
        mqtt_connected = false;
        return false;
    }
}

void handleMQTT() {
    if (mqttClient.connected()) {
        mqttClient.loop();
    } else {
        mqtt_connected = false;
    }
}
