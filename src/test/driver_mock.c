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
#include "driver_mock.h"
#include "motoilet_whisper.h"
#include "motoilet_whisper_data_layer.h"
#include "motoilet_whisper_driver.h"

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

data_buf_t _mock_buf_send;
set_delay_buf_t _mock_set_delay_invocations;
size_t _mock_cancel_delay_invocations;

void data_layer_driver_mock_init(void)
{
    _mock_buf_send.size = 0;
    _mock_set_delay_invocations.size = 0;
    _mock_cancel_delay_invocations = 0;

}

void motoilet_whisper_driver__write(const unsigned char *buf, unsigned char len)
{
    assert(_mock_buf_send.size + len <= BUF_SIZE);

    memcpy(&_mock_buf_send.buf[_mock_buf_send.size], buf, len);
    _mock_buf_send.size += len;
}

void motoilet_whisper_driver__set_delay(unsigned short delay_in_ms)
{
    assert(_mock_set_delay_invocations.size <= BUF_SIZE);
    _mock_set_delay_invocations.buf[_mock_set_delay_invocations.size++] = delay_in_ms;
}

/**
 * @brief cancel the timer previously set by motoilet_whisper_data__set_delay, should be implmeneted by the link layer
 */
void motoilet_whisper_driver__cancel_delay(void)
{
    ++_mock_cancel_delay_invocations;
}