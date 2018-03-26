/***************************************************************************************************
file:SDRAM_driver.h
funciton:
auther:
***************************************************************************************************/
/***************************************************************************************************
                             
***************************************************************************************************/ 
#ifndef      __SDRAM_DRIVER_H__
    #define  __SDRAM_DRIVER_H__
/***************************************************************************************************
                                 
***************************************************************************************************/ 
    #ifdef      EXTERN
        #undef  EXTERN
    #endif
    
    #ifdef  __SDRAM_DRIVER_C__
        #define EXTERN
    #else
        #define EXTERN  extern
    #endif
/***************************************************************************************************
                                          SDRAM端口定义
***************************************************************************************************/
#define PORT_SDRAM_CS                              ( PORTH )
#define PIN_SDRAM_CS                               ( 3 )
#define PORT_SDRAM_WE                              ( PORTH )
#define PIN_SDRAM_WE                               ( 5 )
#define PORT_SDRAM_BS0                             ( PORTG )
#define PIN_SDRAM_BS0                              ( 4 )
#define PORT_SDRAM_BS1                             ( PORTG )
#define PIN_SDRAM_BS1                              ( 5 )
#define PORT_SDRAM_CKE                             ( PORTC )
#define PIN_SDRAM_CKE                              ( 3 )
#define PORT_SDRAM_CLK                             ( PORTG )
#define PIN_SDRAM_CLK                              ( 8 )
#define PORT_SDRAM_RAS                             ( PORTF )
#define PIN_SDRAM_RAS                              ( 11 )
#define PORT_SDRAM_CAS                             ( PORTG )
#define PIN_SDRAM_CAS                              ( 15 )
#define PORT_SDRAM_DQM0                            ( PORTE )
#define PIN_SDRAM_DQM0                             ( 0 )
#define PORT_SDRAM_DQM1                            ( PORTE )
#define PIN_SDRAM_DQM1                             ( 1 )
#define PORT_SDRAM_DQM2                            ( PORTI )
#define PIN_SDRAM_DQM2                             ( 4 )
#define PORT_SDRAM_DQM3                            ( PORTI )
#define PIN_SDRAM_DQM3                             ( 5 )

#define PORT_SDRAM_A0                              ( PORTF )
#define PIN_SDRAM_A0                               ( 0 )
#define PORT_SDRAM_A1                              ( PORTF )
#define PIN_SDRAM_A1                               ( 1 )
#define PORT_SDRAM_A2                              ( PORTF )
#define PIN_SDRAM_A2                               ( 2 )
#define PORT_SDRAM_A3                              ( PORTF )
#define PIN_SDRAM_A3                               ( 3 )
#define PORT_SDRAM_A4                              ( PORTF )
#define PIN_SDRAM_A4                               ( 4 )
#define PORT_SDRAM_A5                              ( PORTF )
#define PIN_SDRAM_A5                               ( 5 )
#define PORT_SDRAM_A6                              ( PORTF )
#define PIN_SDRAM_A6                               ( 12 )
#define PORT_SDRAM_A7                              ( PORTF )
#define PIN_SDRAM_A7                               ( 13 )
#define PORT_SDRAM_A8                              ( PORTF )
#define PIN_SDRAM_A8                               ( 14 )
#define PORT_SDRAM_A9                              ( PORTF )
#define PIN_SDRAM_A9                               ( 15 )
#define PORT_SDRAM_A10                             ( PORTG )
#define PIN_SDRAM_A10                              ( 0 )
#define PORT_SDRAM_A11                             ( PORTG )
#define PIN_SDRAM_A11                              ( 1 )

#define PORT_SDRAM_D0                              ( PORTD )
#define PORT_SDRAM_D1                              ( PORTD )
#define PORT_SDRAM_D2                              ( PORTD )
#define PORT_SDRAM_D3                              ( PORTD )
#define PORT_SDRAM_D4                              ( PORTE )
#define PORT_SDRAM_D5                              ( PORTE )
#define PORT_SDRAM_D6                              ( PORTE )
#define PORT_SDRAM_D7                              ( PORTE )
#define PORT_SDRAM_D8                              ( PORTE )
#define PORT_SDRAM_D9                              ( PORTE )
#define PORT_SDRAM_D10                             ( PORTE )
#define PORT_SDRAM_D11                             ( PORTE )
#define PORT_SDRAM_D12                             ( PORTE )
#define PORT_SDRAM_D13                             ( PORTD )
#define PORT_SDRAM_D14                             ( PORTD )
#define PORT_SDRAM_D15                             ( PORTD )
#define PIN_SDRAM_D0                               ( 14 )
#define PIN_SDRAM_D1                               ( 15 )
#define PIN_SDRAM_D2                               ( 0 )
#define PIN_SDRAM_D3                               ( 1 )
#define PIN_SDRAM_D4                               ( 7 )
#define PIN_SDRAM_D5                               ( 8 )
#define PIN_SDRAM_D6                               ( 9 )
#define PIN_SDRAM_D7                               ( 10 )
#define PIN_SDRAM_D8                               ( 11 )
#define PIN_SDRAM_D9                               ( 12 )
#define PIN_SDRAM_D10                              ( 13 )
#define PIN_SDRAM_D11                              ( 14 )
#define PIN_SDRAM_D12                              ( 15 )
#define PIN_SDRAM_D13                              ( 8 )
#define PIN_SDRAM_D14                              ( 9 )
#define PIN_SDRAM_D15                              ( 10 )

#define PORT_SDRAM_D16                             ( PORTH )
#define PORT_SDRAM_D17                             ( PORTH )
#define PORT_SDRAM_D18                             ( PORTH )
#define PORT_SDRAM_D19                             ( PORTH )
#define PORT_SDRAM_D20                             ( PORTH )
#define PORT_SDRAM_D21                             ( PORTH )
#define PORT_SDRAM_D22                             ( PORTH )
#define PORT_SDRAM_D23                             ( PORTH )
#define PORT_SDRAM_D24                             ( PORTI )
#define PORT_SDRAM_D25                             ( PORTI )
#define PORT_SDRAM_D26                             ( PORTI )
#define PORT_SDRAM_D27                             ( PORTI )
#define PORT_SDRAM_D28                             ( PORTI )
#define PORT_SDRAM_D29                             ( PORTI )
#define PORT_SDRAM_D30                             ( PORTI )
#define PORT_SDRAM_D31                             ( PORTI )
#define PIN_SDRAM_D16                              ( 8 )
#define PIN_SDRAM_D17                              ( 9 )
#define PIN_SDRAM_D18                              ( 10 )
#define PIN_SDRAM_D19                              ( 11 )
#define PIN_SDRAM_D20                              ( 12 )
#define PIN_SDRAM_D21                              ( 13 )
#define PIN_SDRAM_D22                              ( 14 )
#define PIN_SDRAM_D23                              ( 15 )
#define PIN_SDRAM_D24                              ( 0 )
#define PIN_SDRAM_D25                              ( 1 )
#define PIN_SDRAM_D26                              ( 2 )
#define PIN_SDRAM_D27                              ( 3 )
#define PIN_SDRAM_D28                              ( 6 )
#define PIN_SDRAM_D29                              ( 7 )
#define PIN_SDRAM_D30                              ( 9 )
#define PIN_SDRAM_D31                              ( 10 )
/***************************************************************************************************
                                 
***************************************************************************************************/ 
#define SDRAM_BURST_LENGTH_1                       ( 0 )
#define SDRAM_BURST_LENGTH_2                       ( 1 )
#define SDRAM_BURST_LENGTH_4                       ( 2 )
#define SDRAM_BURST_LENGTH_8                       ( 3 )
#define SDRAM_BURST_FULL_PAGE                      ( 7 )

#define BURST_READ_BURST_WRITE                     ( 0 )
#define BURST_READ_SINGLE_WRITE                    ( 1 )
/***************************************************************************************************

***************************************************************************************************/
#define SDRAM_SIZE                                 ( 4UL * 1024UL * 1024UL *  2UL )
/***************************************************************************************************

***************************************************************************************************/
extern void SDRAM_Init( void );
/**************************************************************************************************/
#endif /*__SDRAM_DRIVER_H__*/
/***************************************************************************************************
                                       END OF FILE
***************************************************************************************************/

