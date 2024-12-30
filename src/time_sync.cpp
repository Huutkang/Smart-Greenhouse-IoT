#include <Arduino.h>
#include <time.h>


// Cấu hình NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600; // UTC+7
const int daylightOffset_sec = 0;

// Biến hẹn giờ
bool timer_variable[10] = {false, false, false, false, false, false, false, false};

struct Timer {
    unsigned long startTime; // Thời điểm kích hoạt (giây từ đầu ngày)
    bool isActive;           // Trạng thái của hẹn giờ (đang bật hay tắt)
};

Timer relayTimers[10][4];

// Thiết lập thời gian từ NTP
void setupTimeSync() {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Không thể đồng bộ thời gian từ NTP!");
        return;
    }
    Serial.println("Đã đồng bộ thời gian từ NTP!");
}

// Lấy thời gian hiện tại ở dạng HH:MM:SS
String getCurrentTime() {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char buffer[9];
        sprintf(buffer, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        return String(buffer);
    } else {
        return "00:00:00";
    }
}

// Lấy số giây kể từ đầu ngày
unsigned long getSecondsSinceMidnight() {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        return timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
    }
    return 0;
}

// Khởi tạo tất cả các hẹn giờ là không hoạt động
void initializeTimers() {
    for (int relay = 0; relay <10; relay++) {
        for (int timer = 0; timer < 4; timer++) {
            relayTimers[relay][timer].startTime = 0;
            relayTimers[relay][timer].isActive = false;
        }
    }
}

// Đặt hẹn giờ cho máy bơm
void SetWateringTimer(int relayIndex, int timerIndex, unsigned long startTimeInSeconds) {
    if (relayIndex < 0 || relayIndex > 9 || timerIndex < 0 || timerIndex > 4) {
        return; // Chỉ số không hợp lệ
    }
    relayTimers[relayIndex][timerIndex].startTime = startTimeInSeconds;
    relayTimers[relayIndex][timerIndex].isActive = true;
}

// Xử lý chuỗi hẹn giờ đầu vào
void ProcessTimerString(String& input) {
    if (input.length() < 3) {
        input = ""; // Xóa chuỗi sau khi xử lý
        return;
    }

    int relayIndex = input[0] - '0';
    int timerIndex = input[1] - '0';

    if (relayIndex < 0 || relayIndex > 3 || timerIndex < 0 || timerIndex > 3) {
        input = ""; // Xóa chuỗi sau khi xử lý
        return;
    }

    String remaining = input.substring(2);
    remaining.trim();

    if (remaining == "off") {
        relayTimers[relayIndex][timerIndex].isActive = false;
    } else {
        unsigned long startTimeInSeconds = remaining.toInt();
        if (startTimeInSeconds > 0) {
            SetWateringTimer(relayIndex, timerIndex, startTimeInSeconds);
        }
    }

    input = ""; // Xóa chuỗi sau khi xử lý
}

// Kiểm tra và kích hoạt các hẹn giờ
void checkAndActivateTimers() {
    unsigned long currentSeconds = getSecondsSinceMidnight();

    for (int relay = 0; relay < 10; relay++) {
        for (int timer = 0; timer < 4; timer++) {
            if (relayTimers[relay][timer].isActive &&
                relayTimers[relay][timer].startTime + 60 > currentSeconds &&
                currentSeconds >= relayTimers[relay][timer].startTime) {
                
                timer_variable[relay] = true;
                relayTimers[relay][timer].isActive = false; // Vô hiệu hóa sau khi kích hoạt
            }
        }
    }
}
