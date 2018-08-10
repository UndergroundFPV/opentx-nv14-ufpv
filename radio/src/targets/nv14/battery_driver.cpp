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

#define  __BATTERY_DRIVER_C__


#define GET_ARRAY_SIZE( ARRAY )              ( ( sizeof( ARRAY ) ) / ( sizeof( ARRAY[0] ) ) )


uint16_t  BatteryVoltageMv = 0;


uint16_t battery_voltage_mv( void )
{
  uint32_t t;
  static uint8_t  sampleCount = 0;
  static uint8_t  isFirstSampleOK = 0;
  static uint16_t maxVoltage;
  static uint16_t minVoltage;
  static uint32_t voltageSum;

  t = getAnalogValue(TX_VOLTAGE);
  t += g_eeGeneral.txVoltageCalibration;

  if( 0 == sampleCount )
  {
      sampleCount = 1;
      minVoltage = t;
      maxVoltage = t;
      voltageSum = t;
  }
  else
  {
      voltageSum += t;
      if( t > maxVoltage )
      {
          maxVoltage = t;
      }
      else if( t < minVoltage )
      {
          minVoltage = t;
      }
      sampleCount++;
      if( sampleCount >= 8 )
      {
          t = voltageSum - maxVoltage - minVoltage;
          t /= ( sampleCount - 2 );
          t *= ( 3300 * 20);
          t /= ( 4095 * 10);
          t = ( t * 100  + 50 ) / 100;

          if( isFirstSampleOK )
          {
              BatteryVoltageMv = ( t + BatteryVoltageMv +  1 ) / 2;
          }
          else
          {
              BatteryVoltageMv = t;
              isFirstSampleOK = !0;
          }

          sampleCount = 0;
      }
  }

  return BatteryVoltageMv;
}

uint16_t getBatteryVoltage()
{
#if defined (PCBNV14)
    return battery_voltage_mv() / 100;
#else
    int32_t instant_vbat = anaIn(TX_VOLTAGE);  // using filtered ADC value on purpose
    return (uint16_t)((instant_vbat * (1000 + g_eeGeneral.txVoltageCalibration)) / 1629);
#endif
}



















