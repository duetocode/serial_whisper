/* MIT License
 * 
 * Copyright (c) 2022 Ningbo Peakhonor Technology Co., Limited
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */ 
#ifndef APP_LAYER_H
#define APP_LAYER_H

#include "motoilet_whisper.h"

#define WHISPER_MESSAGE_TYPE__STATE_QUERY 0x01
#define WHISPER_MESSAGE_TYPE__STATE_UPDATE 0x02
#define WHISPER_MESSAGE_TYPE__STATE_SYNC 0x03
#define WHISPER_MESSAGE_TYPE__STATE_UPDATE_CONFIG 0x09

#define WHISPER_MESSAGE_TYPE__PAUSE 0x10
#define WHISPER_MESSAGE_TYPE__RESUME 0x11
#define WHISPER_MESSAGE_TYPE__REBOOT 0x12
#define WHISPER_MESSAGE_TYPE__RESET 0x13
#define WHISPER_MESSAGE_TYPE__OTA 0x14

#define WHISPER_MESSAGE_TYPE__HANDSHAKE 0xFF

#define WHISPER_MESSAGE_STATE_UPDATE_TYPE__SEAT_HEATING_TEMP 0x00
#define WHISPER_MESSAGE_STATE_UPDATE_TYPE__WATER_HEATING_TEMP 0x01
#define WHISPER_MESSAGE_STATE_UPDATE_TYPE__LAMP_STATE 0x02
#define WHISPER_MESSAGE_STATE_UPDATE_TYPE__CLEANING_STATE 0x03
#define WHISPER_MESSAGE_STATE_UPDATE_TYPE__MODULE_STATE 0x04
#define WHISPER_MESSAGE_STATE_UPDATE_TYPE__SEAT_SENSOR_STATE 0x05
#define WHISPER_MESSAGE_STATE_UPDATE_TYPE__COVER_STATE 0x06
#define WHISPER_MESSAGE_STATE_UPDATE_TYPE__SEAT_STATE 0x07

#endif  // APP_LAYER_H