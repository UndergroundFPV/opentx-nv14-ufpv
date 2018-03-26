/***************************************************************************************************
file:
funciton:
auther:
***************************************************************************************************/
/***************************************************************************************************
                             
***************************************************************************************************/ 
#ifndef      __LCD_DRIVER_H__
    #define  __LCD_DRIVER_H__
/***************************************************************************************************
                                 
***************************************************************************************************/ 
    #ifdef      EXTERN
        #undef  EXTERN
    #endif
    
    #ifdef  __LCD_DRIVER_C__
        #define EXTERN
    #else
        #define EXTERN  extern
    #endif
/***************************************************************************************************
                                         
***************************************************************************************************/
#define PORT_BACK_LIGHT                            ( PORTJ )
#define PIN_BACK_LIGHT                             ( 14 )
//#define PORT_BACK_LIGHT                            ( PORTA )
//#define PIN_BACK_LIGHT                             ( 7 )

/***************************************************************************************************
                                         
***************************************************************************************************/


#if 0

#define PORT_LCD_CS                                ( PORTG )
#define PIN_LCD_CS                                 ( 6 )

#define PORT_LCD_CLK                               ( PORTG )
#define PIN_LCD_CLK                                ( 13 )

#define PORT_LCD_MOSI                              ( PORTG )
#define PIN_LCD_MOSI                               ( 14 )

#else

#define PORT_LCD_CS                                ( PORTE )
#define PIN_LCD_CS                                 ( 4 )

#define PORT_LCD_CLK                               ( PORTE )
#define PIN_LCD_CLK                                ( 2 )

#define PORT_LCD_MOSI                              ( PORTE )
#define PIN_LCD_MOSI                               ( 6 )

#define PORT_LCD_MISO                              ( PORTE )
#define PIN_LCD_MISO                               ( 5 )

#endif

#define PORT_LCD_DE                                ( PORTK )
#define PIN_LCD_DE                                 ( 7 )

#define PORT_LCD_RESET                             ( PORTG )
#define PIN_LCD_RESET                              ( 9 )

#define PORT_LCD_HSYNC                             ( PORTI )
#define PIN_LCD_HSYNC                              ( 12 )

#define PORT_LCD_VSYNC                             ( PORTI )
#define PIN_LCD_VSYNC                              ( 13 )

#define PORT_LCD_DOTCLK                            ( PORTG )
#define PIN_LCD_DOTCLK                             ( 7 )

#define PORT_LCD_R0                                ( PORTJ )
#define PORT_LCD_R1                                ( PORTJ )
#define PORT_LCD_R2                                ( PORTJ )
#define PORT_LCD_R3                                ( PORTJ )
#define PORT_LCD_R4                                ( PORTJ )
#define PIN_LCD_R0                                 ( 2 )
#define PIN_LCD_R1                                 ( 3 )
#define PIN_LCD_R2                                 ( 4 )
#define PIN_LCD_R3                                 ( 5 )
#define PIN_LCD_R4                                 ( 6 )

#define PORT_LCD_G0                                ( PORTJ )
#define PORT_LCD_G1                                ( PORTJ )
#define PORT_LCD_G2                                ( PORTJ )
#define PORT_LCD_G3                                ( PORTK )
#define PORT_LCD_G4                                ( PORTK )
#define PORT_LCD_G5                                ( PORTK )
#define PIN_LCD_G0                                 ( 9 )
#define PIN_LCD_G1                                 ( 10 )
#define PIN_LCD_G2                                 ( 11 )
#define PIN_LCD_G3                                 ( 0 )
#define PIN_LCD_G4                                 ( 1 )
#define PIN_LCD_G5                                 ( 2 )

#define PORT_LCD_B0                                ( PORTJ )
#define PORT_LCD_B1                                ( PORTK )
#define PORT_LCD_B2                                ( PORTK )
#define PORT_LCD_B3                                ( PORTK )
#define PORT_LCD_B4                                ( PORTK )                             
#define PIN_LCD_B0                                 ( 15 )
#define PIN_LCD_B1                                 ( 3 )
#define PIN_LCD_B2                                 ( 4 )
#define PIN_LCD_B3                                 ( 5 )
#define PIN_LCD_B4                                 ( 6 )
/***************************************************************************************************
                                         
***************************************************************************************************/
#define SET_LCD_CS()                               { SET_IO_LEVEL( PORT_LCD_CS, PIN_LCD_CS, 1 ); }
#define CLR_LCD_CS()                               { SET_IO_LEVEL( PORT_LCD_CS, PIN_LCD_CS, 0 ); }
#define SET_LCD_CS_OUTPUT()                        { SET_IO_OUTPUT( PORT_LCD_CS, PIN_LCD_CS );   }                       

#define SET_LCD_CLK()                              { SET_IO_LEVEL( PORT_LCD_CLK, PIN_LCD_CLK, 1 ); }
#define CLR_LCD_CLK()                              { SET_IO_LEVEL( PORT_LCD_CLK, PIN_LCD_CLK, 0 ); }
#define SET_LCD_CLK_OUTPUT()                       { SET_IO_OUTPUT( PORT_LCD_CLK, PIN_LCD_CLK );   }                  

#define SET_LCD_DATA()                             { SET_IO_LEVEL( PORT_LCD_MOSI, PIN_LCD_MOSI, 1 ); }
#define CLR_LCD_DATA()                             { SET_IO_LEVEL( PORT_LCD_MOSI, PIN_LCD_MOSI, 0 ); }
#define SET_LCD_DATA_INPUT()                       { SET_IO_INPUT( PORT_LCD_MOSI, PIN_LCD_MOSI ); }    
#define SET_LCD_DATA_OUTPUT()                      { SET_IO_OUTPUT( PORT_LCD_MOSI, PIN_LCD_MOSI ); }    
#define READ_LCD_DATA_PIN()                        ( GPIO_ReadPinLevel( PORT_LCD_MOSI, PIN_LCD_MOSI ) )

#define SET_LCD_RESET()                            { SET_IO_LEVEL( PORT_LCD_RESET, PIN_LCD_RESET, 1 ); }
#define CLR_LCD_RESET()                            { SET_IO_LEVEL( PORT_LCD_RESET, PIN_LCD_RESET, 0 ); }
#define SET_LCD_RESET_OUTPUT()                     { SET_IO_OUTPUT( PORT_LCD_MOSI, PIN_LCD_MOSI ); }
/***************************************************************************************************
                                         
***************************************************************************************************/
#if 1
#define HORIZONTAL_SYNC_WIDTH 			           ( 4 )
#define HORIZONTAL_BACK_PORCH		               ( 24 )
#define HORIZONTAL_FRONT_PORCH                     ( 140 - HORIZONTAL_BACK_PORCH )
#define VERTICAL_SYNC_HEIGHT   		               ( 2 )
#define VERTICAL_BACK_PORCH  		               ( 10 )
#define VERTICAL_FRONT_PORCH    	               ( 22 - VERTICAL_BACK_PORCH )
#else
#define HORIZONTAL_SYNC_WIDTH 			           ( 4 )
#define HORIZONTAL_BACK_PORCH		               ( 0 )
#define HORIZONTAL_FRONT_PORCH                     ( 140 - HORIZONTAL_BACK_PORCH )
#define VERTICAL_SYNC_HEIGHT   		               ( 2 )
#define VERTICAL_BACK_PORCH  		               ( 10 )
#define VERTICAL_FRONT_PORCH    	               ( 13 - VERTICAL_BACK_PORCH )
#endif
/***************************************************************************************************
                                         
***************************************************************************************************/
typedef struct 
{
    unsigned int  ID;
    unsigned int  Width;
    unsigned int  Height;
    void ( *LCD_On )( void );
    void ( *LCD_Off )( void );
    void ( *LCD_Reset )( void );
    void ( *LCD_SetXY )( int x, int y );
}STRUCT_LCD_DRIVER;

typedef struct 
{
    unsigned char Port;
    unsigned char Pin;
    unsigned char Value;
}STRUCT_LCD_GPIO1;
/***************************************************************************************************

***************************************************************************************************/
EXTERN STRUCT_LCD_DRIVER *LCD_DriverInterface;
/***************************************************************************************************

***************************************************************************************************/
#ifdef __LCD_DRIVER_C__

const STRUCT_LCD_DRIVER tLCD_Driver[] = 
{
    { 
        .ID = 0,
        .Width = 320, .Height = 480,
        //.LCD_on = 0,  .LCD_Off = 0,
    }
};

const unsigned char tLCD_GPIO[][2] = 
{
    { PORT_LCD_R0,     PIN_LCD_R0     },
    { PORT_LCD_R1,     PIN_LCD_R1     },
    { PORT_LCD_R2,     PIN_LCD_R2     },
    { PORT_LCD_R3,     PIN_LCD_R3     },
    { PORT_LCD_R4,     PIN_LCD_R4     },
    { PORT_LCD_G0,     PIN_LCD_G0     },
    { PORT_LCD_G1,     PIN_LCD_G1     },
    { PORT_LCD_G2,     PIN_LCD_G2     },
    { PORT_LCD_G3,     PIN_LCD_G3     },
    { PORT_LCD_G4,     PIN_LCD_G4     },
    { PORT_LCD_G5,     PIN_LCD_G5     },
    { PORT_LCD_B0,     PIN_LCD_B0     },
    { PORT_LCD_B1,     PIN_LCD_B1     },
    { PORT_LCD_B2,     PIN_LCD_B2     },
    { PORT_LCD_B3,     PIN_LCD_B3     },
    { PORT_LCD_B4,     PIN_LCD_B4     },
    //{ PORT_LCD_DE,     PIN_LCD_DE     },
    { PORT_LCD_HSYNC,  PIN_LCD_HSYNC  },
    { PORT_LCD_VSYNC,  PIN_LCD_VSYNC  },
    { PORT_LCD_DOTCLK, PIN_LCD_DOTCLK },
};

#endif

/***************************************************************************************************

***************************************************************************************************/
extern void LCD_On( void );
extern void LCD_Off( void );
extern void LCD_Init( void );
/**************************************************************************************************/
#endif /*__BACKLIGHT_H__*/
/***************************************************************************************************
                                       END OF FILE
***************************************************************************************************/

