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
#ifndef MOTOILET_WHISPER_DATA_LAYER_H
#define MOTOILET_WHISPER_DATA_LAYER_H

struct whisper_message
{
    unsigned char type;
    const unsigned char *payload;
};

/**
 * @brief Initialize the data layer of the whisper protocol.
 *
 * @return unsigned char 0 if successful, 1 otherwise.
 */
unsigned char motoilet_whisper_message__data_init(void);

/**
 * @brief send out a whisper message.
 *
 * @param message the message to send.
 * @return unsigned char 0 if successful, 1 if the previous message is still under processing.
 */
unsigned char motoilet_whisper_message__send(struct whisper_message *message);

/**
 * @brief delivery report for the sent message
 *
 * @param delivered 0 if the message was successfully delivered, 1 otherwise.
 */
void motoilet_whisper_message__delivery_cb(unsigned char delivered);
#define MOTOILET_WHISPER_MESSAGE_DELIVERY_SUCCESS 0
#define MOTOILET_WHISPER_MESSAGE_DELIVERY_FAILED 1

/**
 * @brief callback for message received from the other end
 *
 * @param message the message received.
 */
void motoilet_whisper_message__received_cb(struct whisper_message *message);



#endif // MOTOILET_WHISPER_DATA_LAYER_H 