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

#define  __HALLSTICK_DRIVER_C__
#include "opentx.h"
#include "hallStick_driver.h"
#undef   __HALLSTICK_DRIVER_C__

// Fifo<uint8_t, 512> hallSerialTxFifo;
DMAFifo<32> hallSerialTxFifo __DMA (HALL_DMA_Stream_TX);
DMAFifo<32> hallSerialRxFifo __DMA (HALL_DMA_Stream_RX);

STRUCT_HALL HallProtocol =
                        {
                          0,
                          GET_START,
                        };
STRUCT_CHANNEL Channel;
STRUCT_STICK_CALIBRATION StickCallbration[4] = {{0, 0, 0}};

void HallStick_Init(void)
{
#if 0
  STRUCT_GPIO_CONFIG Gpio;
  STRUCT_UART_CONFIG Config;

  Gpio.Mode = IO_MODE_ALTERNATE;
  Gpio.Alternate = IO_AF_UART4;
  Gpio.OpenDrainOut = 0;
  Gpio.PUPD = IO_PULL_UP;
  Gpio.Speed = IO_SPEED_HIGH;
  Gpio.Port = PORTA;
  Gpio.Pin = 0;
  GPIO_Config( &Gpio );
  Gpio.Pin = 1;
  GPIO_Config( &Gpio );

  Config.UART = HALLSTICK_UART;
  Config.BaudRate = 921600;
  Config.MSB = 0;
  Config.Tx = 1;
  Config.Rx = 1;
  Config.TxDma = 1;
  Config.RxDma = 1;
  Config.Enable = 1;
  MCU_UART_Config( &Config );
  MCU_UART_ReceiveData( HALLSTICK_UART, ( U32 )HALL_DataBuffer, HALLSTICK_BUFF_SIZE );
  UART_BufferSize[HALLSTICK_UART] = HALLSTICK_BUFF_SIZE;
  UART_DMA_LastValue[HALLSTICK_UART] = HALLSTICK_BUFF_SIZE;
#endif
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = HALL_SERIAL_RX_DMA_Stream_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  GPIO_PinAFConfig(HALL_SERIAL_GPIO, HALL_SERIAL_RX_GPIO_PinSource, HALL_SERIAL_GPIO_AF);
  GPIO_PinAFConfig(HALL_SERIAL_GPIO, HALL_SERIAL_TX_GPIO_PinSource, HALL_SERIAL_GPIO_AF);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = HALL_SERIAL_TX_GPIO_PIN | HALL_SERIAL_RX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(HALL_SERIAL_GPIO, &GPIO_InitStructure);
#if 1
  USART_InitTypeDef USART_InitStructure;
  USART_InitStructure.USART_BaudRate = FLYSKY_HALL_BAUDRATE;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(HALL_SERIAL_USART, &USART_InitStructure);

  DMA_InitTypeDef DMA_InitStructure;
  hallSerialRxFifo.clear();


  DMA_DeInit(HALL_DMA_Stream_RX);
  USART_ITConfig(HALL_SERIAL_USART, USART_IT_RXNE, DISABLE);
  USART_ITConfig(HALL_SERIAL_USART, USART_IT_TXE, DISABLE);
  DMA_InitStructure.DMA_Channel = HALL_DMA_Channel_RX;
  DMA_InitStructure.DMA_PeripheralBaseAddr = CONVERT_PTR_UINT(&HALL_SERIAL_USART->DR);
  DMA_InitStructure.DMA_Memory0BaseAddr = CONVERT_PTR_UINT(hallSerialRxFifo.buffer());
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = hallSerialRxFifo.size();
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(HALL_DMA_Stream_RX, &DMA_InitStructure);
  DMA_Cmd(HALL_DMA_Stream_RX, ENABLE);

#if 0

  /* DMA1 Channel4 (triggered by USART1 Tx event) Config */
  DMA_DeInit(HALL_DMA_Stream_TX);
  DMA_InitStructure.DMA_PeripheralBaseAddr = CONVERT_PTR_UINT(&HALL_SERIAL_USART->DR);
  DMA_InitStructure.DMA_Memory0BaseAddr = CONVERT_PTR_UINT(hallSerialTxFifo.buffer());
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = 0;
  DMA_Init(HALL_DMA_Stream_TX, &DMA_InitStructure);//初始化
  USART_Cmd(HALL_SERIAL_USART, ENABLE);

#endif

  // no DMA ...
  USART_ClearFlag(HALL_SERIAL_USART, USART_FLAG_TC);
  //USART_ITConfig(AUX_SERIAL_USART, USART_IT_RXNE, ENABLE);
  //USART_ITConfig(HALL_SERIAL_USART, USART_IT_TXE, DISABLE);DMA_GetCurrDataCounter

  USART_ITConfig(HALL_SERIAL_USART, USART_IT_IDLE, ENABLE);
  USART_ITConfig(HALL_SERIAL_USART, USART_IT_TC, ENABLE);
  USART_DMACmd(HALL_SERIAL_USART, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

  //NVIC_SetPriority(HALL_SERIAL_RX_DMA_Stream_IRQn, 6);
  //NVIC_EnableIRQ(HALL_SERIAL_RX_DMA_Stream_IRQn);
#endif
  //ResetHall();
}


extern "C" void HALL_SERIAL_USART_IRQHandler(void)
{
  // Send
  if (USART_GetITStatus(HALL_SERIAL_USART, USART_IT_TXE) != RESET) {
    uint8_t txchar;

    if (hallSerialTxFifo.pop(txchar)) {
      /* Write one byte to the transmit data register */
      USART_SendData(HALL_SERIAL_USART, txchar);
    }
    else {
      USART_ITConfig(HALL_SERIAL_USART, USART_IT_TXE, DISABLE);
    }
  }
}

extern "C" void HALL_RX_DMA_Stream_IRQHandler(void)
{
  if (DMA_GetITStatus(HALL_DMA_Stream_RX, USART_IT_IDLE)) {
    // TODO we could send the 8 next channels here (when needed)
    DMA_ClearITPendingBit(HALL_DMA_Stream_RX, USART_IT_IDLE);
  }
}

void hallSerialPutc(char c)
{
#if 0
#if !defined(SIMU)
  int n = 0;
  while (hallSerialTxFifo.isFull()) {
    delay_ms(1);
    //delay(10);
    if (++n > 100) return;
  }
  hallSerialTxFifo.push(c);
  USART_ITConfig(HALL_SERIAL_USART, USART_IT_TXE, ENABLE);
#endif
#endif
}


void HALL_UART_TransmitData( unsigned char *pData, U32 length )
{
  //MCU_UART_TransmitData( HALLSTICK_UART, ( U32 )pData, Length );
#if 0
  int n = 0;
  int i = 0;

  for (i = 0; i < length; i++)
  {
      while (hallSerialTxFifo.isFull())
      {
        delay_ms(1);
        //delay(10);
        if (++n > 100) return;
      }

      hallSerialTxFifo.push(*pData);
      pData++;
  }

  USART_ITConfig(HALL_SERIAL_USART, USART_IT_TXE, ENABLE);
#endif
}



uint16_t HALL_UART_GetReceiveDataLength( void )
{
  //return( UART_GetReceiveDataLength( HALLSTICK_UART ) );
}

void ResetHall( void )
{
    uint8_t Temp[] = {0x55, 0xD1, 0x01, 0x01, 0xff, 0xff};
    unsigned short CRC16 = 0XFFFF;
    CRC16 = Calculation_CRC16(Temp, 4);
    Temp[4] = CRC16 & 0xff;
    Temp[5] = CRC16>>8 & 0xff ;
    HALL_UART_TransmitData( Temp, 6);
}

void Get_factory( void )
{
    uint8_t Temp[] = {0x55, 0xD1, 0x01, 0x00, 0xff, 0xff};
    unsigned short CRC16 = 0XFFFF;
    CRC16 = Calculation_CRC16(Temp, 4);
    Temp[4] = CRC16 & 0xff;
    Temp[5] = CRC16>>8 & 0xff ;
    HALL_UART_TransmitData( Temp, 6);
}



static void Parse_Character(unsigned char Character)
{
    switch( HallProtocol.Status )
    {
        case GET_START:
                    {
                        if( HALL_PROTOLO_HEAD == Character )
                        {
                            HallProtocol.Head  = HALL_PROTOLO_HEAD;
                            HallProtocol.Status = GET_ID;
                        }
                        break;
                    }
        case GET_ID:
                    {
                        HallProtocol.HallID.ID = Character;
                        HallProtocol.Status = GET_LENGTH;
                        break;
                    }
        case GET_LENGTH:
                    {
                        HallProtocol.Length = Character;
                        HallProtocol.DataIndex = 0;
                        HallProtocol.Status = GET_DATA;
                        if( 0 == HallProtocol.Length )
                        {
                            HallProtocol.Status = GET_CHECKSUM;
                            HallProtocol.CheckSum=0;
                        }
                        break;
                    }
        case GET_DATA:
                    {
                        HallProtocol.Data[HallProtocol.DataIndex++] = Character;
                        if( HallProtocol.DataIndex >= HallProtocol.Length)
                        {
                            HallProtocol.CheckSum = 0;
                            HallProtocol.DataIndex = 0;
                            HallProtocol.Status = GET_STATE;
                        }
                        break;
                    }
        case GET_STATE:
                    {
                        //HallProtocol.StickState = HallProtocol.Data[HallProtocol.Length-1];
                        HallProtocol.CheckSum = 0;
                        HallProtocol.DataIndex = 0;
                        HallProtocol.Status = GET_CHECKSUM;
                    }
        case GET_CHECKSUM:
                    {
                        HallProtocol.CheckSum |= Character<<((HallProtocol.DataIndex++)*8);
                        if(HallProtocol.DataIndex >= 2 )
                        {
                            HallProtocol.DataIndex = 0;
                            HallProtocol.Status = CHECKSUM;
                        }
                        else
                        {
                            break;
                        }
                    }
        case CHECKSUM:
                    {
                        if(HallProtocol.CheckSum == Calculation_CRC16( (U8*)&HallProtocol.Head, HallProtocol.Length+3 ) )
                        {
                            HallProtocol.msg_OK = 1;
                            goto Label_restart;
                        }
                        else
                        {
                            goto Label_error;
                        }
                    }
    }
    return;

    Label_error:
    Label_restart:
        HallProtocol.Status = GET_START;
    return ;
}

STRUCT_CHANNEL HallChannel[7];
void ConvertChannel( void )
{
    uint16_t Value;
    for(uint8_t i = 0; i < 4; i++ )
    {
        if( Channel.Channel[i] < StickCallbration[i].Mid)
        {
            Value = StickCallbration[i].Mid-StickCallbration[i].Min;
            Value= ( 500 * (StickCallbration[i].Mid-Channel.Channel[i] ) ) / ( Value );
            if( Value >= 500 )
            {
                Value = 500;
            }
            HallChannel[i].ServoValue =1500 - Value;
        }
        else
        {
            Value = StickCallbration[i].Max-StickCallbration[i].Mid;
            Value = ( 500 * (Channel.Channel[i] - StickCallbration[i].Mid ) ) / ( Value );
            if( Value >= 500 )
            {
                Value = 500;
            }
            HallChannel[i].ServoValue = Value + 1500;
        }
    }
    HallChannel[4].ServoValue = 1000;
    HallChannel[5].ServoValue = 1500;
    HallChannel[6].ServoValue = 1600;

}

void Hall_Task( void )
{
    unsigned char Character;
    uint8_t n = 0;
    n = HALL_UART_GetReceiveDataLength();
    while( n )
    {
        n--;
        Character = HALL_DataBuffer[HallProtocol.Index++];
        if( HallProtocol.Index >= HALLSTICK_BUFF_SIZE )
        {
            HallProtocol.Index = 0;
        }
        Parse_Character( Character );
        if( HallProtocol.msg_OK )
        {
            HallProtocol.msg_OK = 0;
            HallProtocol.StickState = HallProtocol.Data[HallProtocol.Length-1];
            if( 0x01 == HallProtocol.HallID.Hall_Id.ReceiverID )
            {
                if( 0x0e == HallProtocol.HallID.Hall_Id.PacketID )
                {
                    memcpy(&StickCallbration,HallProtocol.Data,sizeof(StickCallbration));
                }
                else if( 0x0c == HallProtocol.HallID.Hall_Id.PacketID )
                {
                    memcpy(&Channel,HallProtocol.Data,sizeof(Channel));

                    ConvertChannel();

                }
            }
        }
    }
    if( (0 == StickCallbration[0].Max) && (0 == StickCallbration[0].Mid) && (0== StickCallbration[0].Min) )
    {
        Get_factory();
    }

}


/* CRC16 implementation according to CCITT standards */

/* CRC16 implementation according to CCITT standards */
const unsigned short CRC16Table[256]= {
 0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
 0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
 0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
 0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
 0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
 0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
 0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
 0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
 0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
 0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
 0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
 0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
 0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
 0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
 0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
 0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
 0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
 0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
 0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
 0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
 0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
 0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
 0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
 0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
 0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
 0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
 0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
 0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
 0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
 0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
 0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
 0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};

unsigned short Calculation_CRC16(unsigned char *pBuffer, unsigned char BufferSize)
{
    unsigned short CRC16;
    CRC16=0xFFFF;
    while (BufferSize)
    {
        CRC16=(CRC16<<8)^CRC16Table[((CRC16>>8)^(*(unsigned char *)pBuffer))&0x00FF];
        pBuffer++;
        BufferSize--;
    }
    return CRC16;
}

