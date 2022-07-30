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
#ifndef DRIVER_MOCK_H
#define DRIVER_MOCK_H

#include "motoilet_whisper_driver.h"
#include "motoilet_whisper_data_layer.h"

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define BUF_SIZE 10240

typedef struct 
{
    size_t size;
    uint8_t buf[BUF_SIZE]; 
} data_buf_t;

typedef struct
{
    size_t size;
    uint16_t buf[BUF_SIZE];
} set_delay_buf_t;


extern data_buf_t _mock_buf_send;
extern set_delay_buf_t _mock_set_delay_invocations;
extern size_t _mock_cancel_delay_invocations;

void data_layer_driver_mock_init(void);

#endif // DRIVER_MOCK_H