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

void gimbalsInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef SPI_InitStructure;
   
  GPIO_InitStructure.GPIO_Pin = GIMBALS_MOSI_PIN | GIMBALS_MISO_PIN | GIMBALS_SCK_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GIMBALS_SPI_GPIO, &GPIO_InitStructure);
  GPIO_PinAFConfig(GIMBALS_SPI_GPIO, GIMBALS_MOSI_PinSource | GIMBALS_MISO_PinSource | GIMBALS_SCK_PinSource, GIMBALS_GPIO_AF);

  GPIO_InitStructure.GPIO_Pin = GIMBALS_LEFT_CS_GPIO_PIN | GIMBALS_RIGHT_CS_GPIO_PIN;
  GPIO_Init(GIMBALS_CS_INT_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GIMBALS_LEFT_INT_GPIO_PIN | GIMBALS_RIGHT_INT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  // TODO: interrupt needed?
  GPIO_Init(GIMBALS_CS_INT_GPIO, &GPIO_InitStructure);
  
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_Init(GIMBALS_SPI, &SPI_InitStructure);
  SPI_Cmd(GIMBALS_SPI, ENABLE);
}

void gimbalsRead(void)
{
  
}