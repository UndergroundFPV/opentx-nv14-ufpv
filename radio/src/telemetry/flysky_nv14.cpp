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
    {FLYSKY_SENSOR_GPS,        0, ZSTR_SATELLITES,  UNIT_RAW,           0},
    {FLYSKY_SENSOR_GPS,        1, ZSTR_GPS,         UNIT_GPS_LATITUDE,  0},
    {FLYSKY_SENSOR_GPS,        2, ZSTR_GPS,         UNIT_GPS_LONGITUDE, 0},
    {FLYSKY_SENSOR_GPS,        3, ZSTR_ALT,         UNIT_METERS,        0},
    {FLYSKY_SENSOR_GPS,        4, ZSTR_GSPD,        UNIT_KMH,           1},
    {FLYSKY_SENSOR_GPS,        5, ZSTR_HDG,         UNIT_DEGREE,        3},


};
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
     int16_t Value = 0;
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
                    Value = ((rx_sensor_info.voltage[1]<<8) | rx_sensor_info.voltage[0])*10;
             }break;
        case FLYSKY_SENSOR_RX_SIGNAL:
             {
                    rx_sensor_info.signal = (*ptr++);
                    telemetryNoDMAFifo.push(rx_sensor_info.signal);
                    telemetryNoDMAFifo.push(0x00);
                    Value = rx_sensor_info.signal*10;
             }break;
        case FLYSKY_SENSOR_RX_RSSI:
             {
                    rx_sensor_info.rssi[0] = *ptr++;
                    rx_sensor_info.rssi[1] = *ptr++;
                    telemetryNoDMAFifo.push(rx_sensor_info.rssi[0]);
                    telemetryNoDMAFifo.push(rx_sensor_info.rssi[1]);
                    Value = (rx_sensor_info.rssi[1]<<8) | rx_sensor_info.rssi[0];
                    TempRSSI = rx_sensor_info.rssi[0]+(rx_sensor_info.rssi[1]<<8);
                    if(TempRSSI < -200)
                    {
                        TempRSSI = -200;
                    }
                    TempRSSI = 200 + TempRSSI;
                    TempRSSI /= 2;
                    telemetryData.rssi.set( TempRSSI & 0xff );
             }break;
        case FLYSKY_SENSOR_RX_NOISE:
             {
                    rx_sensor_info.noise[0] = *ptr++;
                    rx_sensor_info.noise[1] = *ptr++;
                    telemetryNoDMAFifo.push(rx_sensor_info.noise[0]);
                    telemetryNoDMAFifo.push(rx_sensor_info.noise[1]);
                    Value = (rx_sensor_info.noise[1]<<8) | rx_sensor_info.noise[0];
             }break;
        case FLYSKY_SENSOR_RX_SNR:
             {
                    rx_sensor_info.snr[0] = *ptr++;
                    rx_sensor_info.snr[1] = *ptr++;
                    telemetryNoDMAFifo.push(rx_sensor_info.snr[0]);
                    telemetryNoDMAFifo.push(rx_sensor_info.snr[1]);
                    Value = (rx_sensor_info.snr[1]<<8) | rx_sensor_info.snr[0];
             }break;
        case FLYSKY_SENSOR_TEMP:
             {
                    rx_sensor_info.temp[0] = *ptr++;
                    rx_sensor_info.temp[1] = *ptr++;
                    telemetryNoDMAFifo.push(rx_sensor_info.temp[0]);
                    telemetryNoDMAFifo.push(rx_sensor_info.temp[1]);
                    Value = (rx_sensor_info.temp[1]<<8) | rx_sensor_info.temp[0];
             }break;
        case FLYSKY_SENSOR_EXT_VOLTAGE:
             {
                    rx_sensor_info.ext_voltage[0] = *ptr++;
                    rx_sensor_info.ext_voltage[1] = *ptr++;
                    telemetryNoDMAFifo.push(rx_sensor_info.ext_voltage[0]);
                    telemetryNoDMAFifo.push(rx_sensor_info.ext_voltage[1]);
                    Value = ((rx_sensor_info.ext_voltage[1]<<8) | rx_sensor_info.ext_voltage[0])/10;
             }break;
        case FLYSKY_SENSOR_MOTO_RPM:
             {
                    rx_sensor_info.moto_rpm[0] = *ptr++;
                    rx_sensor_info.moto_rpm[1] = *ptr++;
                    telemetryNoDMAFifo.push(rx_sensor_info.moto_rpm[0]);
                    telemetryNoDMAFifo.push(rx_sensor_info.moto_rpm[1]);
                    Value = (rx_sensor_info.moto_rpm[1]<<8) | rx_sensor_info.moto_rpm[0];
             }break;
        case FLYSKY_SENSOR_PRESURRE:
             {
                    rx_sensor_info.pressure_value[0] = *ptr++;
                    rx_sensor_info.pressure_value[1] = *ptr++;
                    telemetryNoDMAFifo.push(rx_sensor_info.pressure_value[0]);
                    telemetryNoDMAFifo.push(rx_sensor_info.pressure_value[1]);
                    Value = (rx_sensor_info.pressure_value[1]<<8) | rx_sensor_info.pressure_value[0];
             }break;
#endif
        case FLYSKY_SENSOR_GPS:
             {
                 uint8_t* p_data = &rx_sensor_info.gps_info.position_fix;
                    for (int idx = 17; idx > 0; idx--)
                    {
                        *p_data++ = *ptr++;
                    }
             }break;
        default:
            {
                return;
            }break;
    }
    telemetryStreaming = TELEMETRY_TIMEOUT10ms;
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
}

