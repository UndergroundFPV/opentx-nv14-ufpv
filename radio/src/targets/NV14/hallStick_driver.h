/***************************************************************************************************
file:
funciton:
auther:
***************************************************************************************************/
/***************************************************************************************************

***************************************************************************************************/
#ifndef      __HALLSTICK_DRIVER_H__
    #define  __HALLSTICK_DRIVER_H__
/***************************************************************************************************

***************************************************************************************************/
    #ifdef      EXTERN
        #undef  EXTERN
    #endif

    #ifdef  __HALLSTICK_DRIVER_C__
        #define EXTERN
    #else
        #define EXTERN  extern
    #endif
/***************************************************************************************************

***************************************************************************************************/
#define HALLSTICK_UART                  ( UART_ID4 )
#define HALLSTICK_BUFF_SIZE             ( 254 )

#define FLYSKY_HALL_BAUDRATE            (961200)

typedef  struct
{
    signed short Min;
    signed short Mid;
    signed short Max;
} STRUCT_STICK_CALIBRATION;

typedef  struct
{
    STRUCT_STICK_CALIBRATION SticksCalibration[4];
    unsigned char Reststate;
    unsigned short CRC16;
}STRUCT_STICK_CALIBRATION_PACK;

typedef  struct
{
    signed short Channel[4];
    signed short ServoValue;
 /*   signed short Channel2;
    signed short Channel3;
    signed short Channel4;
 */

}STRUCT_CHANNEL;

typedef  struct
{
    STRUCT_CHANNEL Channel;
    unsigned char StickState;
    unsigned short CRC16;
}STRUCT_CHANNEL_PACK;

typedef  union
{
    STRUCT_STICK_CALIBRATION_PACK ChannelPack;
    STRUCT_CHANNEL_PACK SticksCalibrationPack;
}UNION_DATA;

typedef  struct
{
    unsigned char Start;
  unsigned char SenderID:2;
  unsigned char ReceiverID:2;
  unsigned char PacketID:4;
    unsigned char Length;
    UNION_DATA Payload;
} STRUCT_HALLDATA;

typedef  struct
{
  unsigned char SenderID:2;
  unsigned char ReceiverID:2;
  unsigned char PacketID:4;
} STRUCT_HALLID;

typedef  union
{
    STRUCT_HALLID Hall_Id;
    unsigned char ID;
}STRUCT_ID;


typedef  union
{
    STRUCT_HALLDATA Halldat;
    unsigned char buffer[30];
}UNION_HALLDATA;


typedef  struct
{

    unsigned char Head;
    STRUCT_ID HallID;
    unsigned char Length;
    unsigned char Data[HALLSTICK_BUFF_SIZE];
    unsigned char Reserved[15];
    unsigned short CheckSum;
    unsigned char StickState;
    unsigned char StartIndex;
    unsigned char EndIndex;
    unsigned char Index;
    unsigned char DataIndex;
    unsigned char Deindex;
    unsigned char CompleteFlg;
    unsigned char Status;
    unsigned char Recevied;
    unsigned char msg_OK;
  //unsigned char RxBuffer[HALLSTICK_BUFF_SIZE];
    //UNION_HALLDATA Payload;
}STRUCT_HALL;

enum
{
   // UNINIT=0,
    GET_START = 0,
    GET_ID,
    GET_LENGTH,
    GET_DATA,
    GET_STATE,
    GET_CHECKSUM,
    CHECKSUM,
};


#define HALL_PROTOLO_HEAD                ( 0x55 )

#define HALL_SERIAL_USART                 UART4
#define HALL_SERIAL_GPIO                  GPIOA
#define HALL_DMA_Channel_RX               DMA_Channel_4
#define HALL_SERIAL_TX_GPIO_PIN           GPIO_Pin_0  // PC.06
#define HALL_SERIAL_RX_GPIO_PIN           GPIO_Pin_1  // PC.07
#define HALL_SERIAL_TX_GPIO_PinSource     GPIO_PinSource0
#define HALL_SERIAL_RX_GPIO_PinSource     GPIO_PinSource1
#define HALL_SERIAL_GPIO_AF               GPIO_AF_UART4

#define HALL_SERIAL_USART_IRQHandler      UART4_IRQHandler
#define HALL_SERIAL_USART_IRQn            UART4_IRQn
#define HALL_DMA_Stream_RX                DMA1_Stream2

#define HALL_RX_DMA_Stream_IRQHandler     DMA1_Stream2_IRQHandler



/***************************************************************************************************
                                         interface function
***************************************************************************************************/
EXTERN unsigned char HALL_DataBuffer[HALLSTICK_BUFF_SIZE];
void ResetHall( void );
void Get_factory( void );
extern void HallStick_Init(void);
void HALL_UART_TransmitData( unsigned char *pData, unsigned int Length );
unsigned short  Calculation_CRC16(unsigned char *pBuffer,unsigned char BufferSize);
void Hall_Task( void );

#endif

















