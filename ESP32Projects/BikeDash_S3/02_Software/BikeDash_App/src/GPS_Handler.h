#ifndef GPS_HANDLER_H
#define GPS_HANDLER_H

#include <Arduino.h>

class GPS_Handler {
private:
    HardwareSerial* gpsSerial;
    const int rxPin = 18;
    const int txPin = 17;
    const uint32_t baudRate = 9600;
    String rawBuffer;

    // 内部切片辅助工具：获取第 index 个逗号分割的子串
    String getField(const String &src, char separator, int index);

public:
    GPS_Handler();
    void init();
    bool readRawLine(String &line);

    // [新增] 供外部模块直接读取的经纬度字符串
    String latitudeStr;
    String longitudeStr;
    bool isFixed; // 是否定位成功

    // [新增] 解析 NMEA 核心函数
    void parseNmealine(const String &line);
};

#endif