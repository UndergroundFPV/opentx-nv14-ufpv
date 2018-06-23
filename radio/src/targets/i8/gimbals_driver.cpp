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

#define GIMBAL_BUFFER_LEN        5

enum gimbals {
  GIMBAL_LEFT,
  GIMBAL_RIGHT
};

// Commands
#define MLX90393_CMD_NOP         0x00
#define MLX90393_CMD_RESET       0xF0
#define MLX90393_CMD_MEM_RCL     0xD0
#define MLX90393_CMD_MEM_STO     0xE0
#define MLX90393_CMD_REG_RD      0x50
#define MLX90393_CMD_REG_WR      0x60
#define MLX90393_CMD_BURST_START 0x10    // OR with channels to read
#define MLX90393_CMD_BURST_STOP  0x80
#define MLX90393_CMD_SNGLE_MEAS  0x30    // OR with channels to read
#define MLX90393_CMD_READ_MEAS   0x40    // OR with channels to read
#define MLX90393_CMD_WOC_ENABLE  0x20    // OR with channels to read

// Channel flags
#define MLX90393_CHANNEL_T       0x01
#define MLX90393_CHANNEL_X       0x02
#define MLX90393_CHANNEL_Y       0x04
#define MLX90393_CHANNEL_Z       0x08

// Status flags
#define MLX90393_STATUS_BURST    0x80
#define MLX90393_STATUS_WOC      0x40
#define MLX90393_STATUS_SINGLE   0x20
#define MLX90393_STATUS_ERROR    0x10
#define MLX90393_STATUS_SED      0x08
#define MLX90393_STATUS_RES      0x04
#define MLX90393_STATUS_NBYTES   0x03    // Mask for number of bytes to read

// Registers
#define MLX90393_REG_CR0         0x00
#define MLX90393_REG_CR1         0x01
#define MLX90393_REG_CR2         0x02
#define MLX90393_REG_CR3         0x03
#define MLX90393_REG_XOFFSET     0x04
#define MLX90393_REG_YOFFSET     0x05
#define MLX90393_REG_ZOFFSET     0x06
#define MLX90393_REG_XOFFSET     0x04

#define MLX90393_CR0_HALLCONF     0x00  // 4 bits, "recommended value 0x0C"
#define MLX90393_CR0_GAIN_SEL     0x04  // 3 bits
#define MLX90393_CR1_TRIG_INT_SEL 0x0F  // 1 bit
#define MLX90393_CR1_COMM_MODE    0x0D  // 2 bits
#define MLX90393_CR1_TCMP_EN      0x0A  // 1 bit

// Note: The chip will not work properly with the following combinations: OSR=0 and DIG_FILT=0, OSR=0 and DIG_FILT=1 and OSR=1 and DIG_FILT=0
#define MLX90393_CR2_OSR          0x00  // 2 bits
#define MLX90393_CR2_DIG_FILT     0x02  // 3 bits
#define MLX90393_CR2_RES_X        0x05  // 2 bits
#define MLX90393_CR2_RES_Y        0x07  // 2 bits
#define MLX90393_CR2_RES_Z        0x09  // 2 bits
#define MLX90393_CR2_OSR2         0x0B  // 2 bits

#define GIMBALS_NCS_HIGH(x)      (GIMBALS_CS_INT_GPIO->BSRRL = (x == GIMBAL_LEFT ? GIMBALS_LEFT_CS_GPIO_PIN : GIMBALS_RIGHT_CS_GPIO_PIN))
#define GIMBALS_NCS_LOW(x)       (GIMBALS_CS_INT_GPIO->BSRRH = (x == GIMBAL_LEFT ? GIMBALS_LEFT_CS_GPIO_PIN : GIMBALS_RIGHT_CS_GPIO_PIN))

uint8_t gimbalSendPacket(uint8_t * packet, uint8_t len, uint8_t target)
{
  GIMBALS_NCS_LOW(target);

  for (uint8_t i = 0; i <= len; i++) {
    while ((GIMBALS_SPI->SR & SPI_SR_TXE) == 0) {
      // Wait
    }
    (void) GIMBALS_SPI->DR; // Clear receive
    GIMBALS_SPI->DR = (i == len ? MLX90393_CMD_NOP : packet[i]);
    while ((GIMBALS_SPI->SR & SPI_SR_RXNE) == 0) {
      // Wait
    }
  }
  GIMBALS_NCS_HIGH(target);
  return GIMBALS_SPI->DR;
}

uint8_t gimbalReceivePacket(uint8_t * packet, uint8_t len, uint8_t target)
{
  uint8_t i;

  GIMBALS_NCS_LOW(target);

  for (i = 0 ; i <= len; i++) {
    while ((GIMBALS_SPI->SR & SPI_SR_TXE) == 0) {
      // Wait
    }
    (void) GIMBALS_SPI->DR; // Clear receive
    GIMBALS_SPI->DR = MLX90393_CMD_NOP;
    while ((GIMBALS_SPI->SR & SPI_SR_RXNE) == 0) {
      // Wait
    }
    // Skip first dummy byte
    if (i > 0) {
      packet[i-1] = GIMBALS_SPI->DR;
    }
  }
  GIMBALS_NCS_HIGH(target);
  return packet[0];
}

uint16_t gimbalReadReg(uint8_t reg, uint8_t target)
{
  uint8_t txBuffer[GIMBAL_BUFFER_LEN] = {0};
  uint8_t rxBuffer[GIMBAL_BUFFER_LEN] = {0};

  txBuffer[0] = MLX90393_CMD_REG_RD;
  txBuffer[1] = reg << 2;

  gimbalSendPacket(txBuffer, 2, target);
  gimbalReceivePacket(rxBuffer, GIMBAL_BUFFER_LEN, target);

  return rxBuffer[1]<<8 | rxBuffer[2];
}

uint8_t gimbalWriteReg(uint8_t reg, uint16_t data, uint8_t target)
{
  uint8_t txBuffer[GIMBAL_BUFFER_LEN] = {0};

  txBuffer[0] = MLX90393_CMD_REG_WR;
  txBuffer[1] = (uint8_t)((data >> 8) & 0xFF);
  txBuffer[2] = (uint8_t)(data & 0xFF);
  txBuffer[3] = reg << 2;
  return gimbalSendPacket(txBuffer, 4, target);;
}

uint8_t gimbalSendCommand(uint8_t command, uint8_t target)
{
  return gimbalSendPacket(&command, 1, target);
}

void gimbalsInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GIMBALS_SPI->SR = 0; // Clear any mode error
  GIMBALS_SPI->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_CPOL | SPI_CR1_CPHA;
  GIMBALS_SPI->CR1 |= SPI_CR1_BR_2 | SPI_CR1_BR_0; // APB2 clock / 64 = 380uS per clock
  GIMBALS_SPI->CR2 = 0;
  GIMBALS_SPI->CR1 |= SPI_CR1_MSTR;        // Make sure in case SSM/SSI needed to be set first
  GIMBALS_SPI->CR1 |= SPI_CR1_SPE;

  GPIO_InitStructure.GPIO_Pin = GIMBALS_MOSI_PIN | GIMBALS_MISO_PIN | GIMBALS_SCK_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GIMBALS_SPI_GPIO, &GPIO_InitStructure);

  GPIO_PinAFConfig(GIMBALS_SPI_GPIO, GIMBALS_MOSI_PinSource, GIMBALS_GPIO_AF);
  GPIO_PinAFConfig(GIMBALS_SPI_GPIO, GIMBALS_MISO_PinSource, GIMBALS_GPIO_AF);
  GPIO_PinAFConfig(GIMBALS_SPI_GPIO, GIMBALS_SCK_PinSource, GIMBALS_GPIO_AF);

  GPIO_InitStructure.GPIO_Pin = GIMBALS_LEFT_CS_GPIO_PIN | GIMBALS_RIGHT_CS_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_Init(GIMBALS_CS_INT_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GIMBALS_LEFT_INT_GPIO_PIN | GIMBALS_RIGHT_INT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  // TODO: interrupt needed?
  GPIO_Init(GIMBALS_CS_INT_GPIO, &GPIO_InitStructure);

  GIMBALS_CS_INT_GPIO->BSRRL = GIMBALS_LEFT_CS_GPIO_PIN | GIMBALS_RIGHT_CS_GPIO_PIN;

  for (uint8_t i = GIMBAL_LEFT; i <= GIMBAL_RIGHT; i++) {
    gimbalWriteReg(MLX90393_REG_CR0,
      (0x05 << MLX90393_CR0_GAIN_SEL) |
      (0x0C << MLX90393_CR0_HALLCONF), i);

    gimbalWriteReg(MLX90393_REG_CR1,
      (0x01 << MLX90393_CR1_TRIG_INT_SEL) |
      (0x02 << MLX90393_CR1_COMM_MODE) |
      (0x00 << MLX90393_CR1_TCMP_EN), i);

    gimbalWriteReg(MLX90393_REG_CR2,
      (0x00 << MLX90393_CR2_OSR2) |
      (0x02 << MLX90393_CR2_RES_Z) |
      (0x02 << MLX90393_CR2_RES_Y) |
      (0x02 << MLX90393_CR2_RES_X) |
      (0x02 << MLX90393_CR2_DIG_FILT) |
      (0x00 << MLX90393_CR2_OSR), i);

    gimbalSendCommand(MLX90393_CMD_MEM_STO, i);
  }
}

void gimbalsRead(uint16_t * values)
{
  uint8_t rxBuffer[GIMBAL_BUFFER_LEN] = {0};
  
  gimbalSendCommand(MLX90393_CMD_SNGLE_MEAS | MLX90393_CHANNEL_X | MLX90393_CHANNEL_Y, GIMBAL_LEFT);
  gimbalSendCommand(MLX90393_CMD_SNGLE_MEAS | MLX90393_CHANNEL_X | MLX90393_CHANNEL_Y, GIMBAL_RIGHT);

  while (gimbalSendCommand(MLX90393_CMD_NOP, GIMBAL_LEFT) & MLX90393_STATUS_SINGLE) {
    // wait
  }
  while (gimbalSendCommand(MLX90393_CMD_NOP, GIMBAL_RIGHT) & MLX90393_STATUS_SINGLE) {
    // wait
  }

  for (uint8_t i = GIMBAL_LEFT; i <= GIMBAL_RIGHT; i++) {
    gimbalSendCommand(MLX90393_CMD_READ_MEAS | MLX90393_CHANNEL_X | MLX90393_CHANNEL_Y, i);
    gimbalReceivePacket(rxBuffer, GIMBAL_BUFFER_LEN, i);

    values[2*i]     = ((rxBuffer[1]<<8) | rxBuffer[2]) >> 4;
    values[(2*i)+1] = ((rxBuffer[3]<<8) | rxBuffer[4]) >> 4;
  }
}
