/***************************************************************************************************
file:
function:
auther:
***************************************************************************************************/
/***************************************************************************************************
                                          
***************************************************************************************************/
#define  __SDRAM_DRIVER_C__
#include "include.h"
#undef   __SDRAM_DRIVER_C__
/***************************************************************************************************
                                             
***************************************************************************************************/
#define SDRAM_BANK1             ( 0x02 )
#define SDRAM_BANK2             ( 0x01 )
#define SDRAM_BANK              ( SDRAM_BANK1 )
/***************************************************************************************************
                                            code                  
***************************************************************************************************/
/*==================================================================================================
Table:
Function:
==================================================================================================*/
static const unsigned char tSDRAM_GPIO[][2] = 
{
    { PORT_SDRAM_CS,    PIN_SDRAM_CS   },
    { PORT_SDRAM_WE,    PIN_SDRAM_WE   },
    { PORT_SDRAM_BS0,   PIN_SDRAM_BS0  },
    { PORT_SDRAM_BS1,   PIN_SDRAM_BS1  },
    { PORT_SDRAM_CKE,   PIN_SDRAM_CKE  },
    { PORT_SDRAM_CLK,   PIN_SDRAM_CLK  },
    { PORT_SDRAM_RAS,   PIN_SDRAM_RAS  },
    { PORT_SDRAM_CAS,   PIN_SDRAM_CAS  },
    { PORT_SDRAM_DQM0,  PIN_SDRAM_DQM0 },
    { PORT_SDRAM_DQM1,  PIN_SDRAM_DQM1 },

    { PORT_SDRAM_A0,    PIN_SDRAM_A0   },
    { PORT_SDRAM_A1,    PIN_SDRAM_A1   },
    { PORT_SDRAM_A2,    PIN_SDRAM_A2   },
    { PORT_SDRAM_A3,    PIN_SDRAM_A3   },
    { PORT_SDRAM_A4,    PIN_SDRAM_A4   },
    { PORT_SDRAM_A5,    PIN_SDRAM_A5   },
    { PORT_SDRAM_A6,    PIN_SDRAM_A6   },
    { PORT_SDRAM_A7,    PIN_SDRAM_A7   },
    { PORT_SDRAM_A8,    PIN_SDRAM_A8   },
    { PORT_SDRAM_A9,    PIN_SDRAM_A9   },
    { PORT_SDRAM_A10,   PIN_SDRAM_A10  },
    { PORT_SDRAM_A11,   PIN_SDRAM_A11  },

    { PORT_SDRAM_D0,    PIN_SDRAM_D0   },
    { PORT_SDRAM_D1,    PIN_SDRAM_D1   },
    { PORT_SDRAM_D2,    PIN_SDRAM_D2   },
    { PORT_SDRAM_D3,    PIN_SDRAM_D3   },
    { PORT_SDRAM_D4,    PIN_SDRAM_D4   },
    { PORT_SDRAM_D5,    PIN_SDRAM_D5   },
    { PORT_SDRAM_D6,    PIN_SDRAM_D6   },
    { PORT_SDRAM_D7,    PIN_SDRAM_D7   },
    { PORT_SDRAM_D8,    PIN_SDRAM_D8   },
    { PORT_SDRAM_D9,    PIN_SDRAM_D9   },
    { PORT_SDRAM_D10,   PIN_SDRAM_D10  },
    { PORT_SDRAM_D11,   PIN_SDRAM_D11  },
    { PORT_SDRAM_D12,   PIN_SDRAM_D12  },
    { PORT_SDRAM_D13,   PIN_SDRAM_D13  },
    { PORT_SDRAM_D14,   PIN_SDRAM_D14  },
    { PORT_SDRAM_D15,   PIN_SDRAM_D15  },
    
    { PORT_SDRAM_D16,   PIN_SDRAM_D16  },
    { PORT_SDRAM_D17,   PIN_SDRAM_D17  },
    { PORT_SDRAM_D18,   PIN_SDRAM_D18  },
    { PORT_SDRAM_D19,   PIN_SDRAM_D19  },
    { PORT_SDRAM_D20,   PIN_SDRAM_D20  },
    { PORT_SDRAM_D21,   PIN_SDRAM_D21  },
    { PORT_SDRAM_D22,   PIN_SDRAM_D22  },
    { PORT_SDRAM_D23,   PIN_SDRAM_D23  },
    { PORT_SDRAM_D24,   PIN_SDRAM_D24  },
    { PORT_SDRAM_D25,   PIN_SDRAM_D25  },
    { PORT_SDRAM_D26,   PIN_SDRAM_D26  },
    { PORT_SDRAM_D27,   PIN_SDRAM_D27  },
    { PORT_SDRAM_D28,   PIN_SDRAM_D28  },
    { PORT_SDRAM_D29,   PIN_SDRAM_D29  },
    { PORT_SDRAM_D30,   PIN_SDRAM_D30  },
    { PORT_SDRAM_D31,   PIN_SDRAM_D31  },
};
/*==================================================================================================
Name:
Function:
Input:
Output:
==================================================================================================*/
void SDRAM_Init( void )
{
  unsigned int i;
  
  {
      STRUCT_GPIO_CONFIG Gpio;
      
      Gpio.Mode = IO_MODE_ALTERNATE;
      Gpio.Alternate = IO_AF_FMC;
      Gpio.OpenDrainOut = 0;
      Gpio.Speed = IO_SPEED_HIGH;
      Gpio.PUPD = IO_PUSH_PULL;
      for( i = 0; i < GET_ARRAY_SIZE( tSDRAM_GPIO ); i++ )
      {
          Gpio.Pin = tSDRAM_GPIO[i][1];
          Gpio.Port = tSDRAM_GPIO[i][0];
          GPIO_Config( &Gpio );
      }
  }

  {
       unsigned char *pData;
       STRUCT_SDRAM_CONFIG Config;

       pData = ( unsigned char * )&Config;
       for( i = 0; i <= sizeof( Config ); i++ )
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
/***************************************************************************************************
                                       END OF FILE
***************************************************************************************************/



