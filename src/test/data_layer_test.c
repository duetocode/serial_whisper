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
#include "crc.h"
#include "data_layer.c"
#include "motoilet_whisper_data_layer.h"
#include <stdint.h>
#include <stdlib.h>
#include "motoilet_whisper_driver.h"
#include "driver_mock.h"
#include "app_layer.h"

static struct
{
    size_t size;
    struct whisper_message buf[BUF_SIZE];
} _mock_received_cb_buf;

void motoilet_whisper_data__received_cb(const struct whisper_message *message)
{
    struct whisper_message *r = &_mock_received_cb_buf.buf[_mock_received_cb_buf.size++];
    memcpy(r, message, sizeof(struct whisper_message));
    uint8_t *payload = malloc(MOTOILET_WHISPER_MESSAGE_PAYLOAD_LEN);
    r->payload = payload;
    memcpy(payload, message->payload, MOTOILET_WHISPER_MESSAGE_PAYLOAD_LEN);
}

void test_init(void)
{
    TEST_ASSERT_EQUAL(NULL, _buf_recv.num_matched);
    TEST_ASSERT_EQUAL(STATE_PREFIX, _state);
}

void test_receive(void)
{
    uint8_t data[] = {0xA5, WHISPER_MESSAGE_TYPE__STATE_UPDATE, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

    TEST_ASSERT_EQUAL(0, _mock_received_cb_buf.size);

    unsigned short checksum = update_crc_buf(data, sizeof(data), CRC_INIT);
    motoilet_whisper_driver__received_cb(data, sizeof(data));
    motoilet_whisper_driver__received_cb((uint8_t *)&checksum, sizeof(checksum));

    TEST_ASSERT_EQUAL(1, _mock_received_cb_buf.size);
    TEST_ASSERT_EQUAL(WHISPER_MESSAGE_TYPE__STATE_UPDATE, _mock_received_cb_buf.buf[0].type);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(&data[2], _mock_received_cb_buf.buf[0].payload, MOTOILET_WHISPER_MESSAGE_PAYLOAD_LEN);
}

void test_send(void)
{
    uint8_t payload[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    struct whisper_message msg = {
        .type = WHISPER_MESSAGE_TYPE__STATE_UPDATE,
        .payload = payload,
    };

    uint16_t checksum = update_crc_buf(WHISPER_MESSAGE_PREFIX, LEN_PREFIX, CRC_INIT);
    checksum = update_crc(WHISPER_MESSAGE_TYPE__STATE_UPDATE, checksum);
    checksum = update_crc_buf(msg.payload, sizeof(msg.payload), checksum);

    const uint16_t sample = 0x0001;
    if (*(uint8_t *)&sample == 0x01)
    {
        // we are little endian, so we need to swap the bytes as the expected checksum should be big endian
        checksum = ((checksum & 0x00FF) << 8) | ((checksum & 0xFF00) >> 8);
    }


    _mock_buf_send.size = 0;
    motoilet_whisper_data__send(&msg);

    TEST_ASSERT_EQUAL(12, _mock_buf_send.size);
    TEST_ASSERT_EQUAL(0xA5, _mock_buf_send.buf[0]);
    TEST_ASSERT_EQUAL(WHISPER_MESSAGE_TYPE__STATE_UPDATE, _mock_buf_send.buf[1]);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg.payload, &_mock_buf_send.buf[2], 8);
    TEST_ASSERT_EQUAL_UINT8_ARRAY((uint8_t *)&checksum, &_mock_buf_send.buf[10], 2);
}

void setUp(void)
{
    motoilet_whisper_data__init();

    data_layer_driver_mock_init();
    _mock_received_cb_buf.size = 0;
}

void tearDown(void) {
    size_t i = 0;
    for(i = 0; i < _mock_received_cb_buf.size; i++)
    {
        free((void *)_mock_received_cb_buf.buf[i].payload);
    }
    _mock_received_cb_buf.size = 0;
}


int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_init);
    RUN_TEST(test_receive);
    RUN_TEST(test_send);

    return UNITY_END();
}