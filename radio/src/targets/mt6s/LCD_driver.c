/***************************************************************************************************
file:
function:
auther:
***************************************************************************************************/
/***************************************************************************************************
                                          
***************************************************************************************************/
#define  __LCD_DRIVER_C__
#include "include.h"
#undef   __LCD_DRIVER_C__
/***************************************************************************************************
                                             
***************************************************************************************************/
#define LCD_DELAY()                        LCD_Delay()
/*==================================================================================================
Name:
Function:
Input:
Output:
==================================================================================================*/
static void LCD_Delay( void )
{
  volatile unsigned int i;

  for( i = 0; i < 20; i++ )
  {
      ;
  }
}
/*==================================================================================================
Name:
Function:
Input:
Output:
==================================================================================================*/
static void LCD_WriteByte( unsigned char DataEnable, unsigned char SendData )
{
  unsigned int i;

  CLR_LCD_CLK();
  LCD_DELAY();
  if( DataEnable )
  {
      SET_LCD_DATA();
  }
  else
  {
      CLR_LCD_DATA();
  }
  SET_LCD_CLK();
  LCD_DELAY();
  
  for( i = 0; i < 8; i++ )
  {
      CLR_LCD_CLK();
      LCD_DELAY();
      if( SendData & 0x80 )
      {
          SET_LCD_DATA();
      }
      else
      {
          CLR_LCD_DATA();
      }
      
      SET_LCD_CLK();
      SendData <<= 1;
      LCD_DELAY();
  }

  CLR_LCD_CLK();
}
/*==================================================================================================
Name:
Function:
Input:
Output:
==================================================================================================*/
#if 0
static unsigned char LCD_ReceiveByte( void )
{
  unsigned int  i;
  unsigned char ReceiveData = 0;

  SET_LCD_DATA_INPUT();
  for( i = 0; i < 8; i++ )
  {
      SET_LCD_CLK();
      LCD_DELAY();
      CLR_LCD_CLK();
      LCD_DELAY();
      ReceiveData <<= 1;
      if( READ_LCD_DATA_PIN() )
      {
          ReceiveData |= 0x01;
      }
  }

  SET_LCD_DATA_OUTPUT();
  
  return( ReceiveData );
}
#endif
/*==================================================================================================
Name:
Function:
Input:
Output:
==================================================================================================*/
void LCD_WriteCommand( unsigned char Command )
{
  CLR_LCD_CS();
  SET_LCD_CS_OUTPUT();
  SET_LCD_CLK_OUTPUT(); 
  SET_LCD_DATA_OUTPUT();
  LCD_DELAY();
  LCD_DELAY();
  LCD_WriteByte( 0, Command );
  LCD_DELAY();
  LCD_DELAY();
  LCD_DELAY();
  SET_LCD_CS();
}
/*==================================================================================================
Name:
Function:
Input:
Output:
==================================================================================================*/
void LCD_WriteData( unsigned char Parameter )
{
  CLR_LCD_CS();
  SET_LCD_CS_OUTPUT();
  SET_LCD_CLK_OUTPUT();
  SET_LCD_DATA_OUTPUT();
  LCD_DELAY();
  LCD_DELAY();
  LCD_WriteByte( !0, Parameter );
  LCD_DELAY();
  LCD_DELAY();
  LCD_DELAY();
  SET_LCD_CS();
}
/*==================================================================================================
Name:
Function:
Input:
Output:
==================================================================================================*/
void LCD_Init( void )
{
  unsigned int i;

  
  LCD_BufferInit();
  
  SET_LCD_CS();
  SET_LCD_CS_OUTPUT();
  CLR_LCD_CLK();
  SET_LCD_CLK_OUTPUT();
  CLR_LCD_DATA();
  SET_LCD_DATA_OUTPUT();

  CLR_LCD_RESET();
  SET_LCD_RESET_OUTPUT();
  SYS_DelayMs( 100 );
  SET_LCD_RESET();
  SYS_DelayMs( 100 );

  {
      STRUCT_GPIO_CONFIG Config;

      Config.Mode = IO_MODE_ALTERNATE;
      Config.Alternate = IO_AF_LCD;
      Config.OpenDrainOut = 0;
      Config.PUPD = IO_PUSH_PULL;
      Config.Speed = IO_SPEED_HIGH;
      
      for( i = 0; i < GET_ARRAY_SIZE( tLCD_GPIO ); i++ )
      {
          Config.Pin = tLCD_GPIO[i][1];
          Config.Port = tLCD_GPIO[i][0];
          GPIO_Config( &Config );
      }
  }

  LCD_WriteCommand( 0x11 );
  SYS_DelayMs( 120 );
  LCD_WriteCommand( 0x13 );
  
  LCD_WriteCommand( 0xf0 );
  LCD_WriteData( 0xc3 );
  LCD_WriteCommand( 0xf0 );
  LCD_WriteData( 0x96 );
  
  LCD_WriteCommand( 0x36 );	
  
  #if defined( LCD_DIRECTION ) && ( LCD_DIRECTION == LCD_VERTICAL )

  LCD_WriteData( 0x08 );

  #else
  //LCD_WriteData( 0xf8 );
  LCD_WriteData( 0xcc );

  #endif
  
  LCD_WriteCommand( 0x3A );
  LCD_WriteData( 0x65 );
  
  LCD_WriteCommand( 0xB4 );
  LCD_WriteData( 0x01 );

  LCD_WriteCommand( 0xb5 );		
  LCD_WriteData( VERTICAL_FRONT_PORCH );
  LCD_WriteData( VERTICAL_BACK_PORCH + VERTICAL_SYNC_HEIGHT );
  LCD_WriteData( 0x00 );
  LCD_WriteData( HORIZONTAL_BACK_PORCH + 4 );
  
  LCD_WriteCommand( 0xb6 );		
  LCD_WriteData( 0xe0 );
  LCD_WriteData( 0x42 );
  LCD_WriteData( 0x3b );
  
  LCD_WriteCommand( 0xB7 );
  LCD_WriteData( 0x66 );
  
  LCD_WriteCommand( 0xe8 );
  LCD_WriteData( 0x40 );
  LCD_WriteData( 0x8a );
  LCD_WriteData( 0x00 );
  LCD_WriteData( 0x00 );
  LCD_WriteData( 0x29 );
  LCD_WriteData( 0x19 );
  LCD_WriteData( 0xa5 );
  LCD_WriteData( 0x33 );
  
  LCD_WriteCommand( 0xc1 );
  LCD_WriteData( 0x06 );
  
  LCD_WriteCommand( 0xc2 );
  LCD_WriteData( 0xa7 );
  
  LCD_WriteCommand( 0xc5 );
  LCD_WriteData( 0x18 );
  
  LCD_WriteCommand( 0xe0 ); //Positive Voltage Gamma Control
  LCD_WriteData( 0xf0 );
  LCD_WriteData( 0x09 );
  LCD_WriteData( 0x0b );
  LCD_WriteData( 0x06 );
  LCD_WriteData( 0x04 );
  LCD_WriteData( 0x15 );
  LCD_WriteData( 0x2f );
  LCD_WriteData( 0x54 );
  LCD_WriteData( 0x42 );
  LCD_WriteData( 0x3c );
  LCD_WriteData( 0x17 );
  LCD_WriteData( 0x14 );
  LCD_WriteData( 0x18 );
  LCD_WriteData( 0x1b );
  
  LCD_WriteCommand( 0xe1 ); //Negative Voltage Gamma Control
  LCD_WriteData( 0xf0 );
  LCD_WriteData( 0x09 );
  LCD_WriteData( 0x0b );
  LCD_WriteData( 0x06 );
  LCD_WriteData( 0x04 );
  LCD_WriteData( 0x03 );
  LCD_WriteData( 0x2d );
  LCD_WriteData( 0x43 );
  LCD_WriteData( 0x42 );
  LCD_WriteData( 0x3b );
  LCD_WriteData( 0x16 );
  LCD_WriteData( 0x14 );
  LCD_WriteData( 0x17 );
  LCD_WriteData( 0x1b );
  
  LCD_WriteCommand( 0xf0 );
  LCD_WriteData( 0x3c );
  LCD_WriteCommand( 0xf0 );
  LCD_WriteData( 0x69 );
 
  SYS_DelayMs( 5 );
  LCD_WriteCommand( 0x28 );
  //LCD_WriteCommand( 0x29 ); 
  LCD_WriteCommand( 0x2C );  

  {
      STRUCT_LTDC_CONFIG LTDC_Config;

      LTDC_Config.Width = 320;
      LTDC_Config.Height = 480;
      LTDC_Config.PixelFormat = 0x02;
      LTDC_Config.VerticalSynchronizationHeight = VERTICAL_SYNC_HEIGHT;
      LTDC_Config.HorizontalSynchronizationWidth = HORIZONTAL_SYNC_WIDTH;
      LTDC_Config.VerticalBackPorch = VERTICAL_BACK_PORCH;
      LTDC_Config.HorizontalBackPorch = HORIZONTAL_BACK_PORCH;
      LTDC_Config.VerticalFrontHeight = VERTICAL_FRONT_PORCH;
      LTDC_Config.HorizontalFrontWidth = HORIZONTAL_FRONT_PORCH;
      LTDC_Config.LineInterruptEnable = 0;
      LTDC_Config.InterruptPriority = 0;
      LTDC_Config.LineInterruptPosition = 0;
	  LTDC_Config.DotClock = 15000000;
      LTDC_Config.PixelAddress = ( unsigned int )LCD_Buffer[0];
      MCU_LTDC_Config( &LTDC_Config );
  }
}
/*==================================================================================================
Name:
Function:
Input:
Output:
==================================================================================================*/
void LCD_On( void )
{
  LCD_WriteCommand( 0x29 );
}
/*==================================================================================================
Name:
Function:
Input:
Output:
==================================================================================================*/
void LCD_Off( void )
{
  LCD_WriteCommand( 0x28 );
}
/***************************************************************************************************
                                       END OF FILE
***************************************************************************************************/


