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
#include "data_layer_mock.h"

_mock_send_buf_t _mock_send_buf;

unsigned char motoilet_whisper_data__init(void)
{
    _mock_send_buf.size = 0;
    return 0;
}

void mock_data_layer_teardown(void)
{
    int i = 0;
    for(i = 0; i < _mock_send_buf.size; i++)
    {
        free((void *)_mock_send_buf.buf[i].payload);
    }
    _mock_send_buf.size = 0;
}

/**
 * @brief send out a whisper message.
 *
 * @param message the message to send.
 * @return unsigned char 0 if successful, 1 if the previous message is still under processing.
 */
unsigned char motoilet_whisper_data__send(const struct whisper_message *message)
{
    uint8_t *payload = malloc(MOTOILET_WHISPER_MESSAGE_PAYLOAD_LEN);
    memcpy(payload, message->payload, MOTOILET_WHISPER_MESSAGE_PAYLOAD_LEN);

    struct whisper_message *slot = &_mock_send_buf.buf[_mock_send_buf.size++];
    memcpy(slot, message, sizeof(struct whisper_message));
    slot->payload = payload;

    return 0;
}