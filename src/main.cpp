#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// Cấu hình WiFi
const char* ssid = "XIAOXIN-PRO-14";
const char* password = "09032002";

// Cấu hình HiveMQ Broker
const char* mqtt_server = "da515a6f948a482bb656f7310841d60d.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "huuthang";
const char* mqtt_pass = "123456";

// Chủ đề MQTT
const char* light_topic = "greenhouse/light";
const char* control_topic = "greenhouse/control";

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

// Khai báo đối tượng WiFiClientSecure và PubSubClient
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Cấu hình pin
const int relayPin = 19;
const int lightSensorPin = 35;

// Biến trạng thái
enum Mode { MANUAL, AUTO };
Mode currentMode = AUTO;
bool manualState = HIGH;

// Cấu hình cảm biến
const float Vref = 3.3; // Điện áp tham chiếu
const float RL = 1000; // Điện trở cố định trong mạch phân áp

void setup_wifi() {
    delay(10);
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
}

// Hàm callback khi nhận dữ liệu từ MQTT
void callback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    if (String(topic) == control_topic) {
        if (message == "ON") {
            currentMode = MANUAL;
            manualState = LOW;
            digitalWrite(relayPin, LOW);
        } else if (message == "OFF") {
            currentMode = MANUAL;
            manualState = HIGH;
            digitalWrite(relayPin, HIGH);
        } else if (message == "AUTO") {
            currentMode = AUTO;
        }
    }
}

// Hàm reconnect để kết nối lại với MQTT nếu bị mất kết nối
void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
            Serial.println("connected");
            client.subscribe(control_topic);
        } else {
            Serial.print("failed, rc=");
            Serial.println(client.state());
            delay(5000);
        }
    }
}

float readLux() {
    int adcValue = analogRead(lightSensorPin);
    float voltage = (adcValue * Vref) / 4095.0;
    // một cách đổi nào đó từ hiệu điện thế sang thang đo độ sáng
    return 2000*voltage; // Tạm tính, cần điều chỉnh theo datasheet
}

void setup() {
    Serial.begin(115200);
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, HIGH);

    // Thiết lập WiFi
    setup_wifi();

    // Thiết lập chứng chỉ CA
    espClient.setCACert(ca_cert);

    // Thiết lập kết nối MQTT
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    Serial.begin(115200);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    float lux = readLux();
    Serial.println("Lux: " + String(lux));
    client.publish(light_topic, String(lux).c_str());

    // Điều khiển Relay
    if (currentMode == AUTO) {
        if (lux < 500) {
            digitalWrite(relayPin, LOW);
        } else {
            digitalWrite(relayPin, HIGH);
        }
    } else if (currentMode == MANUAL) {
        // Chế độ thủ công: Giữ trạng thái theo điều khiển của người dùng
        digitalWrite(relayPin, manualState);
    }

    delay(1000);
}
