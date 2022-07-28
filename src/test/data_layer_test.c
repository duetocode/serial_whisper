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
#include "data_layer.h"
#include "motoilet_whisper_data_layer.h"
#include <stdint.h>
#include <stdlib.h>
#include "motoilet_whisper_driver.h"
#include "data_layer_driver_mock.h"
#include "app_layer.h"

void test_init(void)
{
    TEST_ASSERT_EQUAL(NULL, _buf_recv.num_matched);
    TEST_ASSERT_EQUAL(STATE_PREFIX, _state);
    TEST_ASSERT_EQUAL(0, _buf_send.num_transmission);
    TEST_ASSERT_EQUAL(NULL, _buf_send.payload);

    data_layer_driver_mock_init();
}

void test_receive(void)
{
    uint8_t data[] = {0xA5, WHISPER_MESSAGE_TYPE__STATE_UPDATE, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

    unsigned short checksum = update_crc_buf(data, sizeof(data), CRC_INIT);
    motoilet_whisper_message_data__received(data, sizeof(data));
    motoilet_whisper_message_data__received((uint8_t *)&checksum, sizeof(checksum));

    TEST_ASSERT_EQUAL(1, _mock_received_cb_buf.size);
    TEST_ASSERT_EQUAL(12, _mock_buf_send.size);
    TEST_ASSERT_EQUAL(WHISPER_MESSAGE_TYPE__ACK, _mock_buf_send.buf[2]);
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

    _mock_buf_send.size = 0;
    motoilet_whisper_message__send(&msg);

    TEST_ASSERT_EQUAL(12, _mock_buf_send.size);
    TEST_ASSERT_EQUAL(0xA5, _mock_buf_send.buf[0]);
    TEST_ASSERT_EQUAL(WHISPER_MESSAGE_TYPE__STATE_UPDATE, _mock_buf_send.buf[1]);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg.payload, &_mock_buf_send.buf[2], 8);
    TEST_ASSERT_EQUAL_UINT8_ARRAY((uint8_t *)&checksum, &_mock_buf_send.buf[10], 2);
}

void test_on_ack(void)
{
    uint8_t payload[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    struct whisper_message msg = {0};
    _buf_send.payload = payload;
    _buf_send.num_transmission = 1;
    _mock_cancel_delay_invocations = 0;

    motoilet_whisper_message_data__received(ACK_MESSAGE, sizeof(ACK_MESSAGE));
    TEST_ASSERT_EQUAL(NULL, _buf_send.payload);
    TEST_ASSERT_EQUAL(0, _buf_send.num_transmission);
    TEST_ASSERT_EQUAL(1, _mock_cancel_delay_invocations);
}

void test_retransmission(void)
{
    data_layer_driver_mock_init();
    uint8_t buf[8] = {0};
    struct whisper_message msg = {.type=0x02, .payload = buf};

    motoilet_whisper_message__send(&msg);
    TEST_ASSERT_EQUAL(1, _mock_set_delay_invocations.size);
    TEST_ASSERT_EQUAL(RETRANSMISSION_DELAY, _mock_set_delay_invocations.buf[0]);
    TEST_ASSERT_EQUAL(0, _mock_cancel_delay_invocations);
    TEST_ASSERT_EQUAL(0, _mock_delivery_cb_buf.size);

    int i = 0;
    for(i = 0; i < RETRANSMISSION_MAX; i++)
    {
        motoilet_whisper_data__timeout_cb();
        TEST_ASSERT_EQUAL(i + 2, _mock_set_delay_invocations.size);
        TEST_ASSERT_EQUAL(0, _mock_cancel_delay_invocations);
        TEST_ASSERT_EQUAL(0, _mock_delivery_cb_buf.size);
    }

    motoilet_whisper_data__timeout_cb();
    TEST_ASSERT_EQUAL(1, _mock_delivery_cb_buf.size);
    TEST_ASSERT_EQUAL(MOTOILET_WHISPER_MESSAGE_DELIVERY_FAILED, _mock_delivery_cb_buf.buf[0]);

    _mock_buf_send.size = 0;
    motoilet_whisper_data__timeout_cb();
    TEST_ASSERT_EQUAL(0, _mock_buf_send.size);
}

void setUp(void)
{
    motoilet_whisper_message__data_init();
}
void tearDown(void) {}


int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_init);
    RUN_TEST(test_receive);
    RUN_TEST(test_send);
    RUN_TEST(test_on_ack);
    RUN_TEST(test_retransmission);

    return UNITY_END();
}