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
#ifndef MOTOILET_WHISPER_DRIVER_H
#define MOTOILET_WHISPER_DRIVER_H

/**
 * @brief dispatch data received from link layer (eg, from UART) to the data layer
 * 
 * @param buf data received from the underlying layer
 * @param len length of the buf
 * @return unsigned char 0 if successful, 1 if failed
 */
unsigned char motoilet_whisper_message_data__received(const unsigned char *buf, unsigned int len);


/**
 * @brief callback for the timer set by motoilet_whisper_data__set_delay
 */
void motoilet_whisper_data__timeout_cb(void);


/**
 * @brief write raw data to the link layer, should implemented by the link layer
 * 
 * @param buf data to be written to the underlying layer
 * @param len length of the buf
 */
void motoilet_whisper_data__write(const unsigned char *buf, unsigned char len);

/**
 * @brief set a timer that calls the function motoilet_whisper_data__timeout_cb in the specified time, should be implemented by the link layer
 * 
 * @param delay_in_ms specified time in milliseconds
 */
void motoilet_whisper_data__set_delay(unsigned short delay_in_ms);

/**
 * @brief cancel the timer previously set by motoilet_whisper_data__set_delay, should be implmeneted by the link layer
 */
void motoilet_whisper_data__cancel_delay(void);

#endif // MOTOILET_WHISPER_DRIVER_H