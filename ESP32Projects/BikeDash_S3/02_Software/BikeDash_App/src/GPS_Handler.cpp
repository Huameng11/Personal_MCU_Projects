#include "GPS_Handler.h"

GPS_Handler::GPS_Handler() {
  gpsSerial = &Serial1;
  rawBuffer = "";
  latitudeStr = "0.0";
  longitudeStr = "0.0";
  isFixed = false;
}

void GPS_Handler::init() {
  gpsSerial->begin(baudRate, SERIAL_8N1, rxPin, txPin);
  Serial.println(">>> [GPS] 硬件串口1初始化成功 (RX:18, TX:17)！");
}

bool GPS_Handler::readRawLine(String &line) {
  while (gpsSerial->available() > 0) {
    char c = gpsSerial->read();
    if (c == '\n') {
      rawBuffer.trim();
      if (rawBuffer.length() > 0) {
        line = rawBuffer;
        rawBuffer = "";
        return true;
      }
    } else {
      if (c >= 32 && c <= 126) {
        rawBuffer += c;
      }
    }
    if (rawBuffer.length() > 120) { rawBuffer = ""; }
  }
  return false;
}

// [新增] 基于高效相对索引的字符串逗号切片函数
String GPS_Handler::getField(const String &src, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = src.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (src.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? src.substring(strIndex[0], strIndex[1]) : "";
}

// [新增] 原生高可靠性 NMEA 语句拆解器
void GPS_Handler::parseNmealine(const String &line) {
  // 我们只盯着推荐定位最小数据集 $GNRMC 或 $GPRMC
  if (line.startsWith("$GNRMC") || line.startsWith("$GPRMC")) {
    // 1. 检查定位状态位 (第 2 个字段)
    String status = getField(line, ',', 2);

    if (status == "A") {
      isFixed = true;

      // 2. 提取原始经纬度字段
      String rawLat = getField(line, ',', 3);  // 格式如 2234.5678
      String latDir = getField(line, ',', 4);  // N 或 S
      String rawLon = getField(line, ',', 5);  // 格式如 11356.7890
      String lonDir = getField(line, ',', 6);  // E 或 W

      // 3. 简单的格式微调（如果你想直接看原始度分格式，直接拼接方向即可）
      // 找到 src/GPS_Handler.cpp 中的 parseNmealine 函数，修改其内部的拼接逻辑：

      if (rawLat.length() > 0 && rawLon.length() > 0) {
        // ------------------------------------------------------------------------
        // 【纬度格式化】 原始如 "2234.5678" -> 拆出前2位为度，后面为分
        // ------------------------------------------------------------------------
        String latDeg = rawLat.substring(0, 2);  // 截取前 2 位："22"
        String latMin = rawLat.substring(2);     // 截取第 2 位到最后："34.5678"

        // 为了防止屏幕显示太长溢出圆形边界，把“分”的小数点后保留 2 位即可（如 "34.56"）
        if (latMin.indexOf('.') != -1) {
          int dotIdx = latMin.indexOf('.');
          latMin = latMin.substring(0, dotIdx + 3);
        }

        // ------------------------------------------------------------------------
        // 【经度格式化】 原始如 "11356.7890" -> 拆出前3位为度，后面为分
        // ------------------------------------------------------------------------
        String lonDeg = rawLon.substring(0, 3);  // 截取前 3 位："113"
        String lonMin = rawLon.substring(3);     // 截取第 3 位到最后："56.7890"

        if (lonMin.indexOf('.') != -1) {
          int dotIdx = lonMin.indexOf('.');
          lonMin = lonMin.substring(0, dotIdx + 3);
        }

        // ------------------------------------------------------------------------
        // 【优雅拼接】 加上方向和标准的度(°)分(')符号
        // ------------------------------------------------------------------------
        latitudeStr = latDir + ": " + latDeg + "deg " + latMin + "'";   // 示例: N: 22deg 34.56'
        longitudeStr = lonDir + ": " + lonDeg + "deg " + lonMin + "'";  // 示例: E: 113deg 56.78'
      }
    } else {
      // 在室内或未定位时，状态为 'V'
      isFixed = false;
    }
  }
}