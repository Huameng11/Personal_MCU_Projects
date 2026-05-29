#include "src/BLE_Handler.h"     
#include "src/Display_Handler.h" 
#include "src/GPS_Handler.h"     

BLE_Handler bleHandler;
Display_Handler displayHandler; 
GPS_Handler gpsHandler;         

// 维护一套基础的业务逻辑变量
float currentSpeed = 0.0f;
float totalDistance = 0.0f;
unsigned long rideStartTime = 0;
bool isRiding = false;

void setup() {
    Serial.begin(115200);
    displayHandler.init();
    gpsHandler.init();
    bleHandler.init();
    
    rideStartTime = millis(); // 记录开机时间
}

void loop() {
    bleHandler.handle();
    
    // 1. 读取并解析 GPS 数据
    String currentLine = "";
    if (gpsHandler.readRawLine(currentLine)) {
        gpsHandler.parseNmealine(currentLine); 
    }
    
    // 2. 每 200ms 计算业务指标并刷新精美界面
    static unsigned long lastUpdateTime = 0;
    if (millis() - lastUpdateTime > 200) {
        // A. 心率及蓝牙状态
        int current_hr = bleHandler.getHeartRate(); 
        bool ble_status = bleHandler.isConnected(); 
        
        // B. 从 GPS 模块提取真实状态（如果未定位，临时用 0.0 模拟测试）
        bool gps_fixed = gpsHandler.isFixed;
        
        // 临时测试模拟：如果你在室内，为了看动态效果，可以让速度模拟跳动
        if (!gps_fixed) {
            currentSpeed = 0.0f; 
            totalDistance = 0.00f;
        } else {
            // 这里留空，后续接入真实的速度和里程累加算法
        }

        // C. 计算骑行时间 (格式化为 HH:MM:SS)
        unsigned long elapsed = (millis() - rideStartTime) / 1000;
        char timeBuf[12];
        sprintf(timeBuf, "%02lu:%02lu:%02lu", elapsed / 3600, (elapsed % 3600) / 60, elapsed % 60);
        String rTimeStr = String(timeBuf);

        // D. 获取系统当前时钟 (临时用 RTC 或开机时间模拟，后续从 GPS 授时中提取)
        // 临时生成一个变化的测试时钟
        unsigned long totalMinutes = (millis() / 60000) + 1151; // 模拟从 19:11 开始
        char clockBuf[8];
        sprintf(clockBuf, "%02lu:%02lu", (totalMinutes / 60) % 24, totalMinutes % 60);
        String rClockStr = String(clockBuf);
        
        // E. 真正送去复刻版界面显示
        displayHandler.update(current_hr, ble_status, currentSpeed, totalDistance, rTimeStr, rClockStr, gps_fixed);
        
        lastUpdateTime = millis();
    }
    
    delay(2); 
}