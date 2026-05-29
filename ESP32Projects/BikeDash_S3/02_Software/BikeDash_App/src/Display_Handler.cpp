#include "Display_Handler.h"

Display_Handler::Display_Handler() : canvas(nullptr) {}

void Display_Handler::init() {
    lcd.init();
    lcd.setRotation(0);
    
    // 强制把物理屏幕底盘彻底刷黑
    lcd.fillScreen(TFT_BLACK); 
    lcd.clear(TFT_BLACK);

    canvas = new LGFX_Sprite(&lcd);
    
    // ============================================================================
    // 【核心修正 1】升级为 16 位全彩色深 (RGB565)
    // 消除 8 位色深带来的颜色映射偏置（暗灰变纯黑）
    // ============================================================================
    canvas->setColorDepth(16); 
    
    if (canvas->createSprite(240, 240) == nullptr) {
        Serial.println("❌ ERROR: 16位全彩画布内存分配失败！");
        while(1) delay(1000);
    }
    
    // 确保画布初始化时就是绝对黑暗
    canvas->fillSprite(TFT_BLACK);
    canvas->clear(TFT_BLACK);
    canvas->setTextDatum(MC_DATUM); 
}

void Display_Handler::drawHeartRateZoneArc() {
    // 维持饱满实体的彩虹圆环
    canvas->fillArc(120, 120, 110, 118, 135, 180, TFT_BLUE);
    canvas->fillArc(120, 120, 110, 118, 180, 240, TFT_GREEN);
    canvas->fillArc(120, 120, 110, 118, 240, 300, TFT_YELLOW);
    canvas->fillArc(120, 120, 110, 118, 300, 360, TFT_ORANGE);
    canvas->fillArc(120, 120, 110, 118, 0,   45,  TFT_RED);
}

void Display_Handler::update(int heartRate, bool bleConnected, float speed, float distance, String rTime, String rClock, bool gpsFixed) {
    if (canvas == nullptr) return;

    // ============================================================================
    // 【核心修改 2】16位标准强制双清屏，彻底杜绝灰色残留
    // ============================================================================
    canvas->fillSprite(TFT_BLACK);  
    canvas->clear(TFT_BLACK);       

    // 渲染饱满粗体的环
    drawHeartRateZoneArc();

    // ============================================================================
    // 【第一层：顶部心率区】
    // ============================================================================
    uint16_t statusBg = bleConnected ? lcd.color565(0, 120, 255) : TFT_DARKGREY;
    canvas->fillRoundRect(95, 22, 50, 14, 7, statusBg); 
    canvas->setTextColor(TFT_WHITE, statusBg); 
    canvas->setFont(&fonts::Font0); 
    canvas->drawString(bleConnected ? "H-RATE" : "DISCONN", 120, 29);

    // 心率数字
    canvas->setFont(&fonts::Font7); 
    if (bleConnected) {
        canvas->setTextColor(lcd.color565(100, 180, 255), TFT_BLACK); 
        canvas->drawNumber(heartRate, 120, 60);
    } else {
        canvas->setTextColor(TFT_LIGHTGRAY, TFT_BLACK);
        canvas->drawString("---", 120, 60);
    }
    
    canvas->setTextColor(TFT_DARKGREY, TFT_BLACK);
    canvas->setFont(&fonts::Font0);
    canvas->drawString("bpm", 120, 85);

    // ============================================================================
    // 【第二层：中部分割区 - 速度与距离大字】
    // ============================================================================
    canvas->drawFastVLine(120, 95, 50, lcd.color565(60, 60, 60)); 
    canvas->drawFastHLine(25, 122, 40, lcd.color565(40, 40, 40));  
    canvas->drawFastHLine(175, 122, 40, lcd.color565(40, 40, 40)); 

    // 速度大字 (Font4 + 1.2倍等比放大)
    canvas->setTextColor(TFT_WHITE, TFT_BLACK);
    canvas->setFont(&fonts::Font4); 
    canvas->setTextSize(1.2, 1.2); 
    char speedBuf[8];
    sprintf(speedBuf, "%.1f", speed); 
    canvas->drawString(speedBuf, 72, 112); 
    canvas->setTextSize(1, 1);     

    canvas->setTextColor(TFT_LIGHTGRAY, TFT_BLACK); 
    canvas->setFont(&fonts::Font0);
    canvas->drawString("km/h", 72, 134);

    // 距离大字 (Font4 + 1.2倍等比放大)
    canvas->setTextColor(TFT_WHITE, TFT_BLACK);
    canvas->setFont(&fonts::Font4);
    canvas->setTextSize(1.2, 1.2); 
    char distBuf[8];
    sprintf(distBuf, "%.2f", distance);
    canvas->drawString(distBuf, 168, 112); 
    canvas->setTextSize(1, 1);     

    canvas->setTextColor(TFT_LIGHTGRAY, TFT_BLACK);
    canvas->setFont(&fonts::Font0);
    canvas->drawString("km", 168, 134);

    // ============================================================================
    // 【第三层：大字骑行时间区】
    // ============================================================================
    canvas->drawFastHLine(40, 148, 160, lcd.color565(50, 50, 50)); 

    // 骑行时间大字 (Font6 + 1.1倍放大)
    canvas->setTextColor(TFT_GREEN, TFT_BLACK); 
    canvas->setFont(&fonts::Font6); 
    canvas->setTextSize(1.1, 1.1);
    canvas->drawString(rTime, 120, 172);
    canvas->setTextSize(1, 1);     

    // ============================================================================
    // 【第四层：底部系统时钟】
    // ============================================================================
    canvas->setTextColor(TFT_SILVER, TFT_BLACK);
    canvas->setFont(&fonts::Font2); 
    canvas->drawString(rClock, 120, 210);

    // GPS 状态卫星灯
    if (gpsFixed) {
        canvas->fillCircle(120, 226, 3, TFT_GREEN);
    } else {
        if ((millis() / 500) % 2 == 0) {
            canvas->fillCircle(120, 226, 3, TFT_RED);
        }
    }

    // 6. 瞬间推送显存到 GC9A01 物理屏
    canvas->pushSprite(0, 0);
}