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
#include <unity.h>

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "motoilet_whisper.h"
#include "motoilet_whisper_transmission_layer.h"
#include "app_layer.h"

struct {
    size_t size;
    struct whisper_message buf[1024];
} _buf_send;

static void _reset(void)
{
    _buf_send.size = 0;
}


void motoilet_whisper__state_sync_cb(const unsigned char *buf, unsigned char len)
{

}

unsigned char motoilet_whisper_transmission__send(struct whisper_message *message)
{
    uint8_t *payload = malloc(MOTOILET_WHISPER_MESSAGE_PAYLOAD_LEN);
    memcpy(payload, message->payload, MOTOILET_WHISPER_MESSAGE_PAYLOAD_LEN);
    _buf_send.buf[_buf_send.size++] = (struct whisper_message){
        .type = message->type,
        .payload = payload,
    };

    return 12;
}

void test_reset(void)
{
    _reset();
    motoilet_whisper__reset();

    TEST_ASSERT_EQUAL(1, _buf_send.size);
    TEST_ASSERT_EQUAL(0x13, _buf_send.buf[0].type);
}

void setUp(void) {}
void tearDown(void) {}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_reset);

    return UNITY_END();
}