#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Arduino.h>
#include "LovyanGFX.hpp" // 使用本地嵌套路径

// ============================================================================
// LovyanGFX 硬件配置类
// ============================================================================
class LGFX_GC9A01 : public lgfx::LGFX_Device {
    lgfx::Panel_GC9A01      _panel_instance;
    lgfx::Bus_SPI           _bus_instance;

public:
    LGFX_GC9A01() {
        auto cfg = _bus_instance.config();
        cfg.spi_host = SPI2_HOST;     
        cfg.spi_mode = 0;
        cfg.freq_write = 40000000;    
        cfg.freq_read  = 16000000;
        cfg.pin_sclk = 12;            // SCLK
        cfg.pin_mosi = 11;            // MOSI
        cfg.pin_miso = -1;
        cfg.pin_dc   = 4;             // 安全引脚 DC
        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);

        auto p_cfg = _panel_instance.config();
        p_cfg.pin_rst     = 5;        // 安全引脚 RST
        p_cfg.pin_cs      = 13;       // CS
        //p_cfg.pin_bl      = -1;       
        p_cfg.panel_width  = 240;
        p_cfg.panel_height = 240;
        p_cfg.offset_x     = 0;
        p_cfg.offset_y     = 0;
        _panel_instance.config(p_cfg);

        setPanel(&_panel_instance);
    }
};

// ============================================================================
// 码表高仿UI渲染器
// ============================================================================
class Display_Handler {
private:
    LGFX_GC9A01 lcd;            
    LGFX_Sprite* canvas;        

    // 内部UI辅助绘制函数：画分段彩色环形边缘
    void drawHeartRateZoneArc();

public:
    Display_Handler();
    void init();
    
    /**
     * @brief 完美复刻手表排版的屏幕刷新函数
     * @param heartRate 实时心率 (bpm)
     * @param bleConnected 蓝牙连接状态
     * @param speed 实时车速 (km/h)
     * @param distance 累计里程 (km)
     * @param rTime 骑行时间 (格式如 "00:00:07")
     * @param rClock 当前时间 (格式如 "19:11")
     * @param gpsFixed GPS是否定位成功
     */
    void update(int heartRate, bool bleConnected, float speed, float distance, String rTime, String rClock, bool gpsFixed);
};

#endif