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

#include "board.h"

#define SDRAM_BANK1             ( 0x02 )
#define SDRAM_BANK2             ( 0x01 )
#define SDRAM_BANK              ( SDRAM_BANK1 )

#define SDRAM_BURST_LENGTH_1                       ( 0 )
#define SDRAM_BURST_LENGTH_2                       ( 1 )
#define SDRAM_BURST_LENGTH_4                       ( 2 )
#define SDRAM_BURST_LENGTH_8                       ( 3 )
#define SDRAM_BURST_FULL_PAGE                      ( 7 )

#define BURST_READ_BURST_WRITE                     ( 0 )
#define BURST_READ_SINGLE_WRITE                    ( 1 )

#define SDRAM_SIZE                                 ( 4UL * 1024UL * 1024UL *  2UL )


/**
  * @brief  Configures all SDRAM memory I/Os pins.
  * @param  None.
  * @retval None.
  */
void SDRAM_GPIOConfig(void)
{
  /*
  ------------------------------------------------------------------------------------------------------------------------------------------------
   PC3  <-> FMC_SDCKE0 | PD0  <-> FMC_D2   | PE0  <-> FMC_NBL0  | PF0  <-> FMC_A0    | PG0  <-> FMC_A10   | PH3  <-> FMC_SDNE0 | PI0  <-> FMC_D24
                       | PD1  <-> FMC_D3   | PE1  <-> FMC_NBL1  | PF1  <-> FMC_A1    | PG1  <-> FMC_A11   | PH5  <-> FMC_SDNWE | PI1  <-> FMC_D25
                       | PD8  <-> FMC_D13  | PE7  <-> FMC_D4    | PF2  <-> FMC_A2    | PG4  <-> FMC_BA0   | PH8  <-> FMC_D16   | PI2  <-> FMC_D26
                       | PD9  <-> FMC_D14  | PE8  <-> FMC_D5    | PF3  <-> FMC_A3    | PG5  <-> FMC_BA1   | PH9  <-> FMC_D17   | PI3  <-> FMC_D27
                       | PD10 <-> FMC_D15  | PE9  <-> FMC_D6    | PF4  <-> FMC_A4    | PG8  <-> FMC_SDCLK | PH10 <-> FMC_D18   | PI4  <-> FMC_NBL2
                       | PD14 <-> FMC_D0   | PE10 <-> FMC_D7    | PF5  <-> FMC_A5    | PG15 <-> FMC_NCAS  | PH11 <-> FMC_D19   | PI5  <-> FMC_NBL3
                       | PD15 <-> FMC_D1   | PE11 <-> FMC_D8    | PF11 <-> FMC_NRAS  |                    | PH12 <-> FMC_D20   | PI6  <-> FMC_D28
                       |                   | PE12 <-> FMC_D9    | PF12 <-> FMC_A6    |                    | PH13 <-> FMC_D21   | PI7  <-> FMC_D29
                       |                   | PE13 <-> FMC_D10   | PF13 <-> FMC_A7    |                    | PH14 <-> FMC_D22   | PI9  <-> FMC_D30
                       |                   | PE14 <-> FMC_D11   | PF14 <-> FMC_A8    |                    | PH15 <-> FMC_D23   | PI10 <-> FMC_D31
                       |                   | PE15 <-> FMC_D12   | PF15 <-> FMC_A9    |                    |                    |
  */

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  /* GPIOC configuration */
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource3 , GPIO_AF_FMC);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* GPIOD configuration */
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FMC);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* GPIOE configuration */
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource0, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource1, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FMC);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* GPIOF configuration */
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource0, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource1, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource2, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource3, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource4, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource5, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource11, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource12, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource13, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource14, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource15, GPIO_AF_FMC);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_Init(GPIOF, &GPIO_InitStructure);

  /* GPIOG configuration */
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource0 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource1 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource4 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource5 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource8 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource15 , GPIO_AF_FMC);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_15;
  GPIO_Init(GPIOG, &GPIO_InitStructure);

  /* GPIOH configuration */
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource3, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource5, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource8, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource9, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource10, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource11, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource12, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource13, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource14, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource15, GPIO_AF_FMC);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_Init(GPIOH, &GPIO_InitStructure);

  /* GPIOI configuration */
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource0, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource1, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource2, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource3, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource4, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource5, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource6, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource7, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource9, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource10, GPIO_AF_FMC);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_10;
  GPIO_Init(GPIOI, &GPIO_InitStructure);
}

void SDRAM_Init(void)
{
  SDRAM_GPIOConfig();

  {
       unsigned char * pData;
       STRUCT_SDRAM_CONFIG Config;

       pData = ( unsigned char * )&Config;
       for(unsigned int i = 0; i <= sizeof( Config ); i++ )
       {
           *pData++ = 0;
       }

       Config.Peripheral = MCU_FMC;
       Config.RowWidth = SDRAM_ROW_12BITS;
       Config.ColumnWidth = SDRAM_COLUMN_8BITS;
       Config.DataBusWidth = SDRAM_DATA_BUS_16BITS;
       Config.Bank = SDRAM_BANK;
       Config.FourInternalBanks = 1;
       Config.CASLatency = 0x02;
       Config.ModeData = ( BURST_READ_SINGLE_WRITE << 9 )|( Config.CASLatency << 4 )|SDRAM_BURST_FULL_PAGE;
       Config.WriteProtection = 0;
       Config.SDRAMClock = 2;
       Config.BurstRead = 1;
       Config.ReadPipeDelay = 1;
       Config.RowToColumnDelay = 1;
       Config.RowPrechargeDelay = 1;
       Config.RecoveryDelay = 2;
       Config.RowCycleDelay = 5;
       Config.SelfRefreshTime = 3;
       Config.ExitSelfRefreshDelay = 6;
       Config.LoadModeDataToActiveTime = 1;
       Config.AutoRefreshCycles = 8;
       Config.RefreshTimeMs = 48;
       MCU_FMC_SDRAM_Config( &Config );
  }
}
