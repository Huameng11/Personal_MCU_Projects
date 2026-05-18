#ifndef BLE_HANDLER_H
#define BLE_HANDLER_H

#include <Arduino.h>

class BLE_Handler {
public:
    BLE_Handler();
    ~BLE_Handler();

    // 初始化蓝牙配置（主频、发射功率等）
    void init();
    
    // 在 main.cpp 的 loop() 中高频调用，负责处理连接状态机
    void handle();

    // 供外部获取当前的实时心率
    int getHeartRate();
    
    // 供外部获取当前是否处于连接状态
    bool isConnected();
};

// 声明一个全局心率变量，方便屏幕或其他模块直接异步读取
extern int g_heart_rate;

#endif // BLE_HANDLER_H