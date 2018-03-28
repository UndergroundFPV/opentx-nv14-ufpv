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

/* Timers Allocation:
 * TIM1 = Haptic
 * TIM4 = Trainer
 * TIM6 = Audio
 * TIM7 = 2MHz counter
 *
 *
 * TIM14 = 5ms counter
 */

/* DMA Allocation:
   DMA/Stream/Channel
   1/5/7 DAC/Audio
   2/0/0 ADC1
*/

// Switches
#define SWITCHES_RCC_AHB1Periph         RCC_AHB1Periph_GPIOJ
#define SWITCHES_GPIO_REG_F             GPIOJ->IDR
#define SWITCHES_GPIO_PIN_F             GPIO_Pin_1  // PJ.01
#define SWITCHES_GPIO_REG_H             GPIOJ->IDR
#define SWITCHES_GPIO_PIN_H             GPIO_Pin_12 // PJ.12

// Trims
#define TRIMS_GPIO_REG_RHL              GPIOF->IDR
#define TRIMS_GPIO_PIN_RHL              GPIO_Pin_10 // PF.10
#define TRIMS_GPIO_REG_RHR              GPIOG->IDR
#define TRIMS_GPIO_PIN_RHR              GPIO_Pin_11 // PG.11
#define TRIMS_GPIO_REG_RVD              GPIOG->IDR
#define TRIMS_GPIO_PIN_RVD              GPIO_Pin_2  // PG.02
#define TRIMS_GPIO_REG_RVU              GPIOH->IDR
#define TRIMS_GPIO_PIN_RVU              GPIO_Pin_7  // PH.07
#define TRIMS_GPIO_REG_LVD              GPIOH->IDR
#define TRIMS_GPIO_PIN_LVD              GPIO_Pin_2  // PH.02
#define TRIMS_GPIO_REG_LHL              GPIOB->IDR
#define TRIMS_GPIO_PIN_LHL              GPIO_Pin_15 // PB.15
#define TRIMS_GPIO_REG_LVU              GPIOC->IDR
#define TRIMS_GPIO_PIN_LVU              GPIO_Pin_13 // PC.13
#define TRIMS_GPIO_REG_LHR              GPIOD->IDR
#define TRIMS_GPIO_PIN_LHR              GPIO_Pin_7  // PD.07
/*
#define TRIMS_GPIO_REG_RSD              GPIOJ->IDR
#define TRIMS_GPIO_PIN_RSD              GPIO_Pin_13 // PJ.13
#define TRIMS_GPIO_REG_RSU              GPIOG->IDR
#define TRIMS_GPIO_PIN_RSU              GPIO_Pin_10 // PG.10
 */

// Index of all switches / trims
#define KEYS_GPIOB_PINS                 (GPIO_Pin_15)
#define KEYS_GPIOC_PINS                 (GPIO_Pin_13)
#define KEYS_GPIOD_PINS                 (GPIO_Pin_7)
#define KEYS_GPIOF_PINS                 (GPIO_Pin_10)
#define KEYS_GPIOG_PINS                 (GPIO_Pin_2 | GPIO_Pin_10 | GPIO_Pin_11)
#define KEYS_GPIOH_PINS                 (GPIO_Pin_2 | GPIO_Pin_7)
#define KEYS_GPIOJ_PINS                 (GPIO_Pin_1 | GPIO_Pin_12 | GPIO_Pin_13)

// ADC
#define ADC_RCC_AHB1Periph              (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
#define ADC_RCC_APB2Periph              (RCC_APB2Periph_ADC1)
#define ADC_GPIO_PIN_STICK_LH           GPIO_Pin_0      // PA.00
#define ADC_GPIO_PIN_STICK_LV           GPIO_Pin_1      // PA.01
#define ADC_GPIO_PIN_STICK_RH           GPIO_Pin_2      // PA.02
#define ADC_GPIO_PIN_STICK_RV           GPIO_Pin_3      // PA.03
#define ADC_GPIO_PIN_POT1               GPIO_Pin_4      // PC.04
#define ADC_GPIO_PIN_POT2               GPIO_Pin_1      // PA.06
#define ADC_GPIO_PIN_SWA                GPIO_Pin_0      // PC.00
#define ADC_GPIO_PIN_SWB                GPIO_Pin_1      // PC.01
#define ADC_GPIO_PIN_SWC                GPIO_Pin_2      // PC.02
#define ADC_GPIO_PIN_SWD                GPIO_Pin_5      // PA.05
#define ADC_GPIO_PIN_SWE                GPIO_Pin_0      // PB.00
#define ADC_GPIO_PIN_SWG                GPIO_Pin_1      // PB.01
#define ADC_GPIO_PIN_BATT               GPIO_Pin_5      // PC.05
#define ADC_GPIOA_PINS                  (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_6)
#define ADC_GPIOB_PINS                  (GPIO_Pin_0 | GPIO_Pin_1)
#define ADC_GPIOC_PINS                  (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5)
#define ADC_CHANNEL_STICK_LH            ADC_Channel_0   // ADC1_IN0
#define ADC_CHANNEL_STICK_LV            ADC_Channel_1   // ADC1_IN1
#define ADC_CHANNEL_STICK_RH            ADC_Channel_2   // ADC1_IN2
#define ADC_CHANNEL_STICK_RV            ADC_Channel_3   // ADC1_IN3
#define ADC_CHANNEL_POT1                ADC_Channel_14  // ADC1_IN14
#define ADC_CHANNEL_POT2                ADC_Channel_6   // ADC1_IN6
#define ADC_CHANNEL_SWA                 ADC_Channel_10  // ADC1_IN10
#define ADC_CHANNEL_SWB                 ADC_Channel_11  // ADC1_IN11
#define ADC_CHANNEL_SWC                 ADC_Channel_12  // ADC1_IN12
#define ADC_CHANNEL_SWD                 ADC_Channel_5   // ADC1_IN5
#define ADC_CHANNEL_SWE                 ADC_Channel_8   // ADC1_IN8
#define ADC_CHANNEL_SWG                 ADC_Channel_9   // ADC1_IN9
#define ADC_CHANNEL_BATT                ADC_Channel_15  // ADC1_IN15
#define ADC_MAIN                        ADC1
#define ADC_SAMPTIME                    3
#define ADC_DMA                         DMA2
#define ADC_DMA_SxCR_CHSEL              DMA_SxCR_CHSEL_1
#define ADC_DMA_Stream                  DMA2_Stream0
#define ADC_SET_DMA_FLAGS()             ADC_DMA->LIFCR = (DMA_LIFCR_CTCIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTEIF0 | DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CFEIF0)
#define ADC_TRANSFER_COMPLETE()         (ADC_DMA->LISR & DMA_LISR_TCIF0)

// Power
#define PWR_RCC_AHB1Periph              RCC_AHB1Periph_GPIOI
#define PWR_GPIO                        GPIOI
#define PWR_SWITCH_GPIO_REG             PWR_GPIO->IDR
#define PWR_SWITCH_GPIO_PIN             GPIO_Pin_11 // PI.11
#define PWR_ON_GPIO_PIN                 GPIO_Pin_14 // PI.14
#define PWR_ON_GPIO_MODER               GPIO_MODER_MODER1
#define PWR_ON_GPIO_MODER_OUT           GPIO_MODER_MODER1_0

// S.Port update connector
#define SPORT_MAX_BAUDRATE              250000 // < 400000
#if defined(PCBX10)
  #define SPORT_UPDATE_RCC_AHB1Periph   RCC_AHB1Periph_GPIOH
  #define SPORT_UPDATE_PWR_GPIO         GPIOH
  #define SPORT_UPDATE_PWR_GPIO_PIN     GPIO_Pin_13  // PH.13
#else
  #define SPORT_UPDATE_RCC_AHB1Periph   0
#endif

// Led
#define STATUS_LEDS
#if defined(PCBX12S)
  #define LED_RCC_AHB1Periph            RCC_AHB1Periph_GPIOI
  #define LED_GPIO                      GPIOI
  #define LED_GPIO_PIN                  GPIO_Pin_5  // PI.05
#elif defined(PCBX10)
  #define LED_RCC_AHB1Periph            RCC_AHB1Periph_GPIOE
  #define LED_GPIO                      GPIOE
  #define LED_RED_GPIO_PIN              GPIO_Pin_2
  #define LED_GREEN_GPIO_PIN            GPIO_Pin_4
  #define LED_BLUE_GPIO_PIN             GPIO_Pin_5
  #define LED_GPIO_PIN                  (LED_RED_GPIO_PIN | LED_GREEN_GPIO_PIN | LED_BLUE_GPIO_PIN)
#endif

// Serial Port (DEBUG)
#define SERIAL_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA1)
#define SERIAL_RCC_APB1Periph           RCC_APB1Periph_USART3
#define SERIAL_GPIO                     GPIOB
#define SERIAL_TX_GPIO_PIN              GPIO_Pin_10 // PB.10
#define SERIAL_RX_GPIO_PIN              GPIO_Pin_11 // PB.11
#define SERIAL_TX_GPIO_PinSource        GPIO_PinSource10
#define SERIAL_RX_GPIO_PinSource        GPIO_PinSource11
#define SERIAL_GPIO_AF                  GPIO_AF_USART3
#define SERIAL_USART                    USART3
#define SERIAL_USART_IRQHandler         USART3_IRQHandler
#define SERIAL_USART_IRQn               USART3_IRQn
#define SERIAL_DMA_Stream_RX            DMA1_Stream1
#define SERIAL_DMA_Channel_RX           DMA_Channel_4

// Telemetry
#define TELEMETRY_RCC_AHB1Periph        (RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA1)
#define TELEMETRY_RCC_APB1Periph        RCC_APB1Periph_USART2
#define TELEMETRY_DIR_GPIO              GPIOD
#define TELEMETRY_DIR_GPIO_PIN          GPIO_Pin_4  // PD.04
#define TELEMETRY_GPIO                  GPIOD
#define TELEMETRY_TX_GPIO_PIN           GPIO_Pin_5  // PD.05
#define TELEMETRY_RX_GPIO_PIN           GPIO_Pin_6  // PD.06
#define TELEMETRY_GPIO_PinSource_TX     GPIO_PinSource5
#define TELEMETRY_GPIO_PinSource_RX     GPIO_PinSource6
#define TELEMETRY_GPIO_AF               GPIO_AF_USART2
#define TELEMETRY_USART                 USART2
#if defined(PCBX12S)
#define TELEMETRY_DMA_Stream_RX         DMA1_Stream5
#define TELEMETRY_DMA_Channel_RX        DMA_Channel_4
#endif
#define TELEMETRY_DMA_Stream_TX         DMA1_Stream6
#define TELEMETRY_DMA_Channel_TX        DMA_Channel_4
#define TELEMETRY_DMA_TX_Stream_IRQ     DMA1_Stream6_IRQn
#define TELEMETRY_DMA_TX_IRQHandler     DMA1_Stream6_IRQHandler
#define TELEMETRY_DMA_TX_FLAG_TC        DMA_IT_TCIF6
#define TELEMETRY_USART_IRQHandler      USART2_IRQHandler
#define TELEMETRY_USART_IRQn            USART2_IRQn

// USB
#define USB_RCC_AHB1Periph_GPIO         RCC_AHB1Periph_GPIOA
#define USB_GPIO                        GPIOA
#define USB_GPIO_PIN_VBUS               GPIO_Pin_9  // PA.09
#define USB_GPIO_PIN_DM                 GPIO_Pin_11 // PA.11
#define USB_GPIO_PIN_DP                 GPIO_Pin_12 // PA.12
#define USB_GPIO_PinSource_DM           GPIO_PinSource11
#define USB_GPIO_PinSource_DP           GPIO_PinSource12
#define USB_GPIO_AF                     GPIO_AF_OTG1_FS

// LCD
#define LCD_RCC_AHB1Periph              (RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOJ | RCC_AHB1Periph_GPIOK | RCC_AHB1Periph_DMA2D)
#define LCD_RCC_APB1Periph              0
#define LCD_RCC_APB2Periph              RCC_APB2Periph_LTDC
#define LCD_GPIO_NRST                   GPIOG
#define LCD_GPIO_PIN_NRST               GPIO_Pin_9  // PG.09
#define LTDC_IRQ_PRIO                   4
#define DMA_SCREEN_IRQ_PRIO             6

// Backlight
// TODO TIM3, TIM8, TIM14, review the channel in backlight_driver.cpp according to the chosen timer
#define BACKLIGHT_RCC_AHB1Periph        RCC_AHB1Periph_GPIOA
#define BACKLIGHT_GPIO                  GPIOA
#define BACKLIGHT_TIMER                 TIM8
#define BACKLIGHT_GPIO_PIN              GPIO_Pin_7  // PA.07
#define BACKLIGHT_GPIO_PinSource        GPIO_PinSource7
#define BACKLIGHT_RCC_APB1Periph        0
#define BACKLIGHT_RCC_APB2Periph        RCC_APB2Periph_TIM8
#define BACKLIGHT_GPIO_AF               GPIO_AF_TIM8
#define BACKLIGHT_TIMER_FREQ            (PERI2_FREQUENCY * TIMER_MULT_APB2)

// SD card
#define SD_RCC_AHB1Periph               (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
#define SD_RCC_APB1Periph               0
#define SD_PRESENT_GPIO                 GPIOC
#define SD_PRESENT_GPIO_PIN             GPIO_Pin_5  // PC.05
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

// SDRAM
#define SDRAM_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOH | RCC_AHB1Periph_GPIOI)
#define SDRAM_RCC_AHB3Periph            RCC_AHB3Periph_FMC

// SPI FLASH
#define EEPROM_RCC_AHB1Periph           RCC_AHB1Periph_GPIOG
#define EEPROM_RCC_APB1Periph           RCC_APB1Periph_SPI6
#define EEPROM_SPI_CS_GPIO              GPIOG
#define EEPROM_SPI_CS_GPIO_PIN          GPIO_Pin_6 // PG.06
#define EEPROM_SPI_SCK_GPIO             GPIOG
#define EEPROM_SPI_SCK_GPIO_PIN         GPIO_Pin_13 // PG.13
#define EEPROM_SPI_SCK_GPIO_PinSource   GPIO_PinSource13
#define EEPROM_SPI_MISO_GPIO            GPIOG
#define EEPROM_SPI_MISO_GPIO_PIN        GPIO_Pin_12 // PG.12
#define EEPROM_SPI_MISO_GPIO_PinSource  GPIO_PinSource12
#define EEPROM_SPI_MOSI_GPIO            GPIOG
#define EEPROM_SPI_MOSI_GPIO_PIN        GPIO_Pin_14 // PG.14
#define EEPROM_SPI_MOSI_GPIO_PinSource  GPIO_PinSource14

// Audio
#define AUDIO_RCC_APB1Periph            (RCC_APB1Periph_TIM6 | RCC_APB1Periph_DAC)
#define AUDIO_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_DMA1)
#define AUDIO_OUTPUT_GPIO               GPIOA
#define AUDIO_OUTPUT_GPIO_PIN           GPIO_Pin_4  // PA.04
#define AUDIO_GPIO_AF                   GPIO_AF_DAC1
#define AUDIO_GPIO_PinSource            GPIO_PinSource4
#define AUDIO_DMA_Stream                DMA1_Stream5
#define AUDIO_DMA_Stream_IRQn           DMA1_Stream5_IRQn
#define AUDIO_TIM_IRQn                  TIM6_DAC_IRQn
#define AUDIO_TIM_IRQHandler            TIM6_DAC_IRQHandler
#define AUDIO_DMA_Stream_IRQHandler     DMA1_Stream5_IRQHandler
#define AUDIO_TIMER                     TIM6
#define AUDIO_DMA                       DMA1

// I2C Bus

// Haptic: TIM1_CH2
#define HAPTIC_PWM
#define HAPTIC_RCC_AHB1Periph           RCC_AHB1Periph_GPIOA
#define HAPTIC_RCC_APB2Periph           RCC_APB2ENR_TIM1EN
#define HAPTIC_GPIO                     GPIOA
#define HAPTIC_GPIO_PIN                 GPIO_Pin_9
#define HAPTIC_GPIO_TIMER               TIM1
#define HAPTIC_GPIO_AF                  GPIO_AF_TIM1
#define HAPTIC_GPIO_PinSource           GPIO_PinSource9
#define HAPTIC_TIMER_OUTPUT_ENABLE      TIM_CCER_CC2E
#define HAPTIC_TIMER_MODE               TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2
#define HAPTIC_TIMER_COMPARE_VALUE      HAPTIC_GPIO_TIMER->CCR2

// Internal Module
#define INTMODULE
#define INTMODULE_RCC_AHB1Periph        (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA2)
#define INTMODULE_PWR_GPIO              GPIOA
#define INTMODULE_PWR_GPIO_PIN          GPIO_Pin_8  // PA.08
#define INTMODULE_TX_GPIO               GPIOB
#define INTMODULE_TX_GPIO_PIN           GPIO_Pin_6  // PB.06
#define INTMODULE_RX_GPIO               GPIOB
#define INTMODULE_RX_GPIO_PIN           GPIO_Pin_7  // PB.07
#define INTMODULE_TX_GPIO_PinSource     GPIO_PinSource6
#define INTMODULE_RX_GPIO_PinSource     GPIO_PinSource7
#define INTMODULE_USART                 USART1
#define INTMODULE_TX_GPIO_AF            GPIO_AF_USART1
#define INTMODULE_USART_IRQn            USART1_IRQn
#define INTMODULE_TX_DMA_STREAM         DMA2_Stream7
#define INTMODULE_TX_DMA_Stream_IRQn    DMA2_Stream7_IRQn
#define INTMODULE_TX_DMA_Stream_IRQHandler DMA2_Stream7_IRQHandler
#define INTMODULE_TX_DMA_FLAG_TC        DMA_IT_TCIF7
#define INTMODULE_DMA_CHANNEL           DMA_Channel_4
#if defined(PCBX12S)
  #define INTMODULE_BOOT_GPIO           GPIOI
  #define INTMODULE_BOOT_GPIO_PIN       GPIO_PIN_9  // PC.02
#elif defined(PCBX10)
  #define INTMODULE_BOOT_GPIO           GPIOI
  #define INTMODULE_BOOT_GPIO_PIN       GPIO_PIN_9  // PI.09
#endif
#if defined(PCBX10) || PCBREV >= 13
  #define INTMODULE_RCC_APB1Periph      RCC_APB1Periph_TIM2
  #define INTMODULE_RCC_APB2Periph      RCC_APB2Periph_USART1
  #define INTMODULE_TIMER               TIM2
  #define INTMODULE_TIMER_IRQn          TIM2_IRQn
  #define INTMODULE_TIMER_IRQHandler    TIM2_IRQHandler
  #define INTMODULE_TIMER_FREQ          (PERI1_FREQUENCY * TIMER_MULT_APB1)
#else
  #define INTMODULE_RCC_APB1Periph      0
  #define INTMODULE_RCC_APB2Periph      (RCC_APB2Periph_TIM1 | RCC_APB2Periph_USART1)
  #define INTMODULE_TIMER               TIM1
  #define INTMODULE_TIMER_IRQn          TIM1_CC_IRQn
  #define INTMODULE_TIMER_IRQHandler    TIM1_CC_IRQHandler
  #define INTMODULE_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
#endif

// External Module
#define EXTMODULE
#define EXTMODULE_PULSES
#define EXTMODULE_PWR_GPIO              GPIOB
#define EXTMODULE_PWR_GPIO_PIN          GPIO_Pin_3  // PB.03
#if defined(PCBX10) || PCBREV >= 13
  #define EXTMODULE_RCC_AHB1Periph      (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA2)
  #define EXTMODULE_RCC_APB1Periph      0
  #define EXTMODULE_RCC_APB2Periph      RCC_APB2Periph_TIM1
  #define EXTMODULE_TX_GPIO             GPIOA
  #define EXTMODULE_TX_GPIO_PIN         GPIO_Pin_10 // PA.10
  #define EXTMODULE_TX_GPIO_PinSource   GPIO_PinSource10
  #define EXTMODULE_TX_GPIO_AF          GPIO_AF_TIM1
  #define EXTMODULE_TIMER               TIM1
  #define EXTMODULE_TIMER_IRQn          TIM1_CC_IRQn
  #define EXTMODULE_TIMER_IRQHandler    TIM1_CC_IRQHandler
  #define EXTMODULE_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define EXTMODULE_DMA_CHANNEL         DMA_Channel_6
  #define EXTMODULE_DMA_STREAM          DMA2_Stream5
  #define EXTMODULE_DMA_IRQn            DMA2_Stream5_IRQn
  #define EXTMODULE_DMA_IRQHandler      DMA2_Stream5_IRQHandler
  #define EXTMODULE_DMA_FLAG_TC         DMA_IT_TCIF5
#else
  #define EXTMODULE_RCC_AHB1Periph      (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA1)
  #define EXTMODULE_RCC_APB1Periph      RCC_APB1Periph_TIM2
  #define EXTMODULE_RCC_APB2Periph      0
  #define EXTMODULE_TX_GPIO             GPIOA
  #define EXTMODULE_TX_GPIO_PIN         GPIO_Pin_15 // PA.15
  #define EXTMODULE_TX_GPIO_PinSource   GPIO_PinSource15
  #define EXTMODULE_TX_GPIO_AF          GPIO_AF_TIM2
  #define EXTMODULE_TIMER               TIM2
  #define EXTMODULE_TIMER_IRQn          TIM2_IRQn
  #define EXTMODULE_TIMER_IRQHandler    TIM2_IRQHandler
  #define EXTMODULE_TIMER_FREQ          (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define EXTMODULE_DMA_CHANNEL         DMA_Channel_3
  #define EXTMODULE_DMA_STREAM          DMA1_Stream7
  #define EXTMODULE_DMA_IRQn            DMA1_Stream7_IRQn
  #define EXTMODULE_DMA_IRQHandler      DMA1_Stream7_IRQHandler
  #define EXTMODULE_DMA_FLAG_TC         DMA_IT_TCIF7
#endif

// Heartbeat (not used)
#define HEARTBEAT_RCC_AHB1Periph        RCC_AHB1Periph_GPIOD
#define HEARTBEAT_GPIO                  GPIOD
#define HEARTBEAT_GPIO_PIN              GPIO_Pin_12 // PD.12

// Trainer Port
#define TRAINERMODULE
#define TRAINER_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOD)
#define TRAINER_RCC_APB1Periph          RCC_APB1Periph_TIM4
#define TRAINER_GPIO                    GPIOD
#define TRAINER_IN_GPIO_PIN             GPIO_Pin_12 // PD.12
#define TRAINER_IN_GPIO_PinSource       GPIO_PinSource12
#define TRAINER_OUT_GPIO_PIN            GPIO_Pin_13 // PD.13
#define TRAINER_OUT_GPIO_PinSource      GPIO_PinSource13
// #define TRAINER_DETECT_GPIO             GPIOB
// #define TRAINER_DETECT_GPIO_PIN         GPIO_Pin_4  // PB.04
#define TRAINER_TIMER                   TIM4
#define TRAINER_TIMER_IRQn              TIM4_IRQn
#define TRAINER_GPIO_AF                 GPIO_AF_TIM4 // TIM4_CH1 (in) + TIM4_CH2 (out)
#define TRAINER_OUT_DMA                 DMA1
// #define TRAINER_OUT_DMA_CHANNEL         DMA_Channel_2
// #define TRAINER_OUT_DMA_STREAM          DMA1_Stream3
// #define TRAINER_OUT_DMA_IRQn            DMA1_Stream3_IRQn
// #define TRAINER_OUT_DMA_IRQHandler      DMA1_Stream3_IRQHandler
// #define TRAINER_OUT_DMA_FLAG_TC         DMA_IT_TCIF3
#define TRAINER_EXTMODULE_TIMER_IRQn    TIM4_IRQn
#define TRAINER_EXTMODULE_TIMER_IRQHandler  TIM4_IRQHandler
#define TRAINER_EXTMODULE_TIMER_FREQ    (PERI1_FREQUENCY * TIMER_MULT_APB1)

// Xms Interrupt
#define INTERRUPT_xMS_RCC_APB1Periph    RCC_APB1Periph_TIM14
#define INTERRUPT_xMS_TIMER             TIM14
#define INTERRUPT_xMS_IRQn              TIM8_TRG_COM_TIM14_IRQn
#define INTERRUPT_xMS_IRQHandler        TIM8_TRG_COM_TIM14_IRQHandler

// 2MHz Timer
#define TIMER_2MHz_RCC_APB1Periph       RCC_APB1Periph_TIM7
#define TIMER_2MHz_TIMER                TIM7

// Bluetooth
#define BT_RCC_APB2Periph               RCC_APB2Periph_USART6
#define BT_USART                        USART6
#define BT_GPIO_AF                      GPIO_AF_USART6
#define BT_USART_IRQn                   USART6_IRQn
#define BT_GPIO_TXRX                    GPIOG
#define BT_TX_GPIO_PIN                  GPIO_Pin_14 // PG.14
#define BT_RX_GPIO_PIN                  GPIO_Pin_9  // PG.09
#define BT_TX_GPIO_PinSource            GPIO_PinSource14
#define BT_RX_GPIO_PinSource            GPIO_PinSource9
#define BT_USART_IRQHandler             USART6_IRQHandler
#if defined(PCBX12S)
  #if PCBREV >= 13
    #define BT_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOG)
    #define BT_EN_GPIO                  GPIOI
    #define BT_EN_GPIO_PIN              GPIO_Pin_10 // PI.10
  #else
    #define BT_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOG)
    #define BT_EN_GPIO                  GPIOA
    #define BT_EN_GPIO_PIN              GPIO_Pin_6 // PA.06
  #endif
  #define BT_BRTS_GPIO                  GPIOG
  #define BT_BRTS_GPIO_PIN              GPIO_Pin_10 // PG.10
  #define BT_BCTS_GPIO                  GPIOG
  #define BT_BCTS_GPIO_PIN              GPIO_Pin_11 // PG.11
#elif defined(PCBX10)
  #define BT_RCC_AHB1Periph             RCC_AHB1Periph_GPIOG
  #define BT_EN_GPIO                    GPIOG
  #define BT_EN_GPIO_PIN                GPIO_Pin_10 // PG.10
#endif

// GPS
#if defined(PCBX12S)
  #define GPS_RCC_AHB1Periph            RCC_AHB1Periph_GPIOA
  #define GPS_RCC_APB1Periph            RCC_APB1Periph_UART4
  #define GPS_USART                     UART4
  #define GPS_GPIO_AF                   GPIO_AF_UART4
  #define GPS_USART_IRQn                UART4_IRQn
  #define GPS_USART_IRQHandler          UART4_IRQHandler
  #define GPS_UART_GPIO                 GPIOA
  #define GPS_TX_GPIO_PIN               GPIO_Pin_0 // PA.00
  #define GPS_RX_GPIO_PIN               GPIO_Pin_1 // PA.01
  #define GPS_TX_GPIO_PinSource         GPIO_PinSource0
  #define GPS_RX_GPIO_PinSource         GPIO_PinSource1
#else
  #define GPS_RCC_AHB1Periph            0
  #define GPS_RCC_APB1Periph            0
#endif

#endif // _HAL_H_
