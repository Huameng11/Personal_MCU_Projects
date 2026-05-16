#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

static BLEUUID serviceUUID("180d"); // 心率服务UUID
static BLEUUID    charUUID("2a37"); // 心率特征值UUID

static boolean doConnect = false;
static boolean isConnected = false;
static BLEAdvertisedDevice* myDevice;
int heart_rate = 0;

// 接收到手表心率推送的回调函数
static void notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
    if (length >= 2) {
        heart_rate = pData[1];
        Serial.printf(">>> 【实时心率】: %d bpm\n", heart_rate);
    }
}

// 扫描到手表后的连接函数
bool connectToServer() {
    Serial.println("正在向手表发起物理连接...");
    BLEClient* pClient = BLEDevice::createClient();
    
    // 动态连接，自动匹配Public/Random地址类型
    if (!pClient->connect(myDevice)) return false; 
    Serial.println("物理层连接成功，正在交换MTU...");
    pClient->setMTU(517); 

    Serial.println("正在获取心率服务...");
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) { pClient->disconnect(); return false; }

    Serial.println("正在获取心率特征值...");
    BLERemoteCharacteristic* pRemoteChar = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteChar == nullptr) { pClient->disconnect(); return false; }

    if(pRemoteChar->canNotify()) {
        pRemoteChar->registerForNotify(notifyCallback);
        Serial.println("【成功】已成功订阅手表心率数据流！");
        isConnected = true;
        return true;
    }
    return false;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.isAdvertisingService(serviceUUID)) {
            Serial.println("发现了支持心率广播的手表，立即停止扫描...");
            BLEDevice::getScan()->stop(); // 关键：释放射频资源
            delay(100);
            myDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
        }
    }
};

void setup() {
    //SET_PERI_REG_BITS(RTC_CNTL_BROWN_OUT_REG, RTC_CNTL_BROWN_OUT_RST_ENA, 0, RTC_CNTL_BROWN_OUT_RST_ENA_S);
    
    // 延迟一段时间让电源稳定
    delay(2000);
    Serial.begin(115200);
    Serial.println("=== 警告：硬件 BOD 已被强制彻底关闭！ ===");
    
    Serial.println("=== 阶段 2 测试：精准心率获取模式 ===");

    BLEDevice::init("S3_HR_NODE");
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_N9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_N9);
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(0, false); // 持续扫描直到锁定目标
}

void loop() {
    if (doConnect) {
        Serial.println("【时序优化】发现目标，已强制关闭扫描...");
        BLEDevice::getScan()->stop(); 
        
        // 强行静默 1 秒钟！让芯片的射频电路彻底冷却、电压回稳
        delay(1000); 
        
        Serial.println("【时序优化】静默结束，开始温柔尝试连接...");
        if (connectToServer()) {
            Serial.println("====================================");
            Serial.println("连接成功！");
            Serial.println("====================================");
        } else {
            Serial.println("连接失败，准备重试...");
            delay(3000);
            BLEDevice::getScan()->start(0, false);
        }
        doConnect = false;
    }
    delay(10);
}