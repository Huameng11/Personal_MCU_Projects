#include "BLE_Handler.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>

// 引入底层电源与频率管理
#include "esp32-hal-cpu.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// 全局变量定义
int g_heart_rate = 0;
static boolean doConnect = false;
static boolean ble_isConnected = false;
static BLEAdvertisedDevice* myDevice = nullptr;

// 蓝牙标准服务UUID
static BLEUUID serviceUUID("180d"); // 心率服务UUID
static BLEUUID    charUUID("2a37"); // 心率特征值UUID

// 接收到手表心率推送的回调函数
static void notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
    if (length >= 2) {
        g_heart_rate = pData[1];
        //Serial.printf(">>> 【实时心率】: %d bpm\n", g_heart_rate);
    }
}

// 客户端连接状态回调
class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
        ble_isConnected = true;
    }
    void onDisconnect(BLEClient* pclient) {
        ble_isConnected = false;
        g_heart_rate = 0; // 断开连接后心率清零
        Serial.println(">>> 【提示】手表断开连接，3秒后重新开启扫描...");
        delay(3000);
        BLEDevice::getScan()->start(0, false);
    }
};

// 扫描回调类
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.isAdvertisingService(serviceUUID)) {
            Serial.println("发现了支持心率广播的手表，立即停止扫描...");
            BLEDevice::getScan()->stop(); // 关键：释放射频资源
            
            if (myDevice != nullptr) {
                delete myDevice;
            }
            myDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
        }
    }
};

// 内部核心连接函数
bool connectToServer() {
    Serial.println("正在向手表发起物理连接...");
    BLEClient* pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback());
    
    // 动态连接
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
        return true;
    }
    return false;
}

// 构造与析构
BLE_Handler::BLE_Handler() {}
BLE_Handler::~BLE_Handler() {
    if (myDevice != nullptr) {
        delete myDevice;
    }
}

// 初始化函数
void BLE_Handler::init() {
    // 留出2秒电源回稳时间
    delay(2000); 
    
    // 如果你发现拔掉电容或冷机启动依然偶发BOD，可以取消下面这行注释（闭锁BOD）
    // SET_PERI_REG_BITS(RTC_CNTL_BROWN_OUT_REG, RTC_CNTL_BROWN_OUT_RST_ENA, 0, RTC_CNTL_BROWN_OUT_RST_ENA_S);
    
    // 降频到 160MHz 运行，平衡蓝牙算力和功耗
    setCpuFrequencyMhz(160); 

    Serial.println("=== 蓝牙心率模块初始化 ===");
    BLEDevice::init("BikeDash_S3_Host");
    
    // 强制全局限制蓝牙发射功率，砍掉尖峰电流
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_N9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_N9);

    // 配置扫描参数
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    
    Serial.println("开始初始扫描，等待手表广播...");
    pBLEScan->start(0, false); 
}

// 状态机轮询
void BLE_Handler::handle() {
    if (doConnect) {
        Serial.println("【时序优化】发现目标，已强制关闭扫描...");
        BLEDevice::getScan()->stop(); 
        
        // 强行静默 1 秒钟！让芯片的射频电路彻底冷却、电压回稳
        delay(1000); 
        
        Serial.println("【时序优化】静默结束，开始温柔尝试连接...");
        if (connectToServer()) {
            Serial.println("====================================");
            Serial.println("连接成功！码表正在持续接收心率...");
            Serial.println("====================================");
        } else {
            Serial.println("连接失败，3秒后重新开启扫描...");
            delay(3000);
            BLEDevice::getScan()->start(0, false);
        }
        doConnect = false;
    }
}

int BLE_Handler::getHeartRate() {
    return g_heart_rate;
}

bool BLE_Handler::isConnected() {
    return ble_isConnected;
}