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

struct FlyskyNv14Sensor {
  const uint8_t id;
  const uint8_t instance;
  const char * name;
  const TelemetryUnit unit;
  const uint8_t precision;
};
struct FlyskyNv14SensorSubNumber
{
    uint8_t id;
    uint8_t number;
};
const FlyskyNv14SensorSubNumber Nv14SensorNumber[]=
{
    {FLYSKY_SENSOR_RX_VOLTAGE, 1},
    {FLYSKY_SENSOR_RX_SIGNAL,  1},
    {FLYSKY_SENSOR_RX_RSSI,    1},
    {FLYSKY_SENSOR_RX_NOISE,   1},
    {FLYSKY_SENSOR_RX_SNR,     1},
    {FLYSKY_SENSOR_TEMP,       1},
    {FLYSKY_SENSOR_EXT_VOLTAGE,1},
    {FLYSKY_SENSOR_MOTO_RPM,   1},
    {FLYSKY_SENSOR_PRESURRE,   2},//
    {FLYSKY_SENSOR_GPS,        6},

};
const FlyskyNv14Sensor Nv14Sensor[]=
{
    {0,                        0, "UNKNOWN",        UNIT_RAW,           0},
    {FLYSKY_SENSOR_RX_VOLTAGE, 0, ZSTR_BATT,        UNIT_VOLTS,         3},
    {FLYSKY_SENSOR_RX_SIGNAL,  0, ZSTR_SIG,         UNIT_RAW,           1},
    {FLYSKY_SENSOR_RX_RSSI,    0, ZSTR_RSSI,        UNIT_DB,            0},
    {FLYSKY_SENSOR_RX_NOISE,   0, ZSTR_NOISE,       UNIT_DB,            0},
    {FLYSKY_SENSOR_RX_SNR,     0, ZSTR_RX_SNR,      UNIT_DB,            0},
    {FLYSKY_SENSOR_TEMP,       0, ZSTR_TEMP1,       UNIT_CELSIUS,       1},
    {FLYSKY_SENSOR_EXT_VOLTAGE,0, ZSTR_EBATT,       UNIT_VOLTS,         1},
    {FLYSKY_SENSOR_MOTO_RPM,   0, ZSTR_RPM,         UNIT_RPMS,          0},
    {FLYSKY_SENSOR_PRESURRE,   0, ZSTR_AIRPRE,      UNIT_RAW,           1},//
    {FLYSKY_SENSOR_PRESURRE,   1, ZSTR_ALT,         UNIT_METERS,        0},//
    {FLYSKY_SENSOR_GPS,        1, ZSTR_SATELLITES,  UNIT_RAW,           0},
    {FLYSKY_SENSOR_GPS,        2, ZSTR_GPS,         UNIT_GPS_LATITUDE,  0},
    {FLYSKY_SENSOR_GPS,        3, ZSTR_GPS,         UNIT_GPS_LONGITUDE, 0},
    {FLYSKY_SENSOR_GPS,        4, ZSTR_ALT,         UNIT_METERS,        0},
    {FLYSKY_SENSOR_GPS,        5, ZSTR_GSPD,        UNIT_KMH,           1},
    {FLYSKY_SENSOR_GPS,        6, ZSTR_HDG,         UNIT_DEGREE,        3},
};
const S16 tAltitude[225]=
{
    20558, 20357, 20158, 19962, 19768, 19576, 19387, 19200, 19015,  18831, 18650, 18471, 18294, 18119, 17946, 17774,
    17604, 17436, 17269, 17105, 16941, 16780, 16619, 16461, 16304,  16148, 15993, 15841, 15689, 15539, 15390, 15242,
    15096, 14950, 14806, 14664, 14522, 14381, 14242, 14104, 13966,  13830, 13695, 13561, 13428, 13296, 13165, 13035,
    12906, 12777, 12650, 12524, 12398, 12273, 12150, 12027, 11904,  11783, 11663, 11543, 11424, 11306, 11189, 11072,
    10956, 10841, 10726, 10613, 10500, 10387, 10276, 10165, 10054,   9945,  9836,  9727,  9620,  9512,  9406,  9300,
    9195,  9090,  8986,  8882,  8779,  8677,   8575,  8474,  8373,   8273,  8173,  8074,  7975,  7877,  7779,  7682,
    7585,  7489,  7394,  7298,  7204,  7109,   7015,  6922,  6829,   6737,  6645,  6553,  6462,  6371,  6281,  6191,
    6102,  6012,  5924,  5836,  5748,  5660,   5573,  5487,  5400,   5314,  5229,  5144,  5059,  4974,  4890,  4807,
    4723,  4640,  4557,  4475,  4393,  4312,   4230,  4149,  4069,   3988,  3908,  3829,  3749,  3670,  3591,  3513,
    3435,  3357,  3280,  3202,  3125,  3049,   2972,  2896,  2821,   2745,  2670,  2595,  2520,  2446,  2372,  2298,
    2224,  2151,  2078,  2005,  1933,   1861,  1789,  1717,  1645,   1574,  1503,  1432,  1362,  1292,  1222,  1152,
    1082,  1013,   944,   875,   806,   738,    670,   602,   534,    467,   399,   332,   265,   199,   132,    66,
     0,     -66,  -131,  -197,  -262,  -327,   -392,  -456,  -521,   -585,  -649,  -713,  -776,  -840,  -903,  -966,
    -1029,-1091, -1154, -1216, -1278, -1340,  -1402, -1463,  -1525, -1586, -1647, -1708, -1769, -1829, -1889, -1950,
    -2010
};
/*==================================================================================================
Name:
Function:
Input:
Output:
==================================================================================================*/
S16 CalculateAltitude( U32 Pressure, U32 SeaLevelPressure )
{
  U32 Index;
  S32 Altitude1;
  S32 Altitude2;
  U32 Decimal;
  U64 Ratio;

  Ratio = ( ( ( unsigned long long ) Pressure << 16 ) + ( SeaLevelPressure / 2 ) ) / SeaLevelPressure;
  if( Ratio < ( ( 1 << 16 ) * 250 / 1000 ) )// 0.250 inclusive
  {
      Ratio = ( 1 << 16 ) * 250 / 1000;
  }
  else if( Ratio > ( 1 << 16 ) * 1125 / 1000 - 1 ) // 1.125 non-inclusive
  {
      Ratio = ( 1 << 16 ) * 1125 / 1000 - 1;
  }

  Ratio -= ( 1 << 16 ) * 250 / 1000; // from 0.000 (inclusive) to 0.875 (non-inclusive)
  Index = Ratio >> 8;
  Decimal = Ratio & ( ( 1 << 8 ) - 1 );
  Altitude1 = tAltitude[Index];
  Altitude2 = Altitude1 - tAltitude[Index + 1];
  Altitude1 = Altitude1 - ( Altitude2 * Decimal + ( 1 << 7 ) ) / ( 1 << 8 );
  if( Altitude1 >= 0 )
  {
      return( ( Altitude1 + 1 ) / 2 );
  }
  else
  {
      return( ( Altitude1 - 1 ) / 2 );
  }
}
const FlyskyNv14Sensor * getFlyskyNv14Sensor(uint16_t id, uint8_t subId )
{
    const FlyskyNv14Sensor * pSensor = NULL;
    if(id+subId < sizeof (Nv14Sensor))
    {
        pSensor = &Nv14Sensor[id+subId];
    }
    return pSensor;
}
void flySkyNv14SetDefault(int index, uint8_t id, uint8_t subId,uint8_t instance)
{
    TelemetrySensor & telemetrySensor = g_model.telemetrySensors[index];
    telemetrySensor.id = id;
    telemetrySensor.subId = subId;
    telemetrySensor.instance = instance;
    const FlyskyNv14Sensor * sensor = getFlyskyNv14Sensor(id, subId);
    TelemetryUnit unit = sensor->unit;
    if (unit == UNIT_GPS_LATITUDE || unit == UNIT_GPS_LONGITUDE)
    {
        unit = UNIT_GPS;
    }
    uint8_t prec = min<uint8_t>(2, sensor->precision);
    telemetrySensor.init(sensor->name, unit, prec);

    storageDirty(EE_MODEL);
}

void flySkyNv14ProcessTelemetryPacket(uint16_t id, uint8_t subId, uint8_t instance, int32_t data, TelemetryUnit unit=UNIT_RAW)
{
 const FlyskyNv14Sensor * sensor = getFlyskyNv14Sensor(id, subId);
  uint8_t precision = 0;
  if (sensor)
  {
    if (unit == UNIT_RAW)
     {
        unit = sensor->unit;
     }
    precision = sensor->precision;
  }
    setTelemetryValue(TELEM_PROTO_FLYSKY_NV14, id, subId, instance, data, unit, precision);
}
extern rx_sensor_t rx_sensor_info;
static uint32_t GetGPSsensorValue(uint8_t index)
{
    uint32_t Temp = 0;
    uint8_t* p_data = &rx_sensor_info.gps_info.position_fix;
    switch(index)
    {
        case 1: Temp = *p_data; break;
        case 2: Temp = p_data[1]+(p_data[2]<<8)+(p_data[3]<<16)+(p_data[4]<<24); break;
        case 3: Temp = p_data[5]+(p_data[6]<<8)+(p_data[73]<<16)+(p_data[8]<<24); break;
        case 4: Temp = p_data[8]+(p_data[9]<<8); break;
        case 5: Temp = p_data[10]+(p_data[11]<<8); break;
        case 6: Temp = p_data[12]+(p_data[13]<<8); break;
        default: break;
    }
    return Temp;
}
void flySkyNv14ProcessTelemetryPacket(const uint8_t * ptr, uint8_t SensorType )
{
    extern Fifo<uint8_t, TELEMETRY_FIFO_SIZE> telemetryNoDMAFifo;
     uint8_t Sensor_id = *ptr++;
     int16_t Value[20] = {0};
    static int16_t TempRSSI = 0;
    telemetryNoDMAFifo.push(0xAA);
    telemetryNoDMAFifo.push(SensorType);// sensor id refer to FlySkySensorType_E
    switch(SensorType)
    {
#if 1
        case FLYSKY_SENSOR_RX_VOLTAGE:
             {
                    rx_sensor_info.voltage[0] = *ptr++;
                    rx_sensor_info.voltage[1] = *ptr++;
                    telemetryNoDMAFifo.push(rx_sensor_info.voltage[0]);
                    telemetryNoDMAFifo.push(rx_sensor_info.voltage[1]);
                    Value[0] = ((rx_sensor_info.voltage[1]<<8) | rx_sensor_info.voltage[0])*10;
             }break;
        case FLYSKY_SENSOR_RX_SIGNAL:
             {
                    rx_sensor_info.signal = (*ptr++);
                    telemetryNoDMAFifo.push(rx_sensor_info.signal);
                    telemetryNoDMAFifo.push(0x00);
                    Value[0] = rx_sensor_info.signal*10;
             }break;
        case FLYSKY_SENSOR_RX_RSSI:
             {
                    rx_sensor_info.rssi[0] = *ptr++;
                    rx_sensor_info.rssi[1] = *ptr++;
                    telemetryNoDMAFifo.push(rx_sensor_info.rssi[0]);
                    telemetryNoDMAFifo.push(rx_sensor_info.rssi[1]);
                    Value[0] = (rx_sensor_info.rssi[1]<<8) | rx_sensor_info.rssi[0];
                    TempRSSI = rx_sensor_info.rssi[0]+(rx_sensor_info.rssi[1]<<8);
                    if(TempRSSI < -200)
                    {
                        TempRSSI = -200;
                    }
                    TempRSSI = 200 + TempRSSI;
                    TempRSSI /= 2;
                    Value[0] = TempRSSI;
                    telemetryData.rssi.set( TempRSSI & 0xff );
             }break;
        case FLYSKY_SENSOR_RX_NOISE:
             {
                    rx_sensor_info.noise[0] = *ptr++;
                    rx_sensor_info.noise[1] = *ptr++;
                    telemetryNoDMAFifo.push(rx_sensor_info.noise[0]);
                    telemetryNoDMAFifo.push(rx_sensor_info.noise[1]);
                    Value[0] = (rx_sensor_info.noise[1]<<8) | rx_sensor_info.noise[0];
             }break;
        case FLYSKY_SENSOR_RX_SNR:
             {
                    rx_sensor_info.snr[0] = *ptr++;
                    rx_sensor_info.snr[1] = *ptr++;
                    telemetryNoDMAFifo.push(rx_sensor_info.snr[0]);
                    telemetryNoDMAFifo.push(rx_sensor_info.snr[1]);
                    Value[0] = (rx_sensor_info.snr[1]<<8) | rx_sensor_info.snr[0];
             }break;
        case FLYSKY_SENSOR_TEMP:
             {
                    rx_sensor_info.temp[0] = *ptr++;
                    rx_sensor_info.temp[1] = *ptr++;
                    telemetryNoDMAFifo.push(rx_sensor_info.temp[0]);
                    telemetryNoDMAFifo.push(rx_sensor_info.temp[1]);
                    Value[0] = (rx_sensor_info.temp[1]<<8) | rx_sensor_info.temp[0];
             }break;
        case FLYSKY_SENSOR_EXT_VOLTAGE:
             {
                    rx_sensor_info.ext_voltage[0] = *ptr++;
                    rx_sensor_info.ext_voltage[1] = *ptr++;
                    telemetryNoDMAFifo.push(rx_sensor_info.ext_voltage[0]);
                    telemetryNoDMAFifo.push(rx_sensor_info.ext_voltage[1]);
                    Value[0] = ((rx_sensor_info.ext_voltage[1]<<8) | rx_sensor_info.ext_voltage[0]);
                    Value[0] /= 10;
             }break;
        case FLYSKY_SENSOR_MOTO_RPM:
             {
                    rx_sensor_info.moto_rpm[0] = *ptr++;
                    rx_sensor_info.moto_rpm[1] = *ptr++;
                    telemetryNoDMAFifo.push(rx_sensor_info.moto_rpm[0]);
                    telemetryNoDMAFifo.push(rx_sensor_info.moto_rpm[1]);
                    Value[0] = (rx_sensor_info.moto_rpm[1]<<8) | rx_sensor_info.moto_rpm[0];
             }break;
        case FLYSKY_SENSOR_PRESURRE:
             {
                    rx_sensor_info.pressure_value[0] = *ptr++;
                    rx_sensor_info.pressure_value[1] = *ptr++;
                    telemetryNoDMAFifo.push(rx_sensor_info.pressure_value[0]);
                    telemetryNoDMAFifo.push(rx_sensor_info.pressure_value[1]);
                    Value[0] = (rx_sensor_info.pressure_value[1]<<8) | rx_sensor_info.pressure_value[0];
                    Value[1] = CalculateAltitude(Value[0], 101325);
                    Value[1] = (Value[1]+500)/1000;
             }break;
#endif
        case FLYSKY_SENSOR_GPS:
             {
                 uint8_t* p_data = &rx_sensor_info.gps_info.position_fix;
                 uint8_t* p_data1 = (uint8_t*)Value;
                    for (int idx = 17; idx > 0; idx--)
                    {
                        *p_data++ = *ptr++;
                        *p_data1++ = *ptr++;
                    }
             }break;
        default:
            {
                return;
            }break;
    }
    telemetryStreaming = TELEMETRY_TIMEOUT10ms;
#if 0
    if(SensorType != FLYSKY_SENSOR_GPS)
    {
        flySkyNv14ProcessTelemetryPacket(SensorType+1, 0, Sensor_id, Value);
    }
    else if(0 != rx_sensor_info.gps_info.position_fix)
    {
        uint32_t Temp = 0;
        for(uint8_t i = 0; i < 4; i++)
        {
            Temp = GetGPSsensorValue(i+1);
            flySkyNv14ProcessTelemetryPacket(SensorType+1, i, Sensor_id, Temp);
        }
    }
#else
    for(uint8_t i = 0; i < Nv14SensorNumber[SensorType].number; i++)
    {
        flySkyNv14ProcessTelemetryPacket(SensorType+1, i, Sensor_id, Value[i]);
    }
#endif
}

