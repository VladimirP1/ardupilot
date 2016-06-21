#include "AnalogIn_HRPI.h"

AnalogSource_HRPI::AnalogSource_HRPI(int16_t pin):
    _pin(pin),
    _value(0.0f)
{
}

void AnalogSource_HRPI::set_pin(uint8_t pin)
{
    if (_pin == pin) {
        return;
    }
    _pin = pin;
}

float AnalogSource_HRPI::read_average()
{
    return read_latest();
}

float AnalogSource_HRPI::read_latest()
{
    return _value;
}

float AnalogSource_HRPI::voltage_average()
{
    return _value;
}

float AnalogSource_HRPI::voltage_latest()
{
    return _value;
}

float AnalogSource_HRPI::voltage_average_ratiometric()
{
    return _value;
}

extern const AP_HAL::HAL &hal;

AnalogIn_HRPI::AnalogIn_HRPI()
{
    _adc = new AP_ADC_HRPI();
    _channels_number = _adc->get_channels_number();
}

AP_HAL::AnalogSource* AnalogIn_HRPI::channel(int16_t pin)
{
    for (uint8_t j = 0; j < _channels_number; j++) {
        if (_channels[j] == NULL) {
            _channels[j] = new AnalogSource_HRPI(pin);
            return _channels[j];
        }
    }

    hal.console->println("Out of analog channels");
    return NULL;
}

void AnalogIn_HRPI::init()
{
    _adc->init();

    hal.scheduler->suspend_timer_procs();
    hal.scheduler->register_timer_process(FUNCTOR_BIND_MEMBER(&AnalogIn_HRPI::_update, void));
    hal.scheduler->resume_timer_procs();
}

void AnalogIn_HRPI::_update()
{
    if (AP_HAL::micros() - _last_update_timestamp < 100000) {
        return;
    }

    adc_report_s reports[HRPI_ADC_MAX_CHANNELS];

    size_t rc = _adc->read(reports, HRPI_ADC_MAX_CHANNELS);

    for (size_t i = 0; i < rc; i++) {
        for (uint8_t j=0; j < rc; j++) {
            AnalogSource_HRPI *source = _channels[j];

            if (source != NULL && reports[i].id == source->_pin) {
                source->_value = reports[i].data / 1000;
            }
        }
    }

    _last_update_timestamp = AP_HAL::micros();
}
