#include "control.h"
#include <Arduino.h>



void setupRelay(int RL[10]) {
    for (int i = 0; i < 10; i++) {
        pinMode(RL[i], OUTPUT);
        digitalWrite(RL[i], HIGH); // Tắt relay ban đầu
    }
}

void manageRelay(int RL[10], bool status[10]) {
    for (int i = 0; i < 10; i++) {
        digitalWrite(RL[i], !status[i]);
    }
}
