/*
 * Copyright (C) OpenTX
 *
 * Dedicate for FlySky NV14 board.
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

enum FlySkyModuleCommandID {
  COMMAND_ID_NONE,
  COMMAND_ID_RF_INIT,
  COMMAND_ID_BIND,
  COMMAND_ID_SET_RECEIVER_ID,
  COMMAND_ID_RF_GET_CONFIG,
  COMMAND_ID_SEND_CHANNEL_DATA,
  COMMAND_ID_RX_SENSOR_DATA,
  COMMAND_ID_SET_RX_PWM_PPM,
  COMMAND_ID_SET_RX_SERVO_FREQ,
  COMMAND_ID_GET_VERSION_INFO,
  COMMAND_ID_SET_RX_IBUS_SBUS,
  COMMAND_ID_SET_RX_IBUS_SERVO_EXT,
  COMMAND_ID0C_UPDATE_RF_FIRMWARE = 0x0C,
  COMMAND_ID0D_SET_TX_POWER = 0x0D,
  COMMAND_ID_SET_RF_PROTOCOL,
  COMMAND_ID_TEST_RANGE,
  COMMAND_ID_TEST_RF_RESERVED,
  COMMAND_ID20_UPDATE_RX_FIRMWARE = 0x20,

  COMMAND_ID_LAST
};
#define IS_VALID_COMMAND_ID(id)         ((id) < COMMAND_ID_LAST)

#ifndef custom_log
#define custom_log
#endif
enum DEBUG_RF_FRAME_PRINT_E {
  FRAME_PRINT_OFF,// 0:OFF, 1:RF frame only, 2:TX frame only, 3:Both RF and TX frame
  RF_FRAME_ONLY,
  TX_FRAME_ONLY,
  BOTH_FRAME_PRINT
};
#define DEBUG_RF_FRAME_PRINT            FRAME_PRINT_OFF
#define FLYSKY_MODULE_TIMEOUT           155 /* ms */
#define NUM_OF_NV14_CHANNELS            (14)
#define VALID_CH_DATA(v)                ((v) > 900 && (v) < 2100)
#define FAILSAVE_SEND_COUNTER_MAX       (400)

#define gRomData                        g_model.moduleData[INTERNAL_MODULE].romData
#define SET_DIRTY()                     storageDirty(EE_MODEL)
#if 0
enum FlySkySensorType_E {
  FLYSKY_SENSOR_RX_VOLTAGE,
  FLYSKY_SENSOR_RX_SIGNAL,
  FLYSKY_SENSOR_RX_RSSI,
  FLYSKY_SENSOR_RX_NOISE,
  FLYSKY_SENSOR_RX_SNR,
  FLYSKY_SENSOR_TEMP,
  FLYSKY_SENSOR_EXT_VOLTAGE,
  FLYSKY_SENSOR_MOTO_RPM,
  FLYSKY_SENSOR_PRESURRE,
  FLYSKY_SENSOR_GPS
};
#endif
enum FlySkyModuleState_E {
  FLYSKY_MODULE_STATE_SET_TX_POWER,
  FLYSKY_MODULE_STATE_INIT,
  FLYSKY_MODULE_STATE_BIND,
  FLYSKY_MODULE_STATE_SET_RECEIVER_ID,
  FLYSKY_MODULE_STATE_SET_RX_PWM_PPM,
  FLYSKY_MODULE_STATE_SET_RX_IBUS_SBUS,
  FLYSKY_MODULE_STATE_SET_RX_FREQUENCY,
  FLYSKY_MODULE_STATE_UPDATE_RF_FIRMWARE,
  FLYSKY_MODULE_STATE_UPDATE_RX_FIRMWARE,
  FLYSKY_MODULE_STATE_UPDATE_HALL_FIRMWARE,
  FLYSKY_MODULE_STATE_UPDATE_RF_PROTOCOL,
  FLYSKY_MODULE_STATE_GET_RECEIVER_CONFIG,
  FLYSKY_MODULE_STATE_GET_RX_VERSION_INFO,
  FLYSKY_MODULE_STATE_GET_RF_VERSION_INFO,
  FLYSKY_MODULE_STATE_IDLE,
  FLYSKY_MODULE_STATE_DEFAULT,
};

enum FlySkyBindState_E {
  BIND_LOW_POWER,
  BIND_NORMAL_POWER,
  BIND_EXIT,
};

enum FlySkyRxPulse_E {
  FLYSKY_PWM,
  FLYSKY_PPM
};

enum FlySkyRxPort_E {
  FLYSKY_IBUS,
  FLYSKY_SBUS
};

enum FlySkyFirmwareType_E {
  FLYSKY_RX_FIRMWARE,
  FLYSKY_RF_FIRMWARE
};

enum FlySkyChannelDataType_E {
  FLYSKY_CHANNEL_DATA_NORMAL,
  FLYSKY_CHANNEL_DATA_FAILSAFE
};

enum FlySkyPulseModeValue_E {
  PWM_IBUS, PWM_SBUS,
  PPM_IBUS, PPM_SBUS
};
#define GET_FLYSKY_PWM_PPM    (gRomData.mode < 2 ? FLYSKY_PWM: FLYSKY_PPM)
#define GET_FLYSKY_IBUS_SBUS  (gRomData.mode & 1 ? FLYSKY_SBUS: FLYSKY_IBUS)

typedef struct RX_FLYSKY_IBUS_S {
  uint8_t id[2];
  uint8_t channel[2];
} rx_ibus_t;

typedef struct FLYSKY_FIRMWARE_INFO_S {
  uint8_t fw_id[4];
  uint8_t fw_len[4];
  uint8_t hw_rev[4];
  uint8_t fw_rev[4];
  uint8_t fw_pkg_addr[4];
  uint8_t fw_pkg_len[4];
  uint8_t * pkg_data;
} fw_info_t;

typedef struct RF_INFO_S {
  uint8_t id[4];
  uint8_t bind_power;
  uint8_t num_of_channel;
  uint8_t channel_data_type;
  uint8_t protocol;
  uint8_t fw_state; // 0: normal, COMMAND_ID0C_UPDATE_RF_FIRMWARE or COMMAND_ID_UPDATE_FIRMWARE_END
  fw_info_t fw_info;
} rf_info_t;

typedef struct RX_INFO_S {
  int16_t servo_value[NUM_OF_NV14_CHANNELS];
  rx_ibus_t ibus;
  fw_info_t fw_info;
} rx_info_t;

static uint8_t tx_working_power = 90;
static STRUCT_HALL rfProtocolRx = {0};
static uint32_t rfRxCount = 0;
static uint8_t lastState = FLYSKY_MODULE_STATE_IDLE;
extern uint8_t intmoduleGetByte(uint8_t * byte);


static rf_info_t rf_info = {
  .id               = {8, 8, 8, 8},
  .bind_power       = BIND_LOW_POWER,
  .num_of_channel   = NUM_OF_NV14_CHANNELS, // TODO + g_model.moduleData[port].channelsCount;
  .channel_data_type= FLYSKY_CHANNEL_DATA_NORMAL,
  .protocol         = 0,
  .fw_state         = 0,
  .fw_info          = {0}
};

static rx_info_t rx_info = {
  .servo_value      = {1500, 1500, 1500, 1500},
  .ibus             = {{0, 0},
                       {0, 0}},
  .fw_info          = {0}
};

/// APIs: ->SetXXXX ->OnXXXX ->GetXXXX
/// For Rx Binding: void onFlySkyBindReceiver(uint8_t port);


void getFlySkyReceiverFirmwareRevision(uint8_t port, uint32_t * revision)
{
  uint8_t * fw_info = (uint8_t *) revision;
  fw_info[0] = rx_info.fw_info.fw_rev[0];
  fw_info[1] = rx_info.fw_info.fw_rev[1];
  fw_info[2] = rx_info.fw_info.fw_rev[2];
  fw_info[3] = rx_info.fw_info.fw_rev[3];
}

void getFlySkyTransmitterFirmwareRevision(uint8_t port, uint32_t * revision)
{
  uint8_t * fw_info = (uint8_t *) revision;
  fw_info[0] = rf_info.fw_info.fw_rev[0];
  fw_info[1] = rf_info.fw_info.fw_rev[1];
  fw_info[2] = rf_info.fw_info.fw_rev[2];
  fw_info[3] = rf_info.fw_info.fw_rev[3];
}

void getFlySkyFirmwareId(uint8_t port, bool is_receiver, uint32_t * firmware_id)
{
  uint8_t * fw_info = (uint8_t *) firmware_id;
  if (is_receiver) {
    fw_info[0] = rx_info.fw_info.fw_id[0];
    fw_info[1] = rx_info.fw_info.fw_id[1];
    fw_info[2] = rx_info.fw_info.fw_id[2];
    fw_info[3] = rx_info.fw_info.fw_id[3];
  }
  else {
    fw_info[0] = rf_info.fw_info.fw_id[0];
    fw_info[1] = rf_info.fw_info.fw_id[1];
    fw_info[2] = rf_info.fw_info.fw_id[2];
    fw_info[3] = rf_info.fw_info.fw_id[3];
  }
}

void getFlySkyHardwareRevision(uint8_t port, bool is_receiver, uint32_t * revision)
{
  uint8_t * fw_info = (uint8_t *) revision;
  if (is_receiver) {
    fw_info[0] = rx_info.fw_info.hw_rev[0];
    fw_info[1] = rx_info.fw_info.hw_rev[1];
    fw_info[2] = rx_info.fw_info.hw_rev[2];
    fw_info[3] = rx_info.fw_info.hw_rev[3];
  }
  else {
    fw_info[0] = rf_info.fw_info.hw_rev[0];
    fw_info[1] = rf_info.fw_info.hw_rev[1];
    fw_info[2] = rf_info.fw_info.hw_rev[2];
    fw_info[3] = rf_info.fw_info.hw_rev[3];
  }
}

void getFlySkyFirmwareRevision(uint8_t port, bool is_receiver, uint32_t * revision)
{
  if (is_receiver) getFlySkyReceiverFirmwareRevision(port, revision);
  else getFlySkyTransmitterFirmwareRevision(port, revision);
}


void setFlySkyGetFirmwarePackageAddr(uint8_t port, bool is_receiver, uint32_t * package_address)
{
  uint8_t * fw_package = (uint8_t *) package_address;
  if (is_receiver) {
    fw_package[0] = rx_info.fw_info.fw_pkg_addr[0];
    fw_package[1] = rx_info.fw_info.fw_pkg_addr[1];
    fw_package[2] = rx_info.fw_info.fw_pkg_addr[2];
    fw_package[3] = rx_info.fw_info.fw_pkg_addr[3];
  }
  else {
    fw_package[0] = rf_info.fw_info.fw_pkg_addr[0];
    fw_package[1] = rf_info.fw_info.fw_pkg_addr[1];
    fw_package[2] = rf_info.fw_info.fw_pkg_addr[2];
    fw_package[3] = rf_info.fw_info.fw_pkg_addr[3];
  }
}

void setFlySkyGetFirmwarePackageLen(uint8_t port, bool is_receiver, uint32_t * package_len)
{
  uint8_t * fw_package = (uint8_t *) package_len;
  if (is_receiver) {
    fw_package[0] = rx_info.fw_info.fw_pkg_len[0];
    fw_package[1] = rx_info.fw_info.fw_pkg_len[1];
    fw_package[2] = rx_info.fw_info.fw_pkg_len[2];
    fw_package[3] = rx_info.fw_info.fw_pkg_len[3];
  }
  else {
    fw_package[0] = rf_info.fw_info.fw_pkg_len[0];
    fw_package[1] = rf_info.fw_info.fw_pkg_len[1];
    fw_package[2] = rf_info.fw_info.fw_pkg_len[2];
    fw_package[3] = rf_info.fw_info.fw_pkg_len[3];
  }
}

void setFlySkyGetFirmwarePackageBuffer(uint8_t port, bool is_receiver, uint8_t * buffer)
{
  if (is_receiver) {
    rx_info.fw_info.pkg_data = buffer;
  }
  else {
    rf_info.fw_info.pkg_data = buffer;
  }
}


void setFlySkyTransmitterId(uint8_t port, uint32_t rf_id)
{
  rf_info.id[0] = rf_id & 0xff;
  rf_info.id[1] = rf_id & 0xff00 >> 8;
  rf_info.id[2] = rf_id & 0xff0000 >> 16;
  rf_info.id[3] = rf_id & 0xff000000 >> 24;
}

void setFlySkyTransmitterProtocol(uint8_t port, uint8_t protocol)
{
  rf_info.protocol = protocol;
}

void setFlySkyReceiverBindPowerLow(uint8_t port)
{
  rf_info.bind_power = BIND_LOW_POWER;
}

void setFlySkyReceiverBindPowerNormal(uint8_t port)
{
  rf_info.bind_power = BIND_NORMAL_POWER;
}

void setFlySkyReceiverChannelCounts(uint8_t port, uint8_t channle_counts)
{
  rf_info.num_of_channel = channle_counts;
}

void setFlySkyChannelDataFailSafe(uint8_t port)
{
  rf_info.channel_data_type = FLYSKY_CHANNEL_DATA_FAILSAFE;
}

void setFlySkyChannelDataNormalMode(uint8_t port)
{
  rf_info.channel_data_type = FLYSKY_CHANNEL_DATA_NORMAL;
}

void setFlySkyChannelOutputs(int channel, int16_t outValue)
{
  channelOutputs[channel] = outValue;
}

static uint32_t set_loop_cnt = 0;

void setFlySkyChannelData(int channel, int16_t servoValue)
{
  if (channel < NUM_OF_NV14_CHANNELS && VALID_CH_DATA(servoValue)) {
    rx_info.servo_value[channel] = (1000 * (servoValue + 1024) / 2048) + 1000;
  }

  if ((DEBUG_RF_FRAME_PRINT & TX_FRAME_ONLY) && (set_loop_cnt++ % 1000 == 0)) {
    TRACE_NOCRLF("HALL(%0d): ", FLYSKY_HALL_BAUDRATE);
    for (int idx = 0; idx < NUM_OF_NV14_CHANNELS; idx++) {
      TRACE_NOCRLF("CH%0d:%0d ", idx + 1, rx_info.servo_value[idx]);
    }
    TRACE(" ");
  }
}

void sendPulsesFrameByState(uint8_t port, uint8_t frameState)
{
  modulePulsesData[port].flysky.state = frameState;
  modulePulsesData[port].flysky.state_index = FLYSKY_MODULE_TIMEOUT;
  //setupPulsesFlySky(port); // effect immediately
}


bool isRxBindingState(uint8_t port)
{
  return moduleFlag[port] == MODULE_BIND;
}

bool isFlySkyUsbDownload(void)
{
  return rf_info.fw_state != 0;
}

void usbSetFrameTransmit(uint8_t packetID, uint8_t *dataBuf, uint32_t nBytes)
{
    // send to host via usb
    uint8_t *pt = (uint8_t*)&rfProtocolRx;
    rfProtocolRx.hallID.hall_Id.packetID = packetID;//0x08;
    rfProtocolRx.hallID.hall_Id.senderID = 0x03;
    rfProtocolRx.hallID.hall_Id.receiverID = 0x02;

    if ( packetID == 0x08 ) {
      uint8_t fwVerision[40];
      for(int idx = 40; idx > 0; idx--)
      {
          if ( idx <= nBytes ) {
              fwVerision[idx-1] = dataBuf[idx-1];
          }
          else fwVerision[idx-1] = 0;
      }
      dataBuf = fwVerision;
      nBytes = 40;
    }

    rfProtocolRx.length = nBytes;

    TRACE_NOCRLF("\r\nToUSB: 55 %02X %02X ", rfProtocolRx.hallID.ID, nBytes);
    for ( int idx = 0; idx < nBytes; idx++ )
    {
        rfProtocolRx.data[idx] = dataBuf[idx];
        TRACE_NOCRLF("%02X ", rfProtocolRx.data[idx]);
    }
#if !defined(SIMU)
    uint16_t checkSum = calc_crc16(pt, rfProtocolRx.length+3);
    TRACE(" CRC:%04X;", checkSum);

    pt[rfProtocolRx.length + 3] = checkSum & 0xFF;
    pt[rfProtocolRx.length + 4] = checkSum >> 8;

    usbDownloadTransmit(pt, rfProtocolRx.length + 5);
#endif
}


void onFlySkyModuleSetPower(uint8_t port, bool isPowerOn)
{
  if ( INTERNAL_MODULE == port )
  {
      if ( isPowerOn ) {
        INTERNAL_MODULE_ON();
        resetPulsesFlySky(port);
      }
      else {
        moduleFlag[port] = MODULE_NORMAL_MODE;
        INTERNAL_MODULE_OFF();
      }
  }
}

void onFlySkyReceiverRange(uint8_t port)
{
    resetPulsesFlySky(port);
    moduleFlag[port] = MODULE_RANGECHECK;
    modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_INIT;
}

void onFlySkyBindReceiver(uint8_t port)
{
  resetPulsesFlySky(port);
  moduleFlag[port] = MODULE_BIND;
  modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_INIT;
}

void onFlySkyReceiverPulseMode(uint8_t port)
{
  modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_SET_RX_PWM_PPM;
}

void onFlySkyReceiverPulsePort(uint8_t port)
{
  modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_SET_RX_IBUS_SBUS;
}

void onFlySkyReceiverSetFrequency(uint8_t port)
{
  sendPulsesFrameByState(port, FLYSKY_MODULE_STATE_SET_RX_FREQUENCY);
}

void onFlySkyReceiverSetPulse(uint8_t port, uint8_t mode_and_port) // mode_and_port = 0,1,2,3
{
  if ((DEBUG_RF_FRAME_PRINT & TX_FRAME_ONLY)) TRACE("PulseMode+Port: %0d", mode_and_port);
  sendPulsesFrameByState(port, FLYSKY_MODULE_STATE_SET_RX_PWM_PPM);
}

void onFlySkyTransmitterPower(uint8_t port, uint8_t dBmValue)
{
  tx_working_power = dBmValue;
  modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_SET_TX_POWER;
}


void onFlySkyUpdateReceiverFirmwareStart(uint8_t port)
{
  modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_UPDATE_RX_FIRMWARE;
}

void onFlySkyUsbDownloadFirmware(uint8_t port, uint8_t isRfTransfer)
{
  if ( isRfTransfer != 0 )
    modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_UPDATE_RF_FIRMWARE;
  else modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_UPDATE_RX_FIRMWARE;
}

void onFlySkyUsbDownloadStart(uint8_t fw_state)
{
  rf_info.fw_state = fw_state;
}

void onFlySkyUpdateTransmitterProtocol(uint8_t port)
{
  modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_UPDATE_RF_PROTOCOL;
}


void onFlySkyGetVersionInfoStart(uint8_t port, uint8_t isRfTransfer)
{
  lastState = modulePulsesData[port].flysky.state;
  if ( isRfTransfer != 0 )
    modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_GET_RF_VERSION_INFO;
  else modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_GET_RX_VERSION_INFO;
}

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

void putFlySkyFrameHead(uint8_t port)
{
  *modulePulsesData[port].flysky.ptr++ = END;
}

void putFlySkyFrameIndex(uint8_t port)
{
  putFlySkyFrameByte(port, modulePulsesData[port].flysky.frame_index);
}

void putFlySkyFrameCrc(uint8_t port)
{
  putFlySkyByte(port, modulePulsesData[port].flysky.crc ^ 0xff);
}

void putFlySkyFrameTail(uint8_t port)
{
  *modulePulsesData[port].flysky.ptr++ = END;
}


void putFlySkyRfInit(uint8_t port)
{
  putFlySkyFrameByte(port, FRAME_TYPE_REQUEST_ACK);
  putFlySkyFrameByte(port, COMMAND_ID_RF_INIT);
}

void putFlySkyBindReceiver(uint8_t port)
{
  putFlySkyFrameByte(port, FRAME_TYPE_REQUEST_ACK);
  putFlySkyFrameByte(port, COMMAND_ID_BIND);
  putFlySkyFrameByte(port, rf_info.bind_power);
  putFlySkyFrameByte(port, rf_info.id[0]);
  putFlySkyFrameByte(port, rf_info.id[1]);
  putFlySkyFrameByte(port, rf_info.id[2]);
  putFlySkyFrameByte(port, rf_info.id[3]);
}

void putFlySkySetReceiverID(uint8_t port)
{
  putFlySkyFrameByte(port, FRAME_TYPE_REQUEST_ACK);
  putFlySkyFrameByte(port, COMMAND_ID_SET_RECEIVER_ID);
  putFlySkyFrameByte(port, gRomData.rx_id[0]); // receiver ID (byte 0)
  putFlySkyFrameByte(port, gRomData.rx_id[1]); // receiver ID (byte 1)
  putFlySkyFrameByte(port, gRomData.rx_id[2]); // receiver ID (byte 2)
  putFlySkyFrameByte(port, gRomData.rx_id[3]); // receiver ID (byte 3)
}

void putFlySkyGetReceiverConfig(uint8_t port)
{
  putFlySkyFrameByte(port, FRAME_TYPE_ANSWER);
  putFlySkyFrameByte(port, COMMAND_ID_RF_GET_CONFIG);
  putFlySkyFrameByte(port, GET_FLYSKY_PWM_PPM);  // 00:PWM, 01:PPM
  putFlySkyFrameByte(port, GET_FLYSKY_IBUS_SBUS);// 00:I-BUS, 01:S-BUS
  putFlySkyFrameByte(port, gRomData.rx_freq[0] < 50 ? 50 : gRomData.rx_freq[0]); // receiver servo freq bit[7:0]
  putFlySkyFrameByte(port, gRomData.rx_freq[1]); // receiver servo freq bit[15:8]
}

void putFlySkySetReceiverPulseMode(uint8_t port)
{
  putFlySkyFrameByte(port, FRAME_TYPE_REQUEST_ACK);
  putFlySkyFrameByte(port, COMMAND_ID_SET_RX_PWM_PPM);
  putFlySkyFrameByte(port, GET_FLYSKY_PWM_PPM); // 00:PWM, 01:PPM
}

void putFlySkySetReceiverPort(uint8_t port)
{
  putFlySkyFrameByte(port, FRAME_TYPE_REQUEST_ACK);
  putFlySkyFrameByte(port, COMMAND_ID_SET_RX_IBUS_SBUS);
  putFlySkyFrameByte(port, GET_FLYSKY_IBUS_SBUS); // 0x00:I-BUS, 0x01:S-BUS
}

void putFlySkySetReceiverServoFreq(uint8_t port)
{
  putFlySkyFrameByte(port, FRAME_TYPE_REQUEST_ACK);
  putFlySkyFrameByte(port, COMMAND_ID_SET_RX_SERVO_FREQ);
  putFlySkyFrameByte(port, gRomData.rx_freq[0]); // receiver servo freq bit[7:0]
  putFlySkyFrameByte(port, gRomData.rx_freq[1]); // receiver servo freq bit[15:8]
}

void putFlySkySetPowerdBm(uint8_t port, uint8_t dBm)
{
  putFlySkyFrameByte(port, FRAME_TYPE_REQUEST_ACK);
  putFlySkyFrameByte(port, COMMAND_ID0D_SET_TX_POWER);
  putFlySkyFrameByte(port, dBm); // 0x00:RX firmware, 0x01:RF firmware
}

void putFlySkyGetFirmwareVersion(uint8_t port, uint8_t fw_word)
{
  putFlySkyFrameByte(port, FRAME_TYPE_REQUEST_ACK);
  putFlySkyFrameByte(port, COMMAND_ID_GET_VERSION_INFO);
  putFlySkyFrameByte(port, fw_word); // 0x00:RX firmware, 0x01:RF firmware
}

void putFlySkySendChannelData(uint8_t port)
{
  uint16_t pulseValue = 0;
  uint8_t channels_start = g_model.moduleData[port].channelsStart;
  uint8_t channels_count = min<unsigned int>(NUM_OF_NV14_CHANNELS, channels_start + 8 + g_model.moduleData[port].channelsCount);

  putFlySkyFrameByte(port, FRAME_TYPE_REQUEST_NACK);
  putFlySkyFrameByte(port, COMMAND_ID_SEND_CHANNEL_DATA);

  if ( failsafeCounter[port]-- == 0 ) {
    failsafeCounter[port] = FAILSAVE_SEND_COUNTER_MAX;
    putFlySkyFrameByte(port, 0x01);
    putFlySkyFrameByte(port, NUM_OF_NV14_CHANNELS/*channels_count*/);
    for (uint8_t channel = channels_start; channel < channels_count; channel++) {
      if ( g_model.moduleData[port].failsafeMode == FAILSAFE_CUSTOM) {
          int16_t failsafeValue = g_model.moduleData[port].failsafeChannels[channel];
          pulseValue = limit<uint16_t>(900, 900 + ((2100 - 900) * (failsafeValue + 1024) / 2048), 2100);
      }
      else {
          pulseValue = 0xfff;
      }
      putFlySkyFrameByte(port, pulseValue & 0xff);
      putFlySkyFrameByte(port, pulseValue >> 8);
    }
    if (DEBUG_RF_FRAME_PRINT & RF_FRAME_ONLY) {
        TRACE("------FAILSAFE------");
    }
  }
  else {
    putFlySkyFrameByte(port, 0x00);
    putFlySkyFrameByte(port, channels_count);
    for (uint8_t channel = channels_start; channel < channels_count; channel++) {
      int channelValue = channelOutputs[channel] + 2*PPM_CH_CENTER(channel) - 2*PPM_CENTER;
      pulseValue = limit<uint16_t>(900, 900 + ((2100 - 900) * (channelValue + 1024) / 2048), 2100);
      putFlySkyFrameByte(port, pulseValue & 0xff);
      putFlySkyFrameByte(port, pulseValue >> 8);
    }
  }
}

void putFlySkyUpdateFirmwareStart(uint8_t port, uint8_t fw_word)
{
  putFlySkyFrameByte(port, FRAME_TYPE_REQUEST_ACK);
  if (fw_word == FLYSKY_RX_FIRMWARE) {
    fw_word = COMMAND_ID20_UPDATE_RX_FIRMWARE;
  }
  else {
    fw_word = COMMAND_ID0C_UPDATE_RF_FIRMWARE;
  }
  putFlySkyFrameByte(port, fw_word);
}

void putFlySkyUpdateRfProtocol(uint8_t port)
{
  putFlySkyFrameByte(port, FRAME_TYPE_REQUEST_ACK);
  putFlySkyFrameByte(port, COMMAND_ID_SET_RF_PROTOCOL);
  putFlySkyFrameByte(port, rf_info.protocol); // 0x00:AFHDS1 0x01:AFHDS2 0x02:AFHDS2A
}


void incrFlySkyFrame(uint8_t port)
{
  if (++modulePulsesData[port].flysky.frame_index == 0)
    modulePulsesData[port].flysky.frame_index = 1;
}

bool checkFlySkyFrameCrc(const uint8_t * ptr, uint8_t size)
{
  uint8_t crc = 0;

  for (uint8_t i = 0; i < size; i++) {
    crc += ptr[i];
  }

  if (DEBUG_RF_FRAME_PRINT & RF_FRAME_ONLY) {
    if (ptr[2] != 0x06 || (set_loop_cnt++ % 50 == 0)) {
      TRACE_NOCRLF("RF(%0d): C0", INTERNAL_MODULE_BAUDRATE);
      for (int idx = 0; idx <= size; idx++) {
        TRACE_NOCRLF(" %02X", ptr[idx]);
      }
      TRACE(" C0;");

      if ((crc ^ 0xff) != ptr[size]) {
        TRACE("ErrorCRC %02X especting %02X", crc ^ 0xFF, ptr[size]);
      }
    }
  }

  return (crc ^ 0xff) == ptr[size];
}


void parseFlySkyFeedbackFrame(uint8_t port)
{
  const uint8_t * ptr = modulePulsesData[port].flysky.telemetry;
  uint8_t dataLen = modulePulsesData[port].flysky.telemetry_index;
  if (*ptr++ != END || dataLen < 2 )
    return;

  uint8_t frame_number = *ptr++;
  uint8_t frame_type = *ptr++;
  uint8_t command_id = *ptr++;
  uint8_t first_para = *ptr++;
  uint8_t * p_data = NULL;

  dataLen -= 2;
  if (!checkFlySkyFrameCrc(modulePulsesData[port].flysky.telemetry + 1, dataLen)) {
    return;
  }

  if ( (moduleFlag[port] != MODULE_BIND ) && (frame_type == FRAME_TYPE_ANSWER)
       && (modulePulsesData[port].flysky.frame_index -1) != frame_number ) {
      return;
  }
  else if ( frame_type == FRAME_TYPE_REQUEST_ACK ) {
     modulePulsesData[port].flysky.frame_index = frame_number;
  }

  switch (command_id) {
    default:
      if (moduleFlag[port] == MODULE_NORMAL_MODE
          && modulePulsesData[port].flysky.state >= FLYSKY_MODULE_STATE_IDLE) {
        modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_DEFAULT;
        if (DEBUG_RF_FRAME_PRINT & RF_FRAME_ONLY) TRACE("State back to channel data");
      }
      break;

    case COMMAND_ID_RF_INIT: {
      if (first_para == 0x01) { // action only RF ready

          if ( moduleFlag[port] == MODULE_BIND ) {
            modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_BIND;
          }

          else if (moduleFlag[port] == MODULE_RANGECHECK && tx_working_power != 0) {
            onFlySkyTransmitterPower(port, 0);
            break;
          }

          if (modulePulsesData[port].flysky.state == FLYSKY_MODULE_STATE_INIT) {
            modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_SET_RECEIVER_ID;
          }
      } else modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_IDLE;
      break; }

    case COMMAND_ID_BIND: {
      if (frame_type != FRAME_TYPE_ANSWER) {
        modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_IDLE;
        return;
      }
      if ( moduleFlag[port] == MODULE_BIND ) {
        moduleFlag[port] = MODULE_NORMAL_MODE;
      }
      g_model.header.modelId[port] = ptr[2];
      gRomData.rx_id[0] = first_para;
      gRomData.rx_id[1] = *ptr++;
      gRomData.rx_id[2] = *ptr++;
      gRomData.rx_id[3] = *ptr++;
      if (DEBUG_RF_FRAME_PRINT & RF_FRAME_ONLY)
        TRACE("New Rx ID: %02X %02X %02X %02X", gRomData.rx_id[0], gRomData.rx_id[1], gRomData.rx_id[2], gRomData.rx_id[3]);
      SET_DIRTY();
      resetPulsesFlySky(port);
      modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_INIT;
      break;
    }

    case COMMAND_ID_RF_GET_CONFIG: {
      sendPulsesFrameByState(port, FLYSKY_MODULE_STATE_GET_RECEIVER_CONFIG);
      break;
    }

    case COMMAND_ID_RX_SENSOR_DATA: {
      flySkyNv14ProcessTelemetryPacket(ptr, first_para );
      if (moduleFlag[port] == MODULE_NORMAL_MODE && modulePulsesData[port].flysky.state >= FLYSKY_MODULE_STATE_IDLE) {
        modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_DEFAULT;
      }
      break;
    }

    case COMMAND_ID_SET_RECEIVER_ID: {
      modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_DEFAULT;
      return;
    }

    case COMMAND_ID0D_SET_TX_POWER: {
      modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_INIT;
      break;
    }

    case COMMAND_ID_SET_RX_PWM_PPM: {
      modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_SET_RX_IBUS_SBUS;
      break;
    }

    case COMMAND_ID_SET_RX_IBUS_SBUS: {
      modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_SET_RX_FREQUENCY;
      break;
    }

    case COMMAND_ID_SET_RX_SERVO_FREQ: {
      modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_DEFAULT;
      break;
    }

    case COMMAND_ID0C_UPDATE_RF_FIRMWARE: {
      rf_info.fw_state = FLYSKY_MODULE_STATE_UPDATE_RF_FIRMWARE;
      modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_IDLE;
      break;
    }

    case COMMAND_ID_GET_VERSION_INFO: {
      if ( dataLen > 4 ) {
        usbSetFrameTransmit(0x08, (uint8_t*)ptr, dataLen - 4 );
      }
      if ( lastState == FLYSKY_MODULE_STATE_GET_RF_VERSION_INFO ||
           lastState == FLYSKY_MODULE_STATE_GET_RX_VERSION_INFO ) {
        lastState = FLYSKY_MODULE_STATE_INIT;
      }
      modulePulsesData[port].flysky.state = lastState;
      break;
    }
  }
}

bool isRfProtocolRxMsgOK(void)
{
  bool isMsgOK = (0 != rfRxCount);
  rfRxCount = 0;
  return isMsgOK && isFlySkyUsbDownload();
}

#if !defined(SIMU)
void checkFlySkyFeedback(uint8_t port)
{
  uint8_t byte;

  while (intmoduleGetByte(&byte)) {
    //if ( modulePulsesData[port].flysky.state == FLYSKY_MODULE_STATE_IDLE
    //  && rf_info.fw_state == FLYSKY_MODULE_STATE_UPDATE_RF_FIRMWARE )
    {
        Parse_Character(&rfProtocolRx, byte );
        if ( rfProtocolRx.msg_OK )
        {
            rfRxCount++;
            rfProtocolRx.msg_OK = 0;
            uint8_t *pt = (uint8_t*)&rfProtocolRx;
            pt[rfProtocolRx.length + 3] = rfProtocolRx.checkSum & 0xFF;
            pt[rfProtocolRx.length + 4] = rfProtocolRx.checkSum >> 8;

            if((DEBUG_RF_FRAME_PRINT & RF_FRAME_ONLY)) {
#if !defined(SIMU)
                TRACE("RF: %02X %02X %02X ...%04X; CRC:%04X", pt[0], pt[1], pt[2],
                      rfProtocolRx.checkSum, calc_crc16(pt, rfProtocolRx.length+3));
#endif
            }

            if ( 0x01 == rfProtocolRx.length &&
               ( 0x05 == rfProtocolRx.data[0] || 0x06 == rfProtocolRx.data[0]) )
            {
                modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_INIT;
                rf_info.fw_state = 0;
            }
#if !defined(SIMU)
            usbDownloadTransmit(pt, rfProtocolRx.length + 5);
#endif
        }
        //continue;
    }

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
#endif

void resetPulsesFlySky(uint8_t port)
{
  modulePulsesData[port].flysky.frame_index = 1;
  modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_SET_TX_POWER;
  modulePulsesData[port].flysky.state_index = 0;
  modulePulsesData[port].flysky.esc_state = 0;
  tx_working_power = 90; // 17dBm
  uint16_t rx_freq = g_model.moduleData[port].romData.rx_freq[0];
  rx_freq += (g_model.moduleData[port].romData.rx_freq[1] * 256);
  if (50 > rx_freq || 400 < rx_freq) {
    g_model.moduleData[port].romData.rx_freq[0] = 50;
  }
}

void setupPulsesFlySky(uint8_t port)
{
#if !defined(SIMU)
  checkFlySkyFeedback(port);
#endif

  initFlySkyArray(port);
  putFlySkyFrameHead(port);
  putFlySkyFrameIndex(port);

  if (modulePulsesData[port].flysky.state < FLYSKY_MODULE_STATE_DEFAULT) {

    if (++modulePulsesData[port].flysky.state_index >= FLYSKY_MODULE_TIMEOUT / PXX_PERIOD_DURATION) {

      modulePulsesData[port].flysky.state_index = 0;

      switch (modulePulsesData[port].flysky.state) {

        case FLYSKY_MODULE_STATE_INIT:
          putFlySkyRfInit(port);
          break;

        case FLYSKY_MODULE_STATE_BIND:
          putFlySkyBindReceiver(port);
          break;

        case FLYSKY_MODULE_STATE_SET_RECEIVER_ID:
          putFlySkySetReceiverID(port);
          break;

        case FLYSKY_MODULE_STATE_GET_RECEIVER_CONFIG:
          putFlySkyGetReceiverConfig(port);
          modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_INIT;
          break;

        case FLYSKY_MODULE_STATE_SET_TX_POWER:
          putFlySkySetPowerdBm(port, tx_working_power);
          break;

        case FLYSKY_MODULE_STATE_SET_RX_PWM_PPM:
          putFlySkySetReceiverPulseMode(port);
          break;

        case FLYSKY_MODULE_STATE_SET_RX_IBUS_SBUS:
          putFlySkySetReceiverPort(port);
          break;

        case FLYSKY_MODULE_STATE_SET_RX_FREQUENCY:
          putFlySkySetReceiverServoFreq(port);
          break;

        case FLYSKY_MODULE_STATE_UPDATE_RF_PROTOCOL:
          putFlySkyUpdateRfProtocol(port);
          break;

        case FLYSKY_MODULE_STATE_UPDATE_RX_FIRMWARE:
          putFlySkyUpdateFirmwareStart(port, FLYSKY_RX_FIRMWARE);
          break;

        case FLYSKY_MODULE_STATE_UPDATE_RF_FIRMWARE:
          putFlySkyUpdateFirmwareStart(port, FLYSKY_RF_FIRMWARE);
          break;

        case FLYSKY_MODULE_STATE_GET_RX_VERSION_INFO:
          putFlySkyGetFirmwareVersion(port, FLYSKY_RX_FIRMWARE);
          break;

        case FLYSKY_MODULE_STATE_GET_RF_VERSION_INFO:
          putFlySkyGetFirmwareVersion(port, FLYSKY_RF_FIRMWARE);
          break;

        case FLYSKY_MODULE_STATE_IDLE:
          initFlySkyArray(port);
          return;

        default:
          modulePulsesData[port].flysky.state = FLYSKY_MODULE_STATE_INIT;
          initFlySkyArray(port);
          if ((DEBUG_RF_FRAME_PRINT & TX_FRAME_ONLY)) {
            TRACE("State back to INIT\r\n");
          }
          return;
      }
    }
    else {
      initFlySkyArray(port);
      return;
    }
  }
  else {
    if ( moduleFlag[port] == MODULE_BIND )
      moduleFlag[port] = MODULE_NORMAL_MODE;
    putFlySkySendChannelData(port);
  }

  incrFlySkyFrame(port);

  putFlySkyFrameCrc(port);
  putFlySkyFrameTail(port);

  if ((DEBUG_RF_FRAME_PRINT & TX_FRAME_ONLY)) {
    /* print each command, except channel data by interval */
    uint8_t * data = modulePulsesData[port].pxx_uart.pulses;
    if (data[3] != COMMAND_ID_SEND_CHANNEL_DATA || (set_loop_cnt++ % 100 == 0)) {
      uint8_t size = modulePulsesData[port].pxx_uart.ptr - data;
      TRACE_NOCRLF("TX(State%0d)%0dB:", modulePulsesData[port].flysky.state, size);
      for (int idx = 0; idx < size; idx++) {
        TRACE_NOCRLF(" %02X", data[idx]);
      }
      TRACE(";");
    }
  }
}

#if !defined(SIMU)
void usbDownloadTransmit(uint8_t *buffer, uint32_t size)
{
    if (USB_SERIAL_MODE != getSelectedUsbMode()) return;

    for (int idx = 0; idx < size; idx++)
    {
        usbSerialPutc(buffer[idx]);
    }
}
#endif
