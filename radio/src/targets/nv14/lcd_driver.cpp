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

#define HBP  24 // TODO use names from FlySky
#define VBP  10

#define HSW  4
#define VSH  2

#define HFP  (140 - HBP)
#define VFP  (22 - VBP)

#define LCD_FIRST_LAYER                0
#define LCD_SECOND_LAYER               1

uint8_t LCD_FIRST_FRAME_BUFFER[DISPLAY_BUFFER_SIZE] __SDRAM;
uint8_t LCD_SECOND_FRAME_BUFFER[DISPLAY_BUFFER_SIZE] __SDRAM;
uint8_t LCD_BACKUP_FRAME_BUFFER[DISPLAY_BUFFER_SIZE] __SDRAM;

uint32_t CurrentLayer = LCD_FIRST_LAYER;

#define LCD_NRST_LOW()  LCD_NRST_GPIO->BSRRH = LCD_NRST_GPIO_PIN
#define LCD_NRST_HIGH() LCD_NRST_GPIO->BSRRL = LCD_NRST_GPIO_PIN

#define LCD_CS_HIGH()   LCD_SPI_GPIO->BSRRL = LCD_SPI_CS_GPIO_PIN
#define LCD_CS_LOW()    LCD_SPI_GPIO->BSRRH = LCD_SPI_CS_GPIO_PIN

#define LCD_SCK_HIGH()  LCD_SPI_GPIO->BSRRL = LCD_SPI_SCK_GPIO_PIN
#define LCD_SCK_LOW()   LCD_SPI_GPIO->BSRRH = LCD_SPI_SCK_GPIO_PIN

#define LCD_MOSI_HIGH() LCD_SPI_GPIO->BSRRL = LCD_SPI_MOSI_GPIO_PIN
#define LCD_MOSI_LOW()  LCD_SPI_GPIO->BSRRH = LCD_SPI_MOSI_GPIO_PIN


static void LCD_AF_GPIOConfig(void)
{
  /*
  -----------------------------------------------------------------------------
  LCD_CLK <-> PG.07 | LCD_HSYNC <-> PI.12 | LCD_R3 <-> PJ.02 | LCD_G5 <-> PK.00
                    | LCD VSYNC <-> PI.13 | LCD_R4 <-> PJ.03 | LCD_G6 <-> PK.01
                    |                     | LCD_R5 <-> PJ.04 | LCD_G7 <-> PK.02
                    |                     | LCD_R6 <-> PJ.05 | LCD_B4 <-> PK.03
                    |                     | LCD_R7 <-> PJ.06 | LCD_B5 <-> PK.04
                    |                     | LCD_G2 <-> PJ.09 | LCD_B6 <-> PK.05
                    |                     | LCD_G3 <-> PJ.10 | LCD_B7 <-> PK.06
                    |                     | LCD_G4 <-> PJ.11 | LCD_DE <-> PK.07
                    |                     | LCD_B3 <-> PJ.15 |
  */

  // GPIOG configuration
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource7, GPIO_AF_LTDC);
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOG, &GPIO_InitStructure);

  // GPIOI configuration
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource12, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource13, GPIO_AF_LTDC);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
  GPIO_Init(GPIOI, &GPIO_InitStructure);

  // GPIOJ configuration
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource2, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource3, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource4, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource5, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource6, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource9, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource10, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource11, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource15, GPIO_AF_LTDC);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_15;
  GPIO_Init(GPIOJ, &GPIO_InitStructure);

  // GPIOK configuration
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource0, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource1, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource2, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource3, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource4, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource5, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource6, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource7, GPIO_AF_LTDC);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_Init(GPIOK, &GPIO_InitStructure);
}

static void lcdSpiConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = LCD_SPI_SCK_GPIO_PIN | LCD_SPI_MOSI_GPIO_PIN | LCD_SPI_CS_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_Init(LCD_SPI_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_NRST_GPIO_PIN;
  GPIO_Init(LCD_NRST_GPIO, &GPIO_InitStructure);

  LCD_CS_LOW();
}

void lcdDelay()
{
  delay_01us(1);
}

static void lcdWriteByte(uint8_t data_enable, uint8_t byte)
{
  LCD_SCK_LOW();
  lcdDelay();
  if (data_enable)
    LCD_MOSI_HIGH();
  else
    LCD_MOSI_LOW();
  LCD_SCK_HIGH();
  lcdDelay();

  for (int i = 0; i < 8; i++) {
    LCD_SCK_LOW();
    lcdDelay();
    if (byte & 0x80)
      LCD_MOSI_HIGH();
    else
      LCD_MOSI_LOW();
    LCD_SCK_HIGH();
    byte <<= 1;
    lcdDelay();
  }
  LCD_SCK_LOW();
}

void lcdWriteCommand(uint8_t command)
{
  lcdWriteByte(0, command);
}

void lcdWriteData(uint8_t data)
{
  lcdWriteByte(1, data);
}

static void lcdSpiInit()
{
  lcdWriteCommand(0x11);
  delay_ms( 120 );
  lcdWriteCommand(0x13);

  lcdWriteCommand(0xf0);
  lcdWriteData(0xc3);
  lcdWriteCommand(0xf0);
  lcdWriteData(0x96);

  lcdWriteCommand(0x36);

// #if defined( LCD_DIRECTION ) && ( LCD_DIRECTION == LCD_VERTICAL )
  lcdWriteData(0x08);
// #else
  //lcdWriteData(0xf8);
  //lcdWriteData(0xcc);
//#endif

  lcdWriteCommand(0x3A);
  lcdWriteData(0x65);

  lcdWriteCommand(0xB4);
  lcdWriteData(0x01);

  lcdWriteCommand(0xb5);
  lcdWriteData(VFP);
  lcdWriteData(VBP + VSH);
  lcdWriteData(0x00);
  lcdWriteData(HBP + 4);

  lcdWriteCommand(0xb6);
  lcdWriteData(0xe0);
  lcdWriteData(0x42);
  lcdWriteData(0x3b);

  lcdWriteCommand(0xB7);
  lcdWriteData(0x66);

  lcdWriteCommand(0xe8);
  lcdWriteData(0x40);
  lcdWriteData(0x8a);
  lcdWriteData(0x00);
  lcdWriteData(0x00);
  lcdWriteData(0x29);
  lcdWriteData(0x19);
  lcdWriteData(0xa5);
  lcdWriteData(0x33);

  lcdWriteCommand(0xc1);
  lcdWriteData(0x06);

  lcdWriteCommand(0xc2);
  lcdWriteData(0xa7);

  lcdWriteCommand(0xc5);
  lcdWriteData(0x18);

  lcdWriteCommand(0xe0); //Positive Voltage Gamma Control
  lcdWriteData(0xf0);
  lcdWriteData(0x09);
  lcdWriteData(0x0b);
  lcdWriteData(0x06);
  lcdWriteData(0x04);
  lcdWriteData(0x15);
  lcdWriteData(0x2f);
  lcdWriteData(0x54);
  lcdWriteData(0x42);
  lcdWriteData(0x3c);
  lcdWriteData(0x17);
  lcdWriteData(0x14);
  lcdWriteData(0x18);
  lcdWriteData(0x1b);

  lcdWriteCommand(0xe1); //Negative Voltage Gamma Control
  lcdWriteData(0xf0);
  lcdWriteData(0x09);
  lcdWriteData(0x0b);
  lcdWriteData(0x06);
  lcdWriteData(0x04);
  lcdWriteData(0x03);
  lcdWriteData(0x2d);
  lcdWriteData(0x43);
  lcdWriteData(0x42);
  lcdWriteData(0x3b);
  lcdWriteData(0x16);
  lcdWriteData(0x14);
  lcdWriteData(0x17);
  lcdWriteData(0x1b);

  lcdWriteCommand(0xf0);
  lcdWriteData(0x3c);
  lcdWriteCommand(0xf0);
  lcdWriteData(0x69);

  delay_ms(5);
  lcdWriteCommand(0x28);
  lcdWriteCommand(0x2C);

  delay_ms(50);
  lcdWriteCommand(0x29); // LCD ON
}

static void lcdReset()
{
  LCD_NRST_HIGH();
  delay_ms(1);

  LCD_NRST_LOW(); // RESET();
  delay_ms(100);

  LCD_NRST_HIGH();
  delay_ms(100);
}

void LCD_Init_LTDC()
{
  LTDC_InitTypeDef LTDC_InitStruct;

  /* Configure PLLSAI prescalers for LCD */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAI_N = 192 Mhz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLL_LTDC = 192/3 = 64 Mhz */
  /* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = 64/4 = 16 Mhz */
  RCC_PLLSAIConfig(192 * 2 / 3, 6, 3);
  RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div4);

  /* Enable PLLSAI Clock */
  RCC_PLLSAICmd(ENABLE);

  /* Wait for PLLSAI activation */
  while (RCC_GetFlagStatus(RCC_FLAG_PLLSAIRDY) == RESET);

  /* LTDC Configuration *********************************************************/
  /* Polarity configuration */
  /* Initialize the horizontal synchronization polarity as active low */
  LTDC_InitStruct.LTDC_HSPolarity = LTDC_HSPolarity_AL;
  /* Initialize the vertical synchronization polarity as active low */
  LTDC_InitStruct.LTDC_VSPolarity = LTDC_VSPolarity_AL;
  /* Initialize the data enable polarity as active low */
  LTDC_InitStruct.LTDC_DEPolarity = LTDC_DEPolarity_AL;
  /* Initialize the pixel clock polarity as input pixel clock */
  LTDC_InitStruct.LTDC_PCPolarity = LTDC_PCPolarity_IPC;

  /* Configure R,G,B component values for LCD background color */
  LTDC_InitStruct.LTDC_BackgroundRedValue = 0;
  LTDC_InitStruct.LTDC_BackgroundGreenValue = 0;
  LTDC_InitStruct.LTDC_BackgroundBlueValue = 0;

  /* Configure horizontal synchronization width */
  LTDC_InitStruct.LTDC_HorizontalSync = HSW;
  /* Configure vertical synchronization height */
  LTDC_InitStruct.LTDC_VerticalSync = VSH;
  /* Configure accumulated horizontal back porch */
  LTDC_InitStruct.LTDC_AccumulatedHBP = HBP;
  /* Configure accumulated vertical back porch */
  LTDC_InitStruct.LTDC_AccumulatedVBP = VBP;
  /* Configure accumulated active width */
  LTDC_InitStruct.LTDC_AccumulatedActiveW = LCD_W + HBP;
  /* Configure accumulated active height */
  LTDC_InitStruct.LTDC_AccumulatedActiveH = LCD_H + VBP;
  /* Configure total width */
  LTDC_InitStruct.LTDC_TotalWidth = LCD_W + HBP + HFP;
  /* Configure total height */
  LTDC_InitStruct.LTDC_TotalHeigh = LCD_H + VBP + VFP;

  LTDC_Init(&LTDC_InitStruct);

#if 0
  LTDC_ITConfig(LTDC_IER_LIE, ENABLE);
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = LTDC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = LTDC_IRQ_PRIO;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; /* Not used as 4 bits are used for the pr     e-emption priority. */;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init( &NVIC_InitStructure );

  DMA2D_ITConfig(DMA2D_CR_TCIE, ENABLE);
  NVIC_InitStructure.NVIC_IRQChannel = DMA2D_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA_SCREEN_IRQ_PRIO;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; /* Not used as 4 bits are used for the pr     e-emption priority. */;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init( &NVIC_InitStructure );

  DMA2D->IFCR = (unsigned long)DMA2D_IFSR_CTCIF;
#endif
}

void LCD_LayerInit()
{
  LTDC_Layer_InitTypeDef LTDC_Layer_InitStruct;

  /* Windowing configuration */
  /* In this case all the active display area is used to display a picture then :
  Horizontal start = horizontal synchronization + Horizontal back porch = 30
  Horizontal stop = Horizontal start + window width -1 = 30 + 240 -1
  Vertical start   = vertical synchronization + vertical back porch     = 4
  Vertical stop   = Vertical start + window height -1  = 4 + 320 -1      */
  LTDC_Layer_InitStruct.LTDC_HorizontalStart = HBP + 1;
  LTDC_Layer_InitStruct.LTDC_HorizontalStop = (LCD_W + HBP);
  LTDC_Layer_InitStruct.LTDC_VerticalStart = VBP + 1;;
  LTDC_Layer_InitStruct.LTDC_VerticalStop = (LCD_H + VBP);

  /* Pixel Format configuration*/
  LTDC_Layer_InitStruct.LTDC_PixelFormat = LTDC_Pixelformat_RGB565;
  /* Alpha constant (255 totally opaque) */
  LTDC_Layer_InitStruct.LTDC_ConstantAlpha = 255;
  /* Default Color configuration (configure A,R,G,B component values) */
  LTDC_Layer_InitStruct.LTDC_DefaultColorBlue = 0;
  LTDC_Layer_InitStruct.LTDC_DefaultColorGreen = 0;
  LTDC_Layer_InitStruct.LTDC_DefaultColorRed = 0;
  LTDC_Layer_InitStruct.LTDC_DefaultColorAlpha = 0;

  /* Configure blending factors */
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_CA;
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_CA;

  /* the length of one line of pixels in bytes + 3 then :
  Line Lenth = Active high width x number of bytes per pixel + 3
  Active high width         = LCD_W
  number of bytes per pixel = 2    (pixel_format : RGB565)
  */
  LTDC_Layer_InitStruct.LTDC_CFBLineLength = ((LCD_W * 2) + 3);
  /* the pitch is the increment from the start of one line of pixels to the
  start of the next line in bytes, then :
  Pitch = Active high width x number of bytes per pixel */
  LTDC_Layer_InitStruct.LTDC_CFBPitch = (LCD_W * 2);

  /* Configure the number of lines */
  LTDC_Layer_InitStruct.LTDC_CFBLineNumber = LCD_H;

  /* Start Address configuration : the LCD Frame buffer is defined on SDRAM w/ Offset */
  LTDC_Layer_InitStruct.LTDC_CFBStartAdress = (uint32_t)LCD_FIRST_FRAME_BUFFER;

  /* Initialize LTDC layer 1 */
  LTDC_LayerInit(LTDC_Layer1, &LTDC_Layer_InitStruct);

  /* Configure Layer 2 */
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_PAxCA;
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_PAxCA;

  /* Start Address configuration : the LCD Frame buffer is defined on SDRAM w/ Offset */
  LTDC_Layer_InitStruct.LTDC_CFBStartAdress = (uint32_t)LCD_SECOND_FRAME_BUFFER;

  /* Initialize LTDC layer 2 */
  LTDC_LayerInit(LTDC_Layer2, &LTDC_Layer_InitStruct);

  /* LTDC configuration reload */
  LTDC_ReloadConfig(LTDC_IMReload);

  LTDC_LayerCmd(LTDC_Layer1, ENABLE);
  LTDC_LayerCmd(LTDC_Layer2, ENABLE);

  LTDC_ReloadConfig(LTDC_IMReload);

  /* dithering activation */
  LTDC_DitherCmd(ENABLE);
}

BitmapBuffer lcdBuffer1(BMP_RGB565, LCD_W, LCD_H, (uint16_t *)LCD_FIRST_FRAME_BUFFER);
BitmapBuffer lcdBuffer2(BMP_RGB565, LCD_W, LCD_H, (uint16_t *)LCD_SECOND_FRAME_BUFFER);
BitmapBuffer * lcd = &lcdBuffer1;

void LCD_SetLayer(uint32_t layer)
{
  if (layer == LCD_FIRST_LAYER) {
    lcd = &lcdBuffer1;
  }
  else {
    lcd = &lcdBuffer2;
  }
  CurrentLayer = layer;
}

void LCD_SetTransparency(uint8_t transparency)
{
  if (CurrentLayer == LCD_FIRST_LAYER) {
    LTDC_LayerAlpha(LTDC_Layer1, transparency);
  }
  else {
    LTDC_LayerAlpha(LTDC_Layer2, transparency);
  }
  LTDC_ReloadConfig(LTDC_IMReload);
}

void lcdInit(void)
{
  /* Configure the LCD SPI+RESET pins */
  lcdSpiConfig();

  /* Reset the LCD --------------------------------------------------------*/
  lcdReset();

  /* Configure the LCD Control pins */
  LCD_AF_GPIOConfig();

  lcdSpiInit();

  LCD_Init_LTDC();

  LCD_LayerInit();

  /* Enable LCD display */
  LTDC_Cmd(ENABLE);

  /* Set Background layer */
  LCD_SetLayer(LCD_FIRST_LAYER);
  // lcdClear();
  LCD_SetTransparency(0);

  /* Set Foreground layer */
  LCD_SetLayer(LCD_SECOND_LAYER);
  lcd->clear();
  LCD_SetTransparency(255);
}

void DMAFillRect(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
  DMA2D_DeInit();

  DMA2D_InitTypeDef DMA2D_InitStruct;
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_R2M;
  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;
  DMA2D_InitStruct.DMA2D_OutputGreen = (0x07E0 & color) >> 5;
  DMA2D_InitStruct.DMA2D_OutputBlue = 0x001F & color;
  DMA2D_InitStruct.DMA2D_OutputRed = (0xF800 & color) >> 11;
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0x0F;
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = CONVERT_PTR_UINT(dest) + 2*(destw*y + x);
  DMA2D_InitStruct.DMA2D_OutputOffset = (destw - w);
  DMA2D_InitStruct.DMA2D_NumberOfLine = h;
  DMA2D_InitStruct.DMA2D_PixelPerLine = w;
  DMA2D_Init(&DMA2D_InitStruct);

  /* Start Transfer */
  DMA2D_StartTransfer();

  /* Wait for CTC Flag activation */
  while (DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET);
}

void DMACopyBitmap(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, const uint16_t * src, uint16_t srcw, uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w, uint16_t h)
{
  DMA2D_DeInit();

  DMA2D_InitTypeDef DMA2D_InitStruct;
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_M2M;
  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = CONVERT_PTR_UINT(dest + y*destw + x);
  DMA2D_InitStruct.DMA2D_OutputGreen = 0;
  DMA2D_InitStruct.DMA2D_OutputBlue = 0;
  DMA2D_InitStruct.DMA2D_OutputRed = 0;
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0;
  DMA2D_InitStruct.DMA2D_OutputOffset = destw - w;
  DMA2D_InitStruct.DMA2D_NumberOfLine = h;
  DMA2D_InitStruct.DMA2D_PixelPerLine = w;
  DMA2D_Init(&DMA2D_InitStruct);

  DMA2D_FG_InitTypeDef DMA2D_FG_InitStruct;
  DMA2D_FG_StructInit(&DMA2D_FG_InitStruct);
  DMA2D_FG_InitStruct.DMA2D_FGMA = CONVERT_PTR_UINT(src + srcy*srcw + srcx);
  DMA2D_FG_InitStruct.DMA2D_FGO = srcw - w;
  DMA2D_FG_InitStruct.DMA2D_FGCM = CM_RGB565;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_VALUE = 0;
  DMA2D_FGConfig(&DMA2D_FG_InitStruct);

  /* Start Transfer */
  DMA2D_StartTransfer();

  /* Wait for CTC Flag activation */
  while (DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET);
}

void DMACopyAlphaBitmap(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, const uint16_t * src, uint16_t srcw, uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w, uint16_t h)
{
  DMA2D_DeInit();

  DMA2D_InitTypeDef DMA2D_InitStruct;
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_M2M_BLEND;
  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = CONVERT_PTR_UINT(dest + y*destw + x);
  DMA2D_InitStruct.DMA2D_OutputGreen = 0;
  DMA2D_InitStruct.DMA2D_OutputBlue = 0;
  DMA2D_InitStruct.DMA2D_OutputRed = 0;
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0;
  DMA2D_InitStruct.DMA2D_OutputOffset = destw - w;
  DMA2D_InitStruct.DMA2D_NumberOfLine = h;
  DMA2D_InitStruct.DMA2D_PixelPerLine = w;
  DMA2D_Init(&DMA2D_InitStruct);

  DMA2D_FG_InitTypeDef DMA2D_FG_InitStruct;
  DMA2D_FG_StructInit(&DMA2D_FG_InitStruct);
  DMA2D_FG_InitStruct.DMA2D_FGMA = CONVERT_PTR_UINT(src + srcy*srcw + srcx);
  DMA2D_FG_InitStruct.DMA2D_FGO = srcw - w;
  DMA2D_FG_InitStruct.DMA2D_FGCM = CM_ARGB4444;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_VALUE = 0;
  DMA2D_FGConfig(&DMA2D_FG_InitStruct);

  DMA2D_BG_InitTypeDef DMA2D_BG_InitStruct;
  DMA2D_BG_StructInit(&DMA2D_BG_InitStruct);
  DMA2D_BG_InitStruct.DMA2D_BGMA = CONVERT_PTR_UINT(dest + y*destw + x);
  DMA2D_BG_InitStruct.DMA2D_BGO = destw - w;
  DMA2D_BG_InitStruct.DMA2D_BGCM = CM_RGB565;
  DMA2D_BG_InitStruct.DMA2D_BGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE;
  DMA2D_BG_InitStruct.DMA2D_BGPFC_ALPHA_VALUE = 0;
  DMA2D_BGConfig(&DMA2D_BG_InitStruct);

  /* Start Transfer */
  DMA2D_StartTransfer();

  /* Wait for CTC Flag activation */
  while (DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET);
}

void DMABitmapConvert(uint16_t * dest, const uint8_t * src, uint16_t w, uint16_t h, uint32_t format)
{
  DMA2D_DeInit();

  DMA2D_InitTypeDef DMA2D_InitStruct;
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_M2M_PFC;
  DMA2D_InitStruct.DMA2D_CMode = format;
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = CONVERT_PTR_UINT(dest);
  DMA2D_InitStruct.DMA2D_OutputGreen = 0;
  DMA2D_InitStruct.DMA2D_OutputBlue = 0;
  DMA2D_InitStruct.DMA2D_OutputRed = 0;
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0;
  DMA2D_InitStruct.DMA2D_OutputOffset = 0;
  DMA2D_InitStruct.DMA2D_NumberOfLine = h;
  DMA2D_InitStruct.DMA2D_PixelPerLine = w;
  DMA2D_Init(&DMA2D_InitStruct);

  DMA2D_FG_InitTypeDef DMA2D_FG_InitStruct;
  DMA2D_FG_StructInit(&DMA2D_FG_InitStruct);
  DMA2D_FG_InitStruct.DMA2D_FGMA = CONVERT_PTR_UINT(src);
  DMA2D_FG_InitStruct.DMA2D_FGO = 0;
  DMA2D_FG_InitStruct.DMA2D_FGCM = CM_ARGB8888;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_MODE = REPLACE_ALPHA_VALUE;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_VALUE = 0;
  DMA2D_FGConfig(&DMA2D_FG_InitStruct);

  /* Start Transfer */
  DMA2D_StartTransfer();

  /* Wait for CTC Flag activation */
  while (DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET);
}

void DMAcopy(void * src, void * dest, int len)
{
  DMA2D_DeInit();

  DMA2D_InitTypeDef DMA2D_InitStruct;
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_M2M;
  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = CONVERT_PTR_UINT(dest);
  DMA2D_InitStruct.DMA2D_OutputGreen = 0;
  DMA2D_InitStruct.DMA2D_OutputBlue = 0;
  DMA2D_InitStruct.DMA2D_OutputRed = 0;
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0;
  DMA2D_InitStruct.DMA2D_OutputOffset = 0;
  DMA2D_InitStruct.DMA2D_NumberOfLine = LCD_H;
  DMA2D_InitStruct.DMA2D_PixelPerLine = LCD_W;
  DMA2D_Init(&DMA2D_InitStruct);

  DMA2D_FG_InitTypeDef DMA2D_FG_InitStruct;
  DMA2D_FG_StructInit(&DMA2D_FG_InitStruct);
  DMA2D_FG_InitStruct.DMA2D_FGMA = CONVERT_PTR_UINT(src);
  DMA2D_FG_InitStruct.DMA2D_FGO = 0;
  DMA2D_FG_InitStruct.DMA2D_FGCM = CM_RGB565;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_VALUE = 0;
  DMA2D_FGConfig(&DMA2D_FG_InitStruct);

  /* Start Transfer */
  DMA2D_StartTransfer();

  /* Wait for CTC Flag activation */
  while (DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET);
}

void lcdStoreBackupBuffer()
{
  DMAcopy(lcd->getData(), LCD_BACKUP_FRAME_BUFFER, DISPLAY_BUFFER_SIZE);
}

int lcdRestoreBackupBuffer()
{
  DMAcopy(LCD_BACKUP_FRAME_BUFFER, lcd->getData(), DISPLAY_BUFFER_SIZE);
  return 1;
}

void lcdRefresh()
{
  if (CurrentLayer == LCD_FIRST_LAYER) {
    LTDC_LayerAlpha(LTDC_Layer1, 255);
    LTDC_LayerAlpha(LTDC_Layer2, 0);
    LCD_SetLayer(LCD_SECOND_LAYER);
  }
  else {
    LTDC_LayerAlpha(LTDC_Layer2, 255);
    LTDC_LayerAlpha(LTDC_Layer1, 0);
    LCD_SetLayer(LCD_FIRST_LAYER);
  }
  LTDC_ReloadConfig(LTDC_IMReload);
}
