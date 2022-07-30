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
#include "app_layer.h"
#include "motoilet_whisper_transmission_layer.h"
#include <string.h>

#define LEN_PAYLOAD 8


static unsigned char payload[LEN_PAYLOAD];
static unsigned char _send(unsigned char type, const unsigned char buf[MOTOILET_WHISPER_MESSAGE_PAYLOAD_LEN])
{
    struct whisper_message msg;

    memcpy(payload, buf, LEN_PAYLOAD);

    msg.type = type;
    msg.payload = payload;

    return motoilet_whisper_transmission__send(&msg);
}

unsigned char motoilet_whisper__setup(const unsigned char buf[MOTOILET_WHISPER_MESSAGE_PAYLOAD_LEN])
{
    return _send(WHISPER_MESSAGE_TYPE__STATE_UPDATE_CONFIG, buf);
};

unsigned char motoilet_whisper__report_state(const unsigned char buf[MOTOILET_WHISPER_MESSAGE_PAYLOAD_LEN])
{
    return _send(WHISPER_MESSAGE_TYPE__STATE_UPDATE, buf);
};

static unsigned char _send_empty_message(unsigned type)
{
    struct whisper_message msg;
 
    memset(payload, 0, sizeof(payload));

    msg.type = type;
    msg.payload = payload;

    return motoilet_whisper_transmission__send(&msg);
}

unsigned char motoilet_whisper__pause(void)
{
    return _send_empty_message(WHISPER_MESSAGE_TYPE__PAUSE);
};

unsigned char motoilet_whisper__resume(void)
{
    return _send_empty_message(WHISPER_MESSAGE_TYPE__RESUME);
}

unsigned char motoilet_whisper__reset(void)
{
    return _send_empty_message(WHISPER_MESSAGE_TYPE__RESET);
}

unsigned char motoilet_whisper__reboot(void)
{
    return _send_empty_message(WHISPER_MESSAGE_TYPE__REBOOT);
}


void motoilet_whisper_data__received_cb(struct whisper_message *message)
{
    switch (message->type)
    {
    case WHISPER_MESSAGE_TYPE__STATE_SYNC:
        motoilet_whisper__state_sync_cb(message->payload, MOTOILET_WHISPER_MESSAGE_PAYLOAD_LEN);
        break;
    }
}