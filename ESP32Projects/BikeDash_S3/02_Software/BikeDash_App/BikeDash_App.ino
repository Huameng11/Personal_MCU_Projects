#include <Arduino.h>
#include "src/BLE_Handler.h"

// 实例化蓝牙处理器对象
BLE_Handler bleHandler;

void setup() {
    Serial.begin(115200);
    
    // 初始化蓝牙模块
    bleHandler.init();
    
    // 后期可以在这里继续增加其他模块的初始化
    // gpsHandler.init();
    // dashDisplay.init();
}

void loop() {
    // 保持蓝牙内部状态机的运转
    bleHandler.handle();
    
    // 老师，你可以在这里的任意地方，直接异步读取全局变量 `g_heart_rate` 
    // 或者通过 `bleHandler.getHeartRate()` 来拿数据发给屏幕，绝对不会卡死
    static unsigned long lastPrintTime = 0;
    if (millis() - lastPrintTime > 1000) {
        if (bleHandler.isConnected()) {
            Serial.printf("[Main主循环] 业务层获取到心率: %d bpm\n", g_heart_rate);
        }
        lastPrintTime = millis();
    }
    
    delay(10); 
}