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

void intmoduleStop(void);
void extmoduleStop(void);

void intmoduleNoneStart(void);
void intmoduleStart(void);
#if defined(TARANIS_INTERNAL_PPM)
void intmodulePpmStart(void);
#endif

void extmoduleNoneStart(void);
void extmodulePpmStart(void);
void extmodulePxxStart(void);
#if defined(DSM2) || defined(MULTIMODULE)
void extmoduleDsm2Start(void);
#endif
void extmoduleCrossfireStart(void);

void intmoduleSendNextFrame();

void init_pxx(uint32_t port)
{
  if (port == EXTERNAL_MODULE)
    extmodulePxxStart();
}

void disable_pxx(uint32_t port)
{
  if (port == INTERNAL_MODULE)
    intmoduleStop();
  else
    extmoduleStop();
}

#if defined(DSM2)
void init_dsm2(uint32_t port)
{
  if (port == EXTERNAL_MODULE) {
    extmoduleDsm2Start();
  }
}

void disable_dsm2(uint32_t port)
{
  if (port == EXTERNAL_MODULE) {
    extmoduleStop();
  }
}
#endif

void init_sbusOut(uint32_t port)
{
  init_dsm2(port);
}

void disable_sbusOut(uint32_t port)
{
  disable_dsm2(port);
}


void init_ppm(uint32_t port)
{
  if (port == EXTERNAL_MODULE) {
    extmodulePpmStart();
  }
}

void disable_ppm(uint32_t port)
{
  if (port == EXTERNAL_MODULE) {
    extmoduleStop();
  }
#if defined(TARANIS_INTERNAL_PPM)
  else if (port == INTERNAL_MODULE) {
    intmoduleStop();
  }
#endif
}

void init_no_pulses(uint32_t port)
{
  if (port == INTERNAL_MODULE)
    intmoduleNoneStart();
  else
    extmoduleNoneStart();
}

void disable_no_pulses(uint32_t port)
{
  if (port == INTERNAL_MODULE)
    intmoduleStop();
  else
    extmoduleStop();
}

void init_crossfire(uint32_t port)
{
  if (port == EXTERNAL_MODULE) {
    extmoduleCrossfireStart();
  }
}

void disable_crossfire(uint32_t port)
{
  if (port == EXTERNAL_MODULE) {
    extmoduleStop();
  }
}

void intmoduleStop()
{
  INTERNAL_MODULE_OFF();

  NVIC_DisableIRQ(INTMODULE_TX_DMA_IRQn);

  INTMODULE_TX_DMA_STREAM->CR &= ~DMA_SxCR_EN; // Disable DMA
}

void intmoduleNoneStart()
{

}

void intmoduleStart()
{
  INTERNAL_MODULE_ON();

  GPIO_PinAFConfig(INTMODULE_GPIO, INTMODULE_GPIO_PinSources , INTMODULE_GPIO_AF);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = INTMODULE_GPIOB_PINS;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(INTMODULE_GPIO, &GPIO_InitStructure);

  intmoduleSendNextFrame();

  NVIC_EnableIRQ(INTMODULE_TX_DMA_IRQn);
  NVIC_SetPriority(INTMODULE_TX_DMA_IRQn, 7);
}

void intmoduleSendNextFrame()
{


}

extern "C" void INTMODULE_TX_DMA_IRQHandler()
{
  if (!DMA_GetITStatus(INTMODULE_TX_DMA_STREAM, INTMODULE_TX_DMA_FLAG_TC))
    return;

  DMA_ClearITPendingBit(INTMODULE_TX_DMA_STREAM, INTMODULE_TX_DMA_FLAG_TC);


}
