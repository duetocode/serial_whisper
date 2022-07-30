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
#ifndef MOTOILET_WHISPER_H
#define MOTOILET_WHISPER_H

#define MOTOILET_WHISPER_MESSAGE_PAYLOAD_LEN 8
#include "motoilet_whisper_message.h"


unsigned char motoilet_whisper__setup(const unsigned char buf[MOTOILET_WHISPER_MESSAGE_PAYLOAD_LEN]);

unsigned char motoilet_whisper__report_state(const unsigned char buf[MOTOILET_WHISPER_MESSAGE_PAYLOAD_LEN]);

unsigned char motoilet_whisper__pause(void);
unsigned char motoilet_whisper__resume(void);
unsigned char motoilet_whisper__reset(void);
unsigned char motoilet_whisper__reboot(void);

void motoilet_whisper__state_sync_cb(const unsigned char *buf, unsigned char len);

/**
 * @brief Intialize the motoilet whisper component and its underlying components.
 * 
 * @return char 0 if successful, 1 otherwise.
 */
unsigned char motoilet_whisper__init(void);

#endif // MOTOILET_WHISPER_H