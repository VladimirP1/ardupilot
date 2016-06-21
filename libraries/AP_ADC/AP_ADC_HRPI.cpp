#include <AP_HAL/AP_HAL.h>

#include "AP_ADC_HRPI.h"
#include <stdio.h>
#include <unistd.h>
#define HRPI_DEFAULT_ADDRESS     0x8

#define HRPI_DEBUG 0
#if HRPI_DEBUG
#include <cstdio>
#define debug(fmt, args ...)  do {hal.console->printf("%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ## args); } while(0)
#define error(fmt, args ...)  do {fprintf(stderr,"%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ## args); } while(0)
#else
#define debug(fmt, args ...)
#define error(fmt, args ...)
#endif

extern const AP_HAL::HAL &hal;

#define HRPI_CHANNELS_COUNT           4

const uint8_t AP_ADC_HRPI::_channels_number  = HRPI_CHANNELS_COUNT;


AP_ADC_HRPI::AP_ADC_HRPI():
    _i2c_sem(NULL),
    _channel_to_read(0)
{
    _samples = new adc_report_s[_channels_number];
}

bool AP_ADC_HRPI::init()
{

    hal.scheduler->suspend_timer_procs();
    _i2c_sem = hal.i2c->get_semaphore();
    if(!_i2c_sem->take(500))
        return false;
    uint8_t config=0xf;
    if (hal.i2c->writeRegisters((uint8_t)HRPI_DEFAULT_ADDRESS, 0x9, 1, (uint8_t *) &config) != 0) {
        _i2c_sem->give();
        return false;
    }
    hal.scheduler->register_timer_process(FUNCTOR_BIND_MEMBER(&AP_ADC_HRPI::_update, void));
    hal.scheduler->resume_timer_procs();
    _i2c_sem->give();
    return true;
}

bool AP_ADC_HRPI::_start_conversion(uint8_t channel)
{
    return true;
}

size_t AP_ADC_HRPI::read(adc_report_s *report, size_t length) const
{
    for (size_t i = 0; i < length; i++) {
        report[i].data = _samples[i].data;
        report[i].id = _samples[i].id;
    }

    return length;
}

float AP_ADC_HRPI::_convert_register_data_to_mv(int16_t word) const
{

    float pga=5000.0/1024.0;
    return (float) word * pga;
}

void AP_ADC_HRPI::_update()
{
    /* TODO: make update rate configurable */
    if (AP_HAL::micros() - _last_update_timestamp < 50000) {
        return;
    }

    uint8_t a,b;
    if (!_i2c_sem->take_nonblocking()) {
        return;
    }

    if ( hal.i2c->readRegisters(HRPI_DEFAULT_ADDRESS, 0x1+(_channel_to_read*2), 1, &a) != 0 ) {
        _i2c_sem->give();
        return;
    }
    if ( hal.i2c->readRegisters(HRPI_DEFAULT_ADDRESS, 0x2+(_channel_to_read*2), 1, &b) != 0 ) {
        _i2c_sem->give();
        return;
    }

    float sample = _convert_register_data_to_mv((a & 0xff) | ((b & 0xff) << 8));
    _samples[_channel_to_read].data = sample;
    _samples[_channel_to_read].id = _channel_to_read;
//if(_channel_to_read==1)
    printf("CH%d\t%f\n",_channel_to_read,sample);
    _channel_to_read = (_channel_to_read + 1) % _channels_number;

    _i2c_sem->give();
    _last_update_timestamp = AP_HAL::micros();
}
