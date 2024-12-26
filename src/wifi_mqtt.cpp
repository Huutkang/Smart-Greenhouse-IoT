#include "wifi_mqtt.h"
#include <WiFi.h>
#include <PubSubClient.h>
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


bool isAuto[4] = {true, true, true, true};  // true: AUTO, false: not AUTO
bool status[4] = {false, false, false, false};
bool mqtt_connected = false;

int count_connect_wifi = 0;
int min_moisture[4] = {60, 60, 60, 60};
int max_moisture[4] = {90, 90, 90, 90};
int max_time[4] = {60, 60, 60, 60};


// Biến lưu trữ chuỗi MQTT nhận được
String mqttMessage = "";

// Định nghĩa MQTT và Wi-Fi
WiFiClientSecure espClient;          // Đối tượng WiFiClient
PubSubClient mqttClient(espClient);  // Đối tượng MQTT client

void setupWiFi() {
    WiFiManager wifiManager;

    // Tự động kết nối hoặc tạo Access Point
    if (!wifiManager.autoConnect("ESP8266_AP")) {
        // Serial.println("Không kết nối được Wi-Fi");
        delay(3000);
        ESP.restart();  // Khởi động lại thiết bị
    }
    // Serial.println("Đã kết nối Wi-Fi!");
    // sau cho set timeout là 5 phút. quá 5 phút sẽ thoát web và tự động kết nối lại với pass cũ nếu có
    // Thiết lập chứng chỉ CA
    espClient.setCACert(ca_cert);
}

void callback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    if (String(topic) == control_topic) {
        if (message.startsWith("ON")) {
            int relayIndex = message.substring(2).toInt() - 1;
            if (relayIndex >= 0 && relayIndex < 4) {
                isAuto[relayIndex] = false;
                status[relayIndex] = true;
            }
        } else if (message.startsWith("OFF")) {
            int relayIndex = message.substring(3).toInt() - 1;
            if (relayIndex >= 0 && relayIndex < 4) {
                isAuto[relayIndex] = false;
                status[relayIndex] = false;
            }
        } else if (message.startsWith("AUTO")) {
            int relayIndex = message.substring(4).toInt() - 1;
            if (relayIndex >= 0 && relayIndex < 4) {
                isAuto[relayIndex] = true;
                status[relayIndex] = false;
            }
        }
    } else if (String(topic) == config_topic) {
        if (message.startsWith("MIN")) {
            int relayIndex = message.substring(3, 4).toInt() - 1;
            int newMin = message.substring(5).toInt();
            if (relayIndex >= 0 && relayIndex < 4 && newMin > 0 && newMin <= max_moisture[relayIndex]) {
                min_moisture[relayIndex] = newMin;
                // Serial.println("Cập nhật min_moisture[" + String(relayIndex) + "]: " + String(min_moisture[relayIndex]));
            }
        } else if (message.startsWith("MAX")) {
            int relayIndex = message.substring(3, 4).toInt() - 1;
            int newMax = message.substring(5).toInt();
            if (relayIndex >= 0 && relayIndex < 4 && newMax > min_moisture[relayIndex] && newMax <= 100) {
                max_moisture[relayIndex] = newMax;
                // Serial.println("Cập nhật max_moisture[" + String(relayIndex) + "]: " + String(max_moisture[relayIndex]));
            }
        } else if (message.startsWith("TM")) {
            int relayIndex = message.substring(2, 3).toInt() - 1;
            int newMaxTime = message.substring(4).toInt();
            if (relayIndex >= 0 && relayIndex < 4 && newMaxTime > 0) {
                max_time[relayIndex] = newMaxTime;
                // Serial.println("Cập nhật max_time[" + String(relayIndex) + "]: " + String(max_time[relayIndex]));
            }
        }else{
            mqttMessage = message; // Lưu lại toàn bộ chuỗi nhận được
        }
    }
}

void connect_MQTT() {
    if (!mqttClient.connected()) {
        Serial.print("Đang kết nối MQTT...");
        if (mqttClient.connect("ESP8266Client", mqtt_user, mqtt_pass)) {
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
            for (int i = 0; i < 4; i++) {
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
