/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
 * Copyright (C) 2015  Intel Corporation. All rights reserved.
 *
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "RCOutput_ServoBlaster.h"

#include <AP_Common/AP_Common.h>
#include <AP_HAL/AP_HAL.h>
#include <AP_Math/AP_Math.h>
#include <fcntl.h>
#include <stdio.h>
namespace Linux {

RCOutput_ServoBlaster::RCOutput_ServoBlaster(uint8_t chip, uint8_t channel_count)
    : _chip(chip)
    , _channel_count(channel_count)
{
}

RCOutput_ServoBlaster::~RCOutput_ServoBlaster()
{
    system("killall servod");
}

void RCOutput_ServoBlaster::init()
{
    system("killall servod");
    system("servod --step-size=2us --p1pins=7,11,13,15,29,31,33,35,37,40,36,26,24");
    fd=::open("/dev/servoblaster", O_RDWR | O_CLOEXEC);
}

void RCOutput_ServoBlaster::set_freq(uint32_t chmask, uint16_t freq_hz)
{
    for (uint8_t i = 0; i < _channel_count; i++) {
        if (chmask & 1 << i) {
            ch[i].freq_hz=freq_hz;
        }
    }
}

uint16_t RCOutput_ServoBlaster::get_freq(uint8_t chan)
{
    if (chan >= _channel_count) {
        return 0;
    }

    return ch[chan].freq_hz;
}

void RCOutput_ServoBlaster::enable_ch(uint8_t chan)
{
    if (chan >= _channel_count) {
        return;
    }

    ch[chan].enabled=true;
}

void RCOutput_ServoBlaster::disable_ch(uint8_t chan)
{
    if (chan >= _channel_count) {
        return;
    }

    ch[chan].enabled=false;
}

void RCOutput_ServoBlaster::write(uint8_t chan, uint16_t period_us)
{
    if (chan >= _channel_count) {
        return;
    }
    ch[chan].val=period_us;
    dprintf(fd,"%u=%uus\n",chan,period_us);

}

uint16_t RCOutput_ServoBlaster::read(uint8_t chan)
{
    if (chan >= _channel_count) {
        return 0;
    }

    return ch[chan].val;
}

void RCOutput_ServoBlaster::read(uint16_t *period_us, uint8_t len)
{
    for (int i = 0; i < len; i++) {
        period_us[i] = read(i);
    }
}
}
