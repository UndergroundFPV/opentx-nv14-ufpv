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

/***************************************************************************************************

***************************************************************************************************/
#ifndef      __BATTERY_DRIVER_H__
    #define  __BATTERY_DRIVER_H__
/***************************************************************************************************

***************************************************************************************************/
    #ifdef      EXTERN
        #undef  EXTERN
    #endif

    #ifdef  __BATTERY_DRIVER_C__
        #define EXTERN
    #else
        #define EXTERN  extern
    #endif

#include "board.h"

#define GET_ARRAY_SIZE( ARRAY )       ( ( sizeof( ARRAY ) ) / ( sizeof( ARRAY[0] ) ) )


#define PWR_CHARGE_FINISHED_GPIO                 GPIOB
#define PWR_CHARGE_FINISHED_GPIO_REG             PWR_CHARGE_FINISHED_GPIO->IDR
#define PWR_CHARGE_FINISHED_GPIO_PIN             GPIO_Pin_13 // PB.13

#define PWR_IN_CHARGING_GPIO                     GPIOB
#define PWR_IN_CHARGING_GPIO_REG                 PWR_IN_CHARGING_GPIO->IDR
#define PWR_IN_CHARGING_GPIO_PIN                 GPIO_Pin_14 // PB.14

extern uint16_t getBatteryVoltage();   // returns current battery voltage in 10mV steps

#endif


















