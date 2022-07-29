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
#include "data_layer.h"
#include <string.h>
#include "motoilet_whisper_data_layer.h"
#include "motoilet_whisper_driver.h"
#include "byte_order.h"

#include "crc.h"

unsigned char _state, _next_state;
#define STATE_PREFIX 0x01
#define STATE_HEADER 0x02
#define STATE_PAYLOAD 0x03
#define STATE_CHECKSUM 0x04

#define LEN_PREFIX sizeof(WHISPER_MESSAGE_PREFIX)
#define LEN_HEADER sizeof(unsigned char)
#define LEN_PAYLOAD 8
#define LEN_CHECKSUM 2

#define RETRANSMISSION_DELAY 50
#define RETRANSMISSION_MAX 3

const unsigned char ACK_MESSAGE[] = {0xA5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x95};

struct
{
    unsigned char num_matched;
    unsigned char type;
    unsigned char payload[LEN_PAYLOAD];
} _buf_recv;

struct
{
    unsigned char num_transmission;
    unsigned char type;
    const unsigned char *payload;
} _buf_send;

unsigned char motoilet_whisper_message__data_init(void)
{
    _state = STATE_PREFIX;
    _next_state = STATE_PREFIX;

    _buf_recv.num_matched = 0;

    // setup send buffer
    _buf_send.num_transmission = 0;
    _buf_send.payload = NULL;

    return 0;
}

unsigned short _write(const unsigned char *buf, unsigned char len, unsigned short crc)
{
    motoilet_whisper_data__write(buf, len);
    return update_crc_buf(buf, len, crc);
}

void _send(void)
{
    // only processed if there is a message to send
    if (_buf_send.payload == NULL)
        return;

    // check if we reached the maximum number of retransmissions
    // the 1 is for the first attempt
    if (_buf_send.num_transmission >= RETRANSMISSION_MAX + 1)
    {
        // report frame loss
        motoilet_whisper_message__delivery_cb(MOTOILET_WHISPER_MESSAGE_DELIVERY_FAILED);

        // reset and return
        _buf_send.payload = NULL;
        _buf_send.num_transmission = 0;
        return;
    }

    unsigned char len = 0;

    // prefix
    unsigned checksum = _write(WHISPER_MESSAGE_PREFIX, LEN_PREFIX, CRC_INIT);
    len += LEN_PREFIX;

    // header
    checksum = _write(&_buf_send.type, LEN_HEADER, checksum);
    len += LEN_HEADER;

    // payload
    checksum = _write(_buf_send.payload, LEN_PAYLOAD, checksum);
    len += LEN_PAYLOAD;

    // checksum
    checksum = htons(checksum);
    _write((unsigned char *)&checksum, LEN_CHECKSUM, checksum);
    len += LEN_CHECKSUM;

    ++(_buf_send.num_transmission);
    motoilet_whisper_data__set_delay(RETRANSMISSION_DELAY);
}

unsigned char motoilet_whisper_message__send(struct whisper_message *message)
{
    if (_buf_send.payload != NULL)
        return 1;

    // setup send buffer
    _buf_send.payload = message->payload;
    _buf_send.type = message->type;
    _buf_send.num_transmission = 0;

    _send();
    return 0;
}

unsigned char _on_prefix(const unsigned char *buf, unsigned char len);
unsigned char _on_header(const unsigned char *buf, unsigned char len);
unsigned char _on_payload(const unsigned char *buf, unsigned char len);
unsigned char _on_checksum(const unsigned char *buf, unsigned char len);

unsigned char motoilet_whisper_message_data__received(const unsigned char *buf, unsigned int len)
{
    unsigned char num_consumed = 0;
    while (len > 0)
    {
        _next_state = _state;

        switch (_state)
        {
        case STATE_PREFIX:
            num_consumed = _on_prefix(buf, len);
            break;
        case STATE_HEADER:
            num_consumed = _on_header(buf, len);
            break;
        case STATE_PAYLOAD:
            num_consumed = _on_payload(buf, len);
            break;
        case STATE_CHECKSUM:
            num_consumed = _on_checksum(buf, len);
            break;
        }

        _state = _next_state;
        buf = &buf[num_consumed];
        len -= num_consumed;
    }

    return 0;
}

unsigned char _on_prefix(const unsigned char *buf, unsigned char len)
{
    unsigned char i = 0;
    for (i = 0; i < len; i++)
    {
        if (buf[i] == WHISPER_MESSAGE_PREFIX[_buf_recv.num_matched])
        {
            // increase matched counter
            ++_buf_recv.num_matched;

            // check if we matched the whole prefix
            if (_buf_recv.num_matched == LEN_PREFIX)
            {
                // transite to header state
                _buf_recv.num_matched = 0;
                _next_state = STATE_HEADER;
                return LEN_PREFIX;
            }
        }
        else {
            // reset matched counter
            _buf_recv.num_matched = 0;
        }
    }

    return len;
}

unsigned char _on_header(const unsigned char *buf, unsigned char len)
{
    _buf_recv.type = buf[0];
    _next_state = STATE_PAYLOAD;
    return 1;
}

unsigned char _on_payload(const unsigned char *buf, unsigned char len)
{
    // copy payload to buffer
    unsigned char bytes_to_copy = len;

    if (bytes_to_copy > LEN_PAYLOAD - _buf_recv.num_matched)
        bytes_to_copy = LEN_PAYLOAD - _buf_recv.num_matched;
    memcpy(&_buf_recv.payload[_buf_recv.num_matched], buf, bytes_to_copy);

    _buf_recv.num_matched += bytes_to_copy;

    // check if we copied the whole payload
    if (_buf_recv.num_matched == LEN_PAYLOAD)
    {
        // transite to checksum state
        _buf_recv.num_matched = 0;
        _next_state = STATE_CHECKSUM;
    }

    return bytes_to_copy;
}

void _on_ack(void);
void _ack(void);

unsigned char _on_checksum(const unsigned char *buf, unsigned char len)
{
    static unsigned short checksum = 0;

    if (_buf_recv.num_matched == 0)
    {
        // the first byte of the checksum
        checksum = buf[0] << 8;
        ++_buf_recv.num_matched;
    }
    else
    {
        // the second byte
        checksum |= buf[0] & 0xFF;
        checksum = ntohs(checksum);

        // transite to prefix state
        _buf_recv.num_matched = 0;
        _next_state = STATE_PREFIX;

        // calcute the checksum of the buffered data
        unsigned short actual_checksum = update_crc_buf(WHISPER_MESSAGE_PREFIX, LEN_PREFIX, CRC_INIT);
        actual_checksum = update_crc(_buf_recv.type, actual_checksum);
        actual_checksum = update_crc_buf(_buf_recv.payload, LEN_PAYLOAD, actual_checksum);

        // check if the checksums are matching
        if (actual_checksum == checksum)
        {
            if (_buf_recv.type == 0x00)
            {
                // acknowledgement received
                _on_ack();
            }
            else
            {
                // report the message and send an acknowledgement
                struct whisper_message msg = {.type = _buf_recv.type, .payload = _buf_recv.payload};
                motoilet_whisper_message__received_cb(&msg);
                _ack();
            }
        }
    }
    return 1;
}

void _on_ack(void)
{
    // only processed if there is a message to be sent
    if (_buf_send.payload == NULL)
        return;

    // cancel the retransmission timer
    motoilet_whisper_data__cancel_delay();
    // report the delivery of the message
    motoilet_whisper_message__delivery_cb(MOTOILET_WHISPER_MESSAGE_DELIVERY_SUCCESS);

    _buf_send.payload = NULL;
    _buf_send.num_transmission = 0;
}

void _ack(void)
{
    // just send back the acknowledgement data frame
    motoilet_whisper_data__write(ACK_MESSAGE, sizeof(ACK_MESSAGE));
}

void motoilet_whisper_data__timeout_cb(void)
{
    // delegate to the retransmission routine 
    _send();
}