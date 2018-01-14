/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "opentx.h"

#define END                             0xC0
#define ESC                             0xDB
#define ESC_END                         0xDC
#define ESC_ESC                         0xDD

#define FRAME_TYPE_REQUEST_ACK          0x01
#define FRAME_TYPE_REQUEST_NACK         0x02
#define FRAME_TYPE_ANSWER               0x10

#define COMMAND_ID_RF_INIT              0x01
#define COMMAND_ID_BIND                 0x02
#define COMMAND_ID_SET_RECEIVER_ID      0x03
#define COMMAND_ID_SEND_CHANNEL_DATA    0x05

#define FLYSKY_MODULE_TIMEOUT           55 /* ms */


enum FlySkyModuleState {
  FLYSKY_MODULE_STATE_INIT,
  FLYSKY_MODULE_STATE_BIND,
  FLYSKY_MODULE_STATE_SET_RECEIVER_ID,
  FLYSKY_MODULE_STATE_DEFAULT,
};

void initFlySkyArray(uint8_t port)
{
  modulePulsesData[port].flysky.ptr = modulePulsesData[port].flysky.pulses;
  modulePulsesData[port].flysky.crc = 0;
}

inline void putFlySkyByte(uint8_t port, uint8_t byte)
{
  if (END == byte) {
    *modulePulsesData[port].flysky.ptr++ = ESC;
    *modulePulsesData[port].flysky.ptr++ = ESC_END;
  }
  else if (ESC == byte) {
    *modulePulsesData[port].flysky.ptr++ = ESC;
    *modulePulsesData[port].flysky.ptr++ = ESC_ESC;
  }
  else {
    *modulePulsesData[port].flysky.ptr++ = byte;
  }
}

void putFlySkyFrameByte(uint8_t port, uint8_t byte)
{
  modulePulsesData[port].flysky.crc += byte;
  putFlySkyByte(port, byte);
}

void putFlySkyHead(uint8_t port)
{
  *modulePulsesData[port].flysky.ptr++ = END;
}

void putFlySkyFrameIndex(uint8_t port)
{
  putFlySkyFrameByte(port, modulePulsesData[port].flysky.frame_index);
}

void putFlySkyCrc(uint8_t port)
{
  putFlySkyByte(port, modulePulsesData[port].flysky.crc ^ 0xff);
}

void putFlySkyTail(uint8_t port)
{
  *modulePulsesData[port].flysky.ptr++ = END;
}

void putFlySkyRfInit(uint8_t port)
{
  putFlySkyFrameByte(port, FRAME_TYPE_REQUEST_ACK);
  putFlySkyFrameByte(port, COMMAND_ID_RF_INIT);
}

void putFlySkyBind(uint8_t port)
{
  putFlySkyFrameByte(port, FRAME_TYPE_REQUEST_ACK);
  putFlySkyFrameByte(port, COMMAND_ID_BIND);
  putFlySkyFrameByte(port, 0x01); // TODO 1 = normal power, 0 = low power
  // TODO transmitter ID (4 bytes)
  putFlySkyFrameByte(port, 0x06);
  putFlySkyFrameByte(port, 0x06);
  putFlySkyFrameByte(port, 0x06);
  putFlySkyFrameByte(port, 0x06);
}

void putFlySkySetReceiverID(uint8_t port)
{
  putFlySkyFrameByte(port, FRAME_TYPE_REQUEST_ACK);
  putFlySkyFrameByte(port, COMMAND_ID_SET_RECEIVER_ID);
  putFlySkyFrameByte(port, g_model.header.modelId[port]); // receiver ID (byte 0)
  putFlySkyFrameByte(port, 0x00); // receiver ID (byte 1)
  putFlySkyFrameByte(port, 0x00); // receiver ID (byte 2)
  putFlySkyFrameByte(port, 0x00); // receiver ID (byte 3)
}

void putFlySkySendChannelData(uint8_t port)
{
  putFlySkyFrameByte(port, FRAME_TYPE_REQUEST_NACK);
  putFlySkyFrameByte(port, COMMAND_ID_SEND_CHANNEL_DATA);
  putFlySkyFrameByte(port, 0x00); // TODO 1 = failsafe
  uint8_t channels_count = 8; // TODO + g_model.moduleData[port].channelsCount;
  putFlySkyFrameByte(port, channels_count);
  for (uint8_t channel=0; channel<channels_count; channel++) {
    uint16_t value = channelOutputs[channel] + 2 * PPM_CH_CENTER(channel) - 2 * PPM_CENTER;
    putFlySkyFrameByte(port, value & 0xff);
    putFlySkyFrameByte(port, value >> 8);
  }
}

void incrFlySkyFrame(uint8_t port)
{
  if (++modulePulsesData[port].flysky.frame_index == 0)
    modulePulsesData[port].flysky.frame_index = 1;
}

bool checkFlySkyFrameCrc(const uint8_t * ptr, uint8_t size)
{
  uint8_t crc = 0;
  for (uint8_t i=0; i<size; i++) {
    crc += ptr[i];
  }
  return (crc ^ 0xff) == ptr[size];
}

void parseFlySkyFeedbackFrame(uint8_t port)
{
  const uint8_t * ptr = modulePulsesData[port].flysky.telemetry;
  if (*ptr++ != END)
    return;

  uint8_t frame_number = *ptr++;
  uint8_t frame_type = *ptr++;
  uint8_t command_id = *ptr++;

  switch (command_id) {
    case COMMAND_ID_RF_INIT:
      if (!checkFlySkyFrameCrc(modulePulsesData[port].flysky.telemetry+1, 4)) {
        return;
      }
      else if (modulePulsesData[port].flysky.state == FLYSKY_MODULE_STATE_INIT && *ptr++ == 0x01) {
        modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_DEFAULT;
        incrFlySkyFrame(port);
      }
      break;

    case COMMAND_ID_BIND:
      if (!checkFlySkyFrameCrc(modulePulsesData[port].flysky.telemetry+1, 7)) {
        return;
      }
      else if (modulePulsesData[port].flysky.state == FLYSKY_MODULE_STATE_BIND) {
        moduleFlag[port] &= ~MODULE_BIND;
        incrFlySkyFrame(port);
      }
      break;
  }
}

uint8_t intmoduleGetByte(uint8_t * byte);

void checkFlySkyFeedback(uint8_t port)
{
  uint8_t byte;

  while (intmoduleGetByte(&byte)) {
    if (byte == END && modulePulsesData[port].flysky.telemetry_index > 0) {
      parseFlySkyFeedbackFrame(port);
      modulePulsesData[port].flysky.telemetry_index = 0;
    }
    else {
      if (byte == ESC) {
        modulePulsesData[port].flysky.esc_state = 1;
      }
      else {
        if (modulePulsesData[port].flysky.esc_state) {
          modulePulsesData[port].flysky.esc_state = 0;
          if (byte == ESC_END)
            byte = END;
          else if (byte == ESC_ESC)
            byte = ESC;
        }
        modulePulsesData[port].flysky.telemetry[modulePulsesData[port].flysky.telemetry_index++] = byte;
        if (modulePulsesData[port].flysky.telemetry_index >= sizeof(modulePulsesData[port].flysky.telemetry)) {
          // TODO buffer is full, log an error?
          modulePulsesData[port].flysky.telemetry_index = 0;
        }
      }
    }
  }
}

void resetPulsesFlySky(uint8_t port)
{
  modulePulsesData[port].flysky.frame_index = 1;
  modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_INIT;
  modulePulsesData[port].flysky.state_index = 0;
  modulePulsesData[port].flysky.esc_state = 0;
}

void setupPulsesFlySky(uint8_t port)
{
  checkFlySkyFeedback(port);

  if (moduleFlag[port] == MODULE_BIND) {
    modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_BIND;
  }
  /*else if (moduleFlag[port] == MODULE_RANGECHECK) {

  }*/

  initFlySkyArray(port);
  putFlySkyHead(port);
  putFlySkyFrameIndex(port);

  if (modulePulsesData[port].flysky.state < FLYSKY_MODULE_STATE_DEFAULT) {
    if (++modulePulsesData[port].flysky.state_index >= FLYSKY_MODULE_TIMEOUT / PXX_PERIOD_DURATION) {
      modulePulsesData[port].flysky.state_index = 0;
      switch (modulePulsesData[port].flysky.state) {
        case FLYSKY_MODULE_STATE_INIT:
          putFlySkyRfInit(port);
          break;
        case FLYSKY_MODULE_STATE_BIND:
          putFlySkyBind(port);
          break;
        case FLYSKY_MODULE_STATE_SET_RECEIVER_ID:
          putFlySkySetReceiverID(port);
          break;
        default:
          modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_INIT;
          initFlySkyArray(port);
          return;
      }
    }
    else {
      initFlySkyArray(port);
      return;
    }
  }
  else {
    putFlySkySendChannelData(port);
    incrFlySkyFrame(port);
  }

  putFlySkyCrc(port);
  putFlySkyTail(port);
}
