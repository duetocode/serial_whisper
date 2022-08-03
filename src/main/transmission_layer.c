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
#include "motoilet_whisper_transmission_layer.h"

#include "motoilet_whisper_message.h"
#include "motoilet_whisper_driver.h"
#include "motoilet_whisper_data_layer.h"

#include <string.h>

#define RETRANSMISSION_DELAY 50
#define RETRANSMISSION_MAX 3

const unsigned char EMPTY_PAYLOAD[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static struct
{
    unsigned char num_transmission;
    struct whisper_message *payload;
} _buf_send;

static void _buf_send_reset(void)
{
    _buf_send.payload = NULL;
    _buf_send.num_transmission = 0;
}

unsigned char motoilet_whisper_transmission__init(void)
{
    _buf_send_reset();
    return 0;
}

static void _send(void)
{
    // only processed if there is a message to send
    if (_buf_send.payload == NULL)
        return;

    // check if we reached the maximum number of retransmissions
    // the 1 is for the first attempt
    if (_buf_send.num_transmission >= RETRANSMISSION_MAX + 1)
    {
        // report frame loss
        motoilet_whisper_transmission__delivery_cb(MOTOILET_WHISPER_MESSAGE_DELIVERY_FAILED, _buf_send.payload);

        // reset and return
        _buf_send_reset();
        return;
    }

    // deliver the message
    motoilet_whisper_data__send(_buf_send.payload);

    // setup next retransmission
    ++(_buf_send.num_transmission);
    motoilet_whisper_driver__set_delay(RETRANSMISSION_DELAY);
}

unsigned char motoilet_whisper_transmission__send(struct whisper_message *message)
{
    if (_buf_send.payload != NULL)
        return 1;

    // setup send buffer
    _buf_send.payload = message;
    _buf_send.num_transmission = 0;

    _send();
    return 0;
}

static void _on_ack(void);
static void _ack(void);

void motoilet_whisper_data__received_cb(const struct whisper_message *msg)
{
    if (msg->type == WHISPER_MESSAGE_TYPE__ACK)
    {
        // acknowledgement received
        _on_ack();
    }
    else
    {
        // report the message and send an acknowledgement
        motoilet_whisper_transmission__received_cb(msg);
        _ack();
    }
}

static void _on_ack(void)
{
    // only processed if there is a message to be sent
    if (_buf_send.payload == NULL)
        return;

    // cancel the retransmission timer to stop further retransmission attempts
    motoilet_whisper_driver__cancel_delay();
    // report the delivery of the message
    motoilet_whisper_transmission__delivery_cb(MOTOILET_WHISPER_MESSAGE_DELIVERY_SUCCESS, _buf_send.payload);

    _buf_send_reset();
}

static void _ack(void)
{
    // just send back the acknowledgement data frame
    unsigned char empty_buf[MOTOILET_WHISPER_MESSAGE_PAYLOAD_LEN];
    struct whisper_message ack; 
    ack.type = WHISPER_MESSAGE_TYPE__ACK;
    ack.payload = empty_buf;

    motoilet_whisper_data__send(&ack);
}

void motoilet_whisper_driver__timeout_cb(void)
{
    // delegate to the retransmission routine 
    _send();
}