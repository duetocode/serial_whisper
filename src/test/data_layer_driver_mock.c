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
#include "data_layer_driver_mock.h"
#include "motoilet_whisper.h"
#include "motoilet_whisper_data_layer.h"
#include "motoilet_whisper_driver.h"

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

received_cb_buf_t _mock_received_cb_buf;
delivery_cb_buf_t _mock_delivery_cb_buf;
data_buf_t _mock_buf_send;
set_delay_buf_t _mock_set_delay_invocations;
size_t _mock_cancel_delay_invocations;

void data_layer_driver_mock_init(void)
{
    _mock_buf_send.size = 0;
    _mock_set_delay_invocations.size = 0;
    _mock_cancel_delay_invocations = 0;
    _mock_delivery_cb_buf.size = 0;

    size_t i = 0;
    for(i = 0; i < _mock_received_cb_buf.size; i++)
    {
        free((void *)_mock_received_cb_buf.buf[i].payload);
    }
    _mock_received_cb_buf.size = 0;
}


void motoilet_whisper_message__received_cb(struct whisper_message *message)
{
    struct whisper_message *r = &_mock_received_cb_buf.buf[_mock_received_cb_buf.size++];
    memcpy(r, message, sizeof(struct whisper_message));
    uint8_t *payload = malloc(MOTOILET_WHISPER_MESSAGE_PAYLOAD_LEN);
    r->payload = payload;
    memcpy(payload, message->payload, MOTOILET_WHISPER_MESSAGE_PAYLOAD_LEN);
}

void motoilet_whisper_message__delivery_cb(unsigned char delivered)
{
    _mock_delivery_cb_buf.buf[_mock_delivery_cb_buf.size++] = delivered;
}


void motoilet_whisper_data__write(const unsigned char *buf, unsigned char len)
{
    assert(_mock_buf_send.size + len <= BUF_SIZE);

    memcpy(&_mock_buf_send.buf[_mock_buf_send.size], buf, len);
    _mock_buf_send.size += len;
}

void motoilet_whisper_data__set_delay(unsigned short delay_in_ms)
{
    assert(_mock_set_delay_invocations.size <= BUF_SIZE);
    _mock_set_delay_invocations.buf[_mock_set_delay_invocations.size++] = delay_in_ms;
}

/**
 * @brief cancel the timer previously set by motoilet_whisper_data__set_delay, should be implmeneted by the link layer
 */
void motoilet_whisper_data__cancel_delay(void)
{
    ++_mock_cancel_delay_invocations;
}