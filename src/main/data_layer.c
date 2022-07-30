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
#include "motoilet_whisper_data_layer.h"
#include <string.h>
#include "motoilet_whisper_driver.h"
#include "byte_order.h"

#include "crc.h"

const static unsigned char WHISPER_MESSAGE_PREFIX[] = {0xA5};

unsigned char _state, _next_state;
#define STATE_PREFIX 0x01
#define STATE_HEADER 0x02
#define STATE_PAYLOAD 0x03
#define STATE_CHECKSUM 0x04

#define LEN_PREFIX sizeof(WHISPER_MESSAGE_PREFIX)
#define LEN_HEADER sizeof(unsigned char)
#define LEN_PAYLOAD 8
#define LEN_CHECKSUM 2

struct
{
    unsigned char num_matched;
    unsigned char type;
    unsigned char payload[LEN_PAYLOAD];
} _buf_recv;

unsigned char motoilet_whisper_data__init(void)
{
    _state = STATE_PREFIX;
    _next_state = STATE_PREFIX;

    _buf_recv.num_matched = 0;

    return 0;
}

unsigned short _write(const unsigned char *buf, unsigned char len, unsigned short crc)
{
    motoilet_whisper_driver__write(buf, len);
    return update_crc_buf(buf, len, crc);
}

unsigned char motoilet_whisper_data__send(const struct whisper_message *message)
{

    unsigned char len = 0;
    unsigned checksum = 0;

    // prefix
    checksum = _write(WHISPER_MESSAGE_PREFIX, LEN_PREFIX, CRC_INIT);
    len += LEN_PREFIX;

    // header
    checksum = _write(&message->type, LEN_HEADER, checksum);
    len += LEN_HEADER;

    // payload
    checksum = _write(message->payload, LEN_PAYLOAD, checksum);
    len += LEN_PAYLOAD;

    // checksum
    checksum = htons(checksum);
    _write((unsigned char *)&checksum, LEN_CHECKSUM, checksum);

    return 0;
}

unsigned char _on_prefix(const unsigned char *buf, unsigned char len);
unsigned char _on_header(const unsigned char *buf, unsigned char len);
unsigned char _on_payload(const unsigned char *buf, unsigned char len);
unsigned char _on_checksum(const unsigned char *buf, unsigned char len);

unsigned char motoilet_whisper_driver__received_cb(const unsigned char *buf, unsigned int len)
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

unsigned char _on_checksum(const unsigned char *buf, unsigned char len)
{
    static unsigned short checksum = 0, actual_checksum = 0;
    struct whisper_message msg;

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
        actual_checksum = update_crc_buf(WHISPER_MESSAGE_PREFIX, LEN_PREFIX, CRC_INIT);
        actual_checksum = update_crc(_buf_recv.type, actual_checksum);
        actual_checksum = update_crc_buf(_buf_recv.payload, LEN_PAYLOAD, actual_checksum);

        // check if the checksums are matching
        if (actual_checksum == checksum)
        {
            // report the received message to the upper layer
            msg.type = _buf_recv.type; 
            msg.payload = _buf_recv.payload;
            motoilet_whisper_data__received_cb(&msg);
        }
    }
    return 1;
}