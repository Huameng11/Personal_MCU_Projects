#include "BLE_Handler.h"
#include "Config.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>

int current_heart_rate = 0;
bool is_watch_connected = false;
static bool doConnect = false;
static BLEAdvertisedDevice* targetDevice;
static BLEUUID serviceUUID(WATCH_SERVICE_UUID);
static BLEUUID charUUID(WATCH_CHAR_UUID);

// 心率数据解析
static void notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
    if (length >= 2) {
        current_heart_rate = pData[1];
    }
}

// 扫描回调类
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.isAdvertisingService(serviceUUID)) {
            BLEDevice::getScan()->stop();
            targetDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
        }
    }
};

void init_ble_service() {
    BLEDevice::init(DEVICE_NAME);
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->start(0, false);
}

void update_ble_loop() {
    if (doConnect) {
        BLEClient* pClient = BLEDevice::createClient();
        if (pClient->connect(targetDevice)) {
            pClient->setMTU(517);
            BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
            if (pRemoteService != nullptr) {
                BLERemoteCharacteristic* pRemoteChar = pRemoteService->getCharacteristic(charUUID);
                if (pRemoteChar != nullptr && pRemoteChar->canNotify()) {
                    pRemoteChar->registerForNotify(notifyCallback);
                    is_watch_connected = true;
                }
            }
        }
        doConnect = false;
    }
}