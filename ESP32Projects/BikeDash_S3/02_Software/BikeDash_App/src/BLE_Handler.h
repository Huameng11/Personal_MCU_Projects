#ifndef BLE_HANDLER_H
#define BLE_HANDLER_H

#include <Arduino.h>

// 导出的全局变量，让主程序能读取心率
extern int current_heart_rate;
extern bool is_watch_connected;

// 导出的函数接口
void init_ble_service();
void update_ble_loop();

#endif