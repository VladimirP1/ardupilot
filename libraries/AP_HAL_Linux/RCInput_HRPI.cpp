#include <AP_HAL/AP_HAL.h>

#if CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_HRPI

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "RCInput_HRPI.h"

#include "px4io_protocol.h"

static const AP_HAL::HAL& hal = AP_HAL::get_HAL();

using namespace Linux;

void RCInput_HRPI::init()
{
    _i2c_sem = hal.i2c->get_semaphore();

    if (_i2c_sem == NULL) {
        AP_HAL::panic("PANIC: RCInput_HRPI could not get a "
                                  "valid SPI semaphore!");
        return; // never reached
    }

    // start the timer process to read samples
    hal.scheduler->register_timer_process(FUNCTOR_BIND_MEMBER(&RCInput_HRPI::_poll_data, void));
}

void RCInput_HRPI::_poll_data(void)
{
    // Throttle read rate to 100hz maximum.
    if (AP_HAL::micros() - _last_timer < 10000) {
        return;
    }

    _last_timer = AP_HAL::micros();

    if (!_i2c_sem->take_nonblocking()) {
        return;
    }

    static int    i = 0;
    i = (i + 1) % 8;
    uint8_t    a,b;
    if (hal.i2c->readRegisters (8, (i * 2) + 0xA, 1, &a) != 0)
    {
        _i2c_sem->give ();
        return;
    }
    if (hal.i2c->readRegisters (8, (i * 2) + 0xA + 1, 1, &b) != 0)
    {
       _i2c_sem->give ();
        return;
    }
    _chann[i] = (a | (b << 8));

    _i2c_sem->give();
    _update_periods(&_chann[0], (uint8_t)8);

}

#endif // CONFIG_HAL_BOARD_SUBTYPE
