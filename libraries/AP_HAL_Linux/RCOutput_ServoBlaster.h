#pragma once

#include "AP_HAL_Linux.h"
#include "PWM_Sysfs.h"

class Linux::RCOutput_ServoBlaster : public AP_HAL::RCOutput {
public:
    RCOutput_ServoBlaster(uint8_t chip, uint8_t channel_count);
    ~RCOutput_ServoBlaster();

    static RCOutput_ServoBlaster *from(AP_HAL::RCOutput *rcoutput)
    {
        return static_cast<RCOutput_ServoBlaster *>(rcoutput);
    }
    int fd;
    void init();
    void set_freq(uint32_t chmask, uint16_t freq_hz);
    uint16_t get_freq(uint8_t ch);
    void enable_ch(uint8_t ch);
    void disable_ch(uint8_t ch);
    void write(uint8_t ch, uint16_t period_us);
    uint16_t read(uint8_t ch);
    void read(uint16_t *period_us, uint8_t len);

    struct chann{
        uint16_t val;
        bool enabled;
        uint16_t freq_hz;
    };
    chann ch[8];
private:
    const uint8_t _chip;
    const uint8_t _channel_count;
};
