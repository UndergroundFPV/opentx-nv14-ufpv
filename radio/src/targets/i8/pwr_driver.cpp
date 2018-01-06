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

void pwrInit()
{
  // if any changes are done to the PWR PIN or pwrOn() function
  // then the same changes must be done in _bootStart()

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = PWR_ON_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(PWR_ON_GPIO, &GPIO_InitStructure);
  
  GPIO_ResetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN);
  GPIO_InitStructure.GPIO_Pin = EXTMODULE_PWR_GPIO_PIN;
  GPIO_Init(EXTMODULE_PWR_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = PWR_SWITCH_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_Init(PWR_SWITCH_GPIO, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = TRAINER_DETECT_GPIO_PIN;
  GPIO_Init(TRAINER_DETECT_GPIO, &GPIO_InitStructure);

  pwrOn();
}

void pwrOn()
{
  GPIO_SetBits(PWR_ON_GPIO, PWR_ON_GPIO_PIN);
}

void pwrOff()
{
  GPIO_ResetBits(PWR_ON_GPIO, PWR_ON_GPIO_PIN);

  // disable interrupts
  __disable_irq();

  while (1) {
    wdt_reset();
  }
  // this function must not return!
}

uint32_t pwrPressed()
{
  return GPIO_ReadInputDataBit(PWR_SWITCH_GPIO, PWR_SWITCH_GPIO_PIN) == Bit_RESET;
}
