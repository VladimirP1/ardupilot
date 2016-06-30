#pragma once

#include "AP_HAL_Linux.h"
#include "RCInput.h"

class Linux::RCInput_HRPI : public Linux::RCInput
{
public:
    void init();
    
private:
    uint32_t _last_timer;
    uint16_t
    _chann[12];

    AP_HAL::Semaphore *_i2c_sem;
    
    void _poll_data(void);
};
