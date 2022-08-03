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

#include "motoilet_whisper_data_layer.h"
#include "transmission_layer.c"

#include "data_layer_mock.h"
#include "driver_mock.h"

static struct whisper_message sample;
static const uint8_t PAYLOAD[] = {0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00};

static struct {
    size_t size;
    uint8_t buf[128];
} _delivery_cb_buf;

static struct {
    size_t size;
    struct whisper_message buf[128]; 
} _received_cb_buf;


void setUp(void)
{
    _delivery_cb_buf.size = 0;
    _received_cb_buf.size = 0;
    sample.payload = malloc(sizeof(PAYLOAD));
    memcpy(sample.payload, PAYLOAD, sizeof(PAYLOAD));

    motoilet_whisper_data__init();
    data_layer_driver_mock_init();
    motoilet_whisper_transmission__init();
}

void motoilet_whisper_transmission__received_cb(const struct whisper_message *msg)
{
    memcpy(&_received_cb_buf.buf[_received_cb_buf.size++], msg, sizeof(struct whisper_message));
}

void motoilet_whisper_transmission__delivery_cb(unsigned char delivered, struct whisper_message *msg)
{
    _delivery_cb_buf.buf[_delivery_cb_buf.size++] = delivered;
}

void test_init(void)
{
    TEST_ASSERT_EQUAL(0, _buf_send.num_transmission);
    TEST_ASSERT_EQUAL(NULL, _buf_send.payload);
}

void test_send(void)
{
    motoilet_whisper_transmission__send(&sample);
    TEST_ASSERT_EQUAL(1, _mock_send_buf.size);
    TEST_ASSERT_EQUAL(sample.type, _mock_send_buf.buf[0].type);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(sample.payload, _mock_send_buf.buf[0].payload, sizeof(PAYLOAD));

    TEST_ASSERT_EQUAL(1, _buf_send.num_transmission);
    TEST_ASSERT_EQUAL(sample.type, _buf_send.payload->type);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(sample.payload, _buf_send.payload->payload, sizeof(PAYLOAD));
    TEST_ASSERT_EQUAL(1, _mock_set_delay_invocations.size);
    TEST_ASSERT_EQUAL(RETRANSMISSION_DELAY, _mock_set_delay_invocations.buf[0]);
    TEST_ASSERT_EQUAL(0, _mock_cancel_delay_invocations);
}

void test_receive(void)
{
    motoilet_whisper_data__received_cb(&sample);

    TEST_ASSERT_EQUAL(1, _received_cb_buf.size);
    TEST_ASSERT_EQUAL(sample.type, _received_cb_buf.buf[0].type);
}

void test_retransmission(void)
{
    _buf_send.num_transmission = 1;
    _buf_send.payload = &sample;

    int i = 0;
    for(i = 0; i < RETRANSMISSION_MAX; i++)
    {
        motoilet_whisper_driver__timeout_cb();
        TEST_ASSERT_EQUAL(i + 1, _mock_set_delay_invocations.size);
        TEST_ASSERT_EQUAL(0, _mock_cancel_delay_invocations);
        TEST_ASSERT_EQUAL(0, _delivery_cb_buf.size);
    }

    // the next trigger will results dropping the message and the sending of failed delivery report
    motoilet_whisper_driver__timeout_cb();
    TEST_ASSERT_EQUAL(1, _delivery_cb_buf.size);
    TEST_ASSERT_EQUAL(MOTOILET_WHISPER_MESSAGE_DELIVERY_FAILED, _delivery_cb_buf.buf[0]);
    TEST_ASSERT_EQUAL(0, _mock_cancel_delay_invocations);
    TEST_ASSERT_EQUAL(NULL, _buf_send.payload);

    // nothing should happened at this point
    motoilet_whisper_driver__timeout_cb();
    TEST_ASSERT_EQUAL(0, _mock_buf_send.size);
    TEST_ASSERT_EQUAL(0, _mock_cancel_delay_invocations);
}

void test_on_ack(void)
{
    _buf_send.payload = &sample;
    _buf_send.num_transmission = 1;

    struct whisper_message ack = {
        .type = WHISPER_MESSAGE_TYPE__ACK,
        .payload = sample.payload,
    };

    motoilet_whisper_data__received_cb(&ack);

    // the buffer should be rested and the retransmission should be cancelled
    TEST_ASSERT_EQUAL(NULL, _buf_send.payload);
    TEST_ASSERT_EQUAL(0, _buf_send.num_transmission);
    TEST_ASSERT_EQUAL(1, _mock_cancel_delay_invocations);
}

void tearDown(void)
{
    mock_data_layer_teardown();
    sample.type = 0x01;
    if (sample.payload != NULL)
        free(sample.payload);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_init);
    RUN_TEST(test_send);
    RUN_TEST(test_receive);
    RUN_TEST(test_retransmission);
    RUN_TEST(test_on_ack);
    return UNITY_END();
}