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

#ifndef _HAL_H_
#define _HAL_H_

/* DMA Allocation:
   DMA/Stream/Channel
   1/0/0 TIM4_CH1 (Ext module PPM)
   1/0/1 or 1/5/1 I2C1_RX (EEPROM, touch)
   1/1/4 USART3_RX (Internal module)
   1/3/7 USART3_TX (Internal module)
   1/4/0 SPI2_TX (LCD)
   1/5/4 Serial2 RX (disabled because of Audio)
   1/5/7 DAC/Audio
   1/6/1 or 1/7/1 I2C1_TX (EEPROM, touch)  - NOT USED
   2/0/3 or 2/2/3 (Gimbals)
   2/4/0 ADC1
   2/5/3 or 2/3/3 SPI1_TX (Gimbals)
*/

// Trims
#define TRIMS_GPIO_REG_LHL            GPIOE->IDR
#define TRIMS_GPIO_PIN_LHL            GPIO_Pin_0  // PE.00
#define TRIMS_GPIO_REG_LHR            GPIOD->IDR
#define TRIMS_GPIO_PIN_LHR            GPIO_Pin_14 // PD.14
#define TRIMS_GPIO_REG_LVD            GPIOD->IDR
#define TRIMS_GPIO_PIN_LVD            GPIO_Pin_1  // PD.01
#define TRIMS_GPIO_REG_LVU            GPIOE->IDR
#define TRIMS_GPIO_PIN_LVU            GPIO_Pin_1  // PE.01
#define TRIMS_GPIO_REG_RVD            GPIOD->IDR
#define TRIMS_GPIO_PIN_RVD            GPIO_Pin_15 // PD.15
#define TRIMS_GPIO_REG_RHL            GPIOD->IDR
#define TRIMS_GPIO_PIN_RHL            GPIO_Pin_10 // PD.10
#define TRIMS_GPIO_REG_RVU            GPIOD->IDR
#define TRIMS_GPIO_PIN_RVU            GPIO_Pin_11 // PD.11
#define TRIMS_GPIO_REG_RHR            GPIOD->IDR
#define TRIMS_GPIO_PIN_RHR            GPIO_Pin_3  // PD.03

// Keys
#define KEYS_GPIO_REG_K1              GPIOD->IDR
#define KEYS_GPIO_PIN_K1              GPIO_Pin_8  // PD.08
#define KEYS_GPIO_REG_K2              GPIOD->IDR
#define KEYS_GPIO_PIN_K2              GPIO_Pin_9  // PD.09

#define KEYS_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE)
#define KEYS_GPIOE_PINS               (TRIMS_GPIO_PIN_LHL | TRIMS_GPIO_PIN_LVU)
#define KEYS_GPIOD_PINS               (TRIMS_GPIO_PIN_LHR | TRIMS_GPIO_PIN_LVD | TRIMS_GPIO_PIN_RVD | TRIMS_GPIO_PIN_RHL | TRIMS_GPIO_PIN_RVU | TRIMS_GPIO_PIN_RHR | KEYS_GPIO_PIN_K1 | KEYS_GPIO_PIN_K2)

// ADC
#define ADC_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
#define ADC_RCC_APB2Periph            (RCC_APB2Periph_ADC1)

#define ADC_GPIO_PIN_SWA              GPIO_Pin_3  // PA.03
#define ADC_GPIO_PIN_SWB              GPIO_Pin_6  // PA.06
#define ADC_GPIO_PIN_SWC              GPIO_Pin_7  // PA.07
#define ADC_GPIO_PIN_SWD              GPIO_Pin_4  // PC.04
#define ADC_GPIO_PIN_SWE              GPIO_Pin_3  // PC.03
#define ADC_GPIO_PIN_SWF              GPIO_Pin_2  // PA.02
#define ADC_GPIO_PIN_VRA              GPIO_Pin_5  // PC.05
#define ADC_GPIO_PIN_VRB              GPIO_Pin_0  // PB.00
#define ADC_GPIO_PIN_LIBATT           GPIO_Pin_0  // PC.00
#define ADC_GPIO_PIN_DRYBATT          GPIO_Pin_2  // PC.02

#define ADC_GPIOA_PINS                (GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_6 | GPIO_Pin_7)
#define ADC_GPIOB_PINS                (GPIO_Pin_0)
#define ADC_GPIOC_PINS                (GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5)

#define ADC_CHANNEL_SWA               ADC_Channel_3 // ADC123_IN3
#define ADC_CHANNEL_SWB               ADC_Channel_6 // ADC12_IN6
#define ADC_CHANNEL_SWC               ADC_Channel_7 // ADC12_IN7
#define ADC_CHANNEL_SWD               ADC_Channel_14 // ADC12_IN14
#define ADC_CHANNEL_SWE               ADC_Channel_13 // ADC123_IN13
#define ADC_CHANNEL_SWF               ADC_Channel_2 // ADC123_IN2
#define ADC_CHANNEL_POT1              ADC_Channel_15 // ADC12_IN15
#define ADC_CHANNEL_POT2              ADC_Channel_8  // ADC12_IN8
#define ADC_CHANNEL_LIBATT            ADC_Channel_10 // ADC123_IN10
#define ADC_CHANNEL_DRYBATT           ADC_Channel_12 // ADC123_IN12

#define ADC_MAIN                      ADC1
#define ADC_DMA                       DMA2
#define ADC_DMA_SxCR_CHSEL            0
#define ADC_DMA_Stream                DMA2_Stream4
#define ADC_SET_DMA_FLAGS()           ADC_DMA->HIFCR = (DMA_HIFCR_CTCIF4 | DMA_HIFCR_CHTIF4 | DMA_HIFCR_CTEIF4 | DMA_HIFCR_CDMEIF4 | DMA_HIFCR_CFEIF4)
#define ADC_TRANSFER_COMPLETE()       (ADC_DMA->HISR & DMA_HISR_TCIF4)
#define ADC_SAMPTIME                  2   // sample time = 28 cycles

// Gimbals
#define GIMBALS_RCC_AHB1Periph        RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_DMA2
#define GIMBALS_RCC_APB2Periph        RCC_APB2Periph_SPI1
#define GIMBALS_SPI                   SPI1
#define GIMBALS_GPIO_AF               GPIO_AF_SPI1
#define GIMBALS_SPI_GPIO              GPIOB
#define GIMBALS_MOSI_PIN              GPIO_Pin_5 // PB.05
#define GIMBALS_MISO_PIN              GPIO_Pin_4 // PB.04
#define GIMBALS_SCK_PIN               GPIO_Pin_3 // PB.03
#define GIMBALS_MOSI_PinSource        GPIO_PinSource5
#define GIMBALS_MISO_PinSource        GPIO_PinSource4
#define GIMBALS_SCK_PinSource         GPIO_PinSource3
#define GIMBALS_CS_INT_GPIO           GPIOE
#define GIMBALS_LEFT_CS_GPIO_PIN      GPIO_Pin_9 // PE.09
#define GIMBALS_RIGHT_CS_GPIO_PIN     GPIO_Pin_10 // PE.10
#define GIMBALS_LEFT_INT_GPIO_PIN     GPIO_Pin_6 // PE.06
#define GIMBALS_RIGHT_INT_GPIO_PIN    GPIO_Pin_8 // PE.08
#define GIMBALS_DMA                   DMA2
#define GIMBALS_DMA_Stream_TX         DMA2_Stream5
#define GIMBALS_DMA_Stream_RX         DMA2_Stream0
#define GIMBALS_DMA_Stream_TX_IRQn    DMA2_Stream5_IRQn
#define GIMBALS_DMA_Stream_RX_IRQn    DMA2_Stream0_IRQn
#define GIMBALS_DMA_Stream_TX_IRQHandler  DMA2_Stream5_IRQHandler
#define GIMBALS_DMA_Stream_RX_IRQHandler  DMA2_Stream0_IRQHandler
// TODO: Check
#define GIMBALS_TX_DMA_FLAGS          (DMA_HIFCR_CTCIF5 | DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTEIF5 | DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CFEIF5)
#define GIMBALS_TX_DMA_FLAG_INT       DMA_HIFCR_CTCIF5
#define GIMBALS_RX_DMA_FLAGS          (DMA_HIFCR_CTCIF0 | DMA_HIFCR_CHTIF0 | DMA_HIFCR_CTEIF0 | DMA_HIFCR_CDMEIF0 | DMA_HIFCR_CFEIF0)
#define GIMBALS_RX_DMA_FLAG_INT       DMA_HIFCR_CTCIF0

// PWR and LED driver
#define PWR_RCC_AHB1Periph              (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE)
#define PWR_PRESS_BUTTON
#define PWR_SWITCH_GPIO                 GPIOA
#define PWR_SWITCH_GPIO_PIN             GPIO_Pin_5  // PA.05
#define PWR_ON_GPIO                     GPIOE
#define PWR_ON_GPIO_PIN                 GPIO_Pin_13 // PE.13
#define LED_RED_GPIO                    GPIOB
#define LED_RED_GPIO_PIN                GPIO_Pin_1  // PB.01

// Internal Module
#define INTMODULE
#define INTMODULE_RCC_AHB1Periph        (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA1)
#define INTMODULE_RCC_APB1Periph        (RCC_APB1Periph_USART3 | RCC_APB1Periph_TIM2)
#define INTMODULE_GPIO                  GPIOB
// #define INTMODULE_TX_GPIO               GPIOB
#define INTMODULE_TX_GPIO_PIN           GPIO_Pin_10 // PB.10
// #define INTMODULE_RX_GPIO               GPIOB
#define INTMODULE_RX_GPIO_PIN           GPIO_Pin_11 // PB.11
#define INTMODULE_TX_GPIO_PinSource     GPIO_PinSource10
#define INTMODULE_RX_GPIO_PinSource     GPIO_PinSource11
#define INTMODULE_GPIOB_PINS            (GPIO_Pin_10 | GPIO_Pin_11)
#define INTMODULE_GPIO_AF               GPIO_AF_USART3
#define INTMODULE_USART                 USART3
#define INTMODULE_USART_IRQn            USART3_IRQn
#define INTMODULE_USART_IRQHandler      USART3_IRQHandler
#define INTMODULE_DMA                   DMA1
#define INTMODULE_DMA_CHANNEL           DMA_Channel_4
#define INTMODULE_TX_DMA_STREAM         DMA1_Stream3
#define INTMODULE_TX_DMA_Stream_IRQn    DMA1_Stream3_IRQn
#define INTMODULE_TX_DMA_Stream_IRQHandler DMA1_Stream3_IRQHandler
#define INTMODULE_TX_DMA_FLAG_TC        DMA_IT_TCIF3
#define INTMODULE_TIMER                 TIM2
#define INTMODULE_TIMER_IRQn            TIM2_IRQn
#define INTMODULE_TIMER_IRQHandler      TIM2_IRQHandler
#define INTMODULE_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)

// External Module
#define EXTMODULE
#define EXTMODULE_PULSES
#define EXTMODULE_RCC_AHB1Periph        (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_DMA1)
#define EXTMODULE_RCC_APB1Periph        RCC_APB1Periph_TIM4
#define EXTMODULE_PWR_GPIO              GPIOE
#define EXTMODULE_PWR_GPIO_PIN          GPIO_Pin_15 // PE.15
#define EXTMODULE_PPM_GPIO              GPIOB
#define EXTMODULE_PPM_GPIO_PIN          GPIO_Pin_6  // PB.06
#define EXTMODULE_PPM_GPIO_PinSource    GPIO_PinSource6
#define EXTMODULE_TIMER                 TIM4
#define EXTMODULE_PPM_GPIO_AF           GPIO_AF_TIM4 // TIM4_CH1
#define EXTMODULE_TIMER_IRQn            TIM4_IRQn
#define EXTMODULE_TIMER_IRQHandler      TIM4_IRQHandler
#define EXTMODULE_DMA_CHANNEL           DMA_Channel_2
#define EXTMODULE_DMA_STREAM            DMA1_Stream0
#define EXTMODULE_DMA_IRQn              DMA1_Stream0_IRQn
#define EXTMODULE_DMA_IRQHandler        DMA1_Stream0_IRQHandler
#define EXTMODULE_DMA_FLAG_TC           DMA_IT_TCIF0
#define EXTMODULE_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)

// Trainer Port
#define TRAINERMODULE
#define TRAINER_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOC)
#define TRAINER_RCC_APB1Periph          RCC_APB1Periph_TIM4
#define TRAINER_GPIO                    GPIOD
#define TRAINER_IN_GPIO_PIN             GPIO_Pin_13 // PD.13
#define TRAINER_IN_GPIO_PinSource       GPIO_PinSource13
#define TRAINER_OUT_GPIO_PIN            GPIO_Pin_9  // PD.12
#define TRAINER_OUT_GPIO_PinSource      GPIO_PinSource12
#define TRAINER_TIMER                   TIM4
#define TRAINER_TIMER_IRQn              TIM4_IRQn
#define TRAINER_GPIO_AF                 GPIO_AF_TIM4 // TIM4_CH2 (in) + TIM4_CH1 (out)
#define TRAINER_OUT_DMA                 DMA1
// #define TRAINER_OUT_DMA_CHANNEL         DMA_Channel_2
// #define TRAINER_OUT_DMA_STREAM          DMA1_Stream3
// #define TRAINER_OUT_DMA_IRQn            DMA1_Stream3_IRQn
// #define TRAINER_OUT_DMA_IRQHandler      DMA1_Stream3_IRQHandler
// #define TRAINER_OUT_DMA_FLAG_TC         DMA_IT_TCIF3
#define TRAINER_EXTMODULE_TIMER_IRQn    TIM4_IRQn
#define TRAINER_EXTMODULE_TIMER_IRQHandler  TIM4_IRQHandler
#define TRAINER_EXTMODULE_TIMER_FREQ    (PERI1_FREQUENCY * TIMER_MULT_APB1)

// Serial Port
#define SERIAL_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA1)
#define SERIAL_RCC_APB1Periph           RCC_APB1Periph_USART2
#define SERIAL_GPIO                     GPIOD
#define SERIAL_TX_GPIO_PIN              GPIO_Pin_5 // PD.05
#define SERIAL_RX_GPIO_PIN              GPIO_Pin_6 // PD.06
#define SERIAL_TX_GPIO_PinSource        GPIO_PinSource5
#define SERIAL_RX_GPIO_PinSource        GPIO_PinSource6
#define SERIAL_GPIO_AF                  GPIO_AF_USART2
#define SERIAL_USART                    USART2
#define SERIAL_USART_IRQHandler         USART2_IRQHandler
#define SERIAL_USART_IRQn               USART2_IRQn
// DMA conflict with Audio
// #define SERIAL_DMA_Stream_RX            DMA1_Stream5
// #define SERIAL_DMA_Channel_RX           DMA_Channel_4

// Telemetry
#define TELEMETRY_RCC_AHB1Periph        (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
#define TELEMETRY_RCC_APB2Periph        RCC_APB2Periph_USART6
#define TELEMETRY_TXEN_GPIO             GPIOC
#define TELEMETRY_TXEN_GPIO_PIN         GPIO_Pin_1  // PC.01
#define TELEMETRY_RXEN_GPIO             GPIOC
#define TELEMETRY_RXEN_GPIO_PIN         GPIO_Pin_13 // PC.13
#define TELEMETRY_GPIO                  GPIOC
#define TELEMETRY_TX_GPIO_PIN           GPIO_Pin_6  // PC.06
#define TELEMETRY_RX_GPIO_PIN           GPIO_Pin_7  // PC.07
#define TELEMETRY_GPIO_PinSource_TX     GPIO_PinSource6
#define TELEMETRY_GPIO_PinSource_RX     GPIO_PinSource7
#define TELEMETRY_GPIO_AF               GPIO_AF_USART6
#define TELEMETRY_USART                 USART6
#define TELEMETRY_DMA_Stream_TX         DMA1_Stream7
#define TELEMETRY_DMA_Channel_TX        DMA_Channel_5
#define TELEMETRY_DMA_TX_Stream_IRQ     DMA1_Stream7_IRQn
#define TELEMETRY_DMA_TX_IRQHandler     DMA1_Stream7_IRQHandler
#define TELEMETRY_DMA_TX_FLAG_TC        DMA_IT_TCIF7
#define TELEMETRY_USART_IRQHandler      USART6_IRQHandler
#define TELEMETRY_USART_IRQn            USART6_IRQn

// Heartbeat
#define HEARTBEAT_RCC_AHB1Periph        RCC_AHB1Periph_GPIOC
#define HEARTBEAT_RCC_APB2Periph        RCC_APB2Periph_USART6
#define HEARTBEAT_GPIO                  GPIOC
#define HEARTBEAT_GPIO_PIN              GPIO_Pin_7  // PC.07
#define HEARTBEAT_GPIO_PinSource        GPIO_PinSource7
#define HEARTBEAT_GPIO_AF_SBUS          GPIO_AF_USART6
#define HEARTBEAT_GPIO_AF_CAPTURE       GPIO_AF_TIM3
#define HEARTBEAT_USART                 USART6
#define HEARTBEAT_USART_IRQHandler      USART6_IRQHandler
#define HEARTBEAT_USART_IRQn            USART6_IRQn
#define HEARTBEAT_DMA_Stream            DMA2_Stream1
#define HEARTBEAT_DMA_Channel           DMA_Channel_5

// USB
#define USB_RCC_AHB1Periph_GPIO         RCC_AHB1Periph_GPIOA
#define USB_GPIO                        GPIOA
#define USB_GPIO_PIN_VBUS               GPIO_Pin_9  // PA.09
#define USB_GPIO_PIN_DM                 GPIO_Pin_11 // PA.11
#define USB_GPIO_PIN_DP                 GPIO_Pin_12 // PA.12
#define USB_GPIO_PinSource_DM           GPIO_PinSource11
#define USB_GPIO_PinSource_DP           GPIO_PinSource12
#define USB_GPIO_AF                     GPIO_AF_OTG1_FS

// BackLight
#define BACKLIGHT_RCC_AHB1Periph      0
#define BACKLIGHT_RCC_APB1Periph      0
#define BACKLIGHT_RCC_APB2Periph      0
#define BACKLIGHT_GPIO                GPIOE
#define BACKLIGHT_GPIO_PIN            GPIO_Pin_7  // PE.07

// LCD driver
#define LCD_RCC_AHB1Periph             (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_APB1Periph_SPI2 | RCC_AHB1Periph_DMA1)
#define LCD_RCC_APB1Periph             RCC_APB1Periph_SPI2
#define LCD_RCC_APB2Periph             0
#define LCD_SPI_GPIO                   GPIOB
#define LCD_MOSI_GPIO_PIN              GPIO_Pin_15 // PB.15
#define LCD_MOSI_GPIO_PinSource        GPIO_PinSource15
#define LCD_CLK_GPIO_PIN               GPIO_Pin_13 // PB.13
#define LCD_CLK_GPIO_PinSource         GPIO_PinSource13
#define LCD_A0_GPIO                    GPIOE
#define LCD_A0_GPIO_PIN                GPIO_Pin_11 // PE.11, labeled RS on schematics
#define LCD_NCS_GPIO                   GPIOD
#define LCD_NCS_GPIO_PIN               GPIO_Pin_4  // PD.04
#define LCD_RST_GPIO                   GPIOE
#define LCD_RST_GPIO_PIN               GPIO_Pin_12 // PE.12
#define LCD_RW_GPIO                    GPIOE
#define LCD_RW_GPIO_PIN                GPIO_Pin_14 // PE.14
#define LCD_RD_GPIO                    GPIOD
#define LCD_RD_GPIO_PIN                GPIO_Pin_7  // PD.07
#define LCD_DMA                        DMA1
#define LCD_DMA_Stream                 DMA1_Stream4
#define LCD_DMA_Stream_IRQn            DMA1_Stream4_IRQn
#define LCD_DMA_Stream_IRQHandler      DMA1_Stream4_IRQHandler
#define LCD_DMA_FLAGS                  (DMA_HIFCR_CTCIF4 | DMA_HIFCR_CHTIF4 | DMA_HIFCR_CTEIF4 | DMA_HIFCR_CDMEIF4 | DMA_HIFCR_CFEIF4)
#define LCD_DMA_FLAG_INT               DMA_HIFCR_CTCIF4
#define LCD_SPI                        SPI2
#define LCD_GPIO_AF                    GPIO_AF_SPI2

// I2C Bus: EEPROM and FT6236 (touch)
#define I2C_RCC_AHB1Periph              RCC_AHB1Periph_GPIOB
#define I2C_RCC_APB1Periph              RCC_APB1Periph_I2C1
#define I2C                             I2C1
#define I2C_GPIO                        GPIOB
#define I2C_SCL_GPIO_PIN                GPIO_Pin_8  // PB.08
#define I2C_SDA_GPIO_PIN                GPIO_Pin_9  // PB.09
#define I2C_GPIO_AF                     GPIO_AF_I2C1
#define I2C_SCL_GPIO_PinSource          GPIO_PinSource8
#define I2C_SDA_GPIO_PinSource          GPIO_PinSource9
#define I2C_SPEED                       400000
#define I2C_DMA_RCC_AHB1Periph          RCC_AHB1Periph_DMA1
// Could use DMA to read touch panel, but stream conflict with Ext. module PPM TIM4_CH1
//#define I2C_DMA                         DMA1
#define I2C_DMA_RX_Channel              DMA_Channel_1
#define I2C_DMA_RX_Stream               DMA1_Stream0
#define I2C_DMA_RX_IRQn                 DMA1_Stream0_IRQn
#define I2C_DMA_RX_IRQHandler           DMA1_Stream0_IRQHandler
#define I2C_DMA_RX_IRQPriority          7
#define I2C_DMA_RX_FLAG_FEIF            DMA_FLAG_FEIF0
#define I2C_DMA_RX_FLAG_DMEIF           DMA_FLAG_DMEIF0
#define I2C_DMA_RX_FLAG_TEIF            DMA_FLAG_TEIF0
#define I2C_DMA_RX_FLAG_HTIF            DMA_FLAG_HTIF0
#define I2C_DMA_RX_FLAG_TCIF            DMA_FLAG_TCIF0
// I2C DMA TX unused
//#define I2C_DMA_TX_Channel              DMA1_Channel_1
//#define I2C_DMA_TX_Stream               DMA1_Stream6
//#define I2C_DMA_TX_IRQn                 DMA1_Stream6_IRQn
//#define I2C_DMA_TX_IRQHandler           DMA1_Stream6_IRQHandler
//#define I2C_DMA_TX_FLAG_FEIF            DMA_FLAG_FEIF6
//#define I2C_DMA_TX_FLAG_DMEIF           DMA_FLAG_DMEIF6
//#define I2C_DMA_TX_FLAG_TEIF            DMA_FLAG_TEIF6
//#define I2C_DMA_TX_FLAG_HTIF            DMA_FLAG_HTIF6
//#define I2C_DMA_TX_FLAG_TCIF            DMA_FLAG_TCIF6

// EEPROM
#define EEPROM_SIZE                     (32*1024)
#define EEPROM_PAGESIZE                 64
#define EEPROM_I2C_ADDRESS              0xA0

// FT6236 touch
#define TOUCH_SCREEN
#define TOUCH_RCC_AHB1Periph            RCC_AHB1Periph_GPIOD
#define TOUCH_RCC_APB2Periph            RCC_APB2Periph_SYSCFG
#define TOUCH_INT_GPIO                  GPIOD
#define TOUCH_INT_GPIO_PIN              GPIO_Pin_0
#define TOUCH_INT_GPIO_PinSource        GPIO_PinSource0
#define TOUCH_INT_EXTI_PortSource       EXTI_PortSourceGPIOD
#define TOUCH_INT_EXTI_Line             EXTI_Line0
#define TOUCH_INT_EXTI_IRQ              EXTI0_IRQn
#define TOUCH_INT_EXTI_IRQHandler       EXTI0_IRQHandler
#define TOUCH_INT_EXTI_IRQPriority      5
#define TOUCH_RST_GPIO                  GPIOA
#define TOUCH_RST_GPIO_PIN              GPIO_Pin_15
#define TOUCH_RST_GPIO_PinSource        GPIO_PinSource15
#define TOUCH_I2C_ADDRESS               0x70
#define TOUCH_POINTS                    2
#if defined(I2C_DMA)
  #define TOUCH_USE_DMA           1
#else
  #define TOUCH_USE_DMA           0
#endif

// SD
#define SD_RCC_AHB1Periph               (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
#define SD_RCC_APB1Periph               0
#define SD_PRESENT_GPIO                 0
#define SD_PRESENT_GPIO_PIN             0
#define SD_SDIO_DMA_STREAM              DMA2_Stream3
#define SD_SDIO_DMA_CHANNEL             DMA_Channel_4
#define SD_SDIO_DMA_FLAG_FEIF           DMA_FLAG_FEIF3
#define SD_SDIO_DMA_FLAG_DMEIF          DMA_FLAG_DMEIF3
#define SD_SDIO_DMA_FLAG_TEIF           DMA_FLAG_TEIF3
#define SD_SDIO_DMA_FLAG_HTIF           DMA_FLAG_HTIF3
#define SD_SDIO_DMA_FLAG_TCIF           DMA_FLAG_TCIF3
#define SD_SDIO_DMA_IRQn                DMA2_Stream3_IRQn
#define SD_SDIO_DMA_IRQHANDLER          DMA2_Stream3_IRQHandler
#define SD_SDIO_FIFO_ADDRESS            ((uint32_t)0x40012C80)
#define SD_SDIO_CLK_DIV(fq)             ((48000000 / (fq)) - 2)
#define SD_SDIO_INIT_CLK_DIV            SD_SDIO_CLK_DIV(400000)
#define SD_SDIO_TRANSFER_CLK_DIV        SD_SDIO_CLK_DIV(24000000)

// Audio
#define AUDIO_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_DMA1)
#define AUDIO_RCC_APB1Periph            (RCC_APB1Periph_TIM6 | RCC_APB1Periph_DAC)
#define AUDIO_OUTPUT_GPIO               GPIOA
#define AUDIO_OUTPUT_GPIO_PIN           GPIO_Pin_4  // PA.04
#define AUDIO_DMA_Stream                DMA1_Stream5
#define AUDIO_DMA_Stream_IRQn           DMA1_Stream5_IRQn
#define AUDIO_TIM_IRQn                  TIM6_DAC_IRQn
#define AUDIO_TIM_IRQHandler            TIM6_DAC_IRQHandler
#define AUDIO_DMA_Stream_IRQHandler     DMA1_Stream5_IRQHandler
#define AUDIO_TIMER                     TIM6
#define AUDIO_DMA                       DMA1

// Haptic
#define HAPTIC_RCC_AHB1Periph          RCC_AHB1Periph_GPIOA
#define HAPTIC_RCC_APB2Periph          RCC_APB2Periph_TIM1
#define HAPTIC_GPIO_PinSource          GPIO_PinSource8
#define HAPTIC_GPIO                    GPIOA
#define HAPTIC_GPIO_PIN                GPIO_Pin_8  // PA.08
#define HAPTIC_GPIO_AF                 GPIO_AF_TIM1
#define HAPTIC_TIMER                   TIM1

// Bluetooth
#define BT_USART                       USART6
#define BT_GPIO_AF                     GPIO_AF_USART6
#define BT_USART_IRQn                  USART6_IRQn
#define BT_RCC_AHB1Periph              (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC)
#define BT_RCC_APB1Periph              0
#define BT_RCC_APB2Periph              RCC_APB2Periph_USART6
#define BT_GPIO_TXRX                   GPIOC
#define BT_TX_GPIO_PIN                 GPIO_Pin_8  // PC.08
#define BT_RX_GPIO_PIN                 GPIO_Pin_7  // PC.07
#define BT_BLUETOOTH_STATE_GPIO        GPIOA
#define BT_BLUETOOTH_STATE_GPIO_PIN    GPIO_Pin_0  // PA.00
#define BT_BLUETOOTH_GPIO              GPIOB
#define BT_BLUETOOTH_GPIO_PIN          GPIO_Pin_14 // PB.14
#define BT_TX_GPIO_PinSource           GPIO_PinSource8
#define BT_RX_GPIO_PinSource           GPIO_PinSource7
#define BT_USART_IRQHandler            USART6_IRQHandler

// Xms Interrupt
#define INTERRUPT_xMS_RCC_APB1Periph   RCC_APB1Periph_TIM14
#define INTERRUPT_xMS_TIMER            TIM14
#define INTERRUPT_xMS_IRQn             TIM8_TRG_COM_TIM14_IRQn
#define INTERRUPT_xMS_IRQHandler       TIM8_TRG_COM_TIM14_IRQHandler

// 2MHz Timer
#define TIMER_2MHz_RCC_APB1Periph      RCC_APB1Periph_TIM7
#define TIMER_2MHz_TIMER               TIM7

#endif // _HAL_H_
