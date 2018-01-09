option(SHUTDOWN_CONFIRMATION "Shutdown confirmation" OFF)
option(LCD_DUAL_BUFFER "Dual LCD Buffer" OFF)
option(TARANIS_INTERNAL_PPM "Taranis internal module hack to output PPM" OFF)

set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
set(CPU_TYPE STM32F4)
set(CPU_TYPE_FULL STM32F405)  # for size report
set(LINKER_SCRIPT targets/i8/stm32f4_flash.ld)
set(HAPTIC YES)
set(LUA_EXPORT lua_export_i8)
set(FLAVOUR i8)
add_definitions(-DSTM32F40_41xxx -DPCBI8 -DSOFTWARE_VOLUME)
add_definitions(-DEEPROM_VARIANT=0x4000)
add_definitions(-DPWR_BUTTON_${PWR_BUTTON})
set(GUI_DIR 128x64)
set(BITMAPS_TARGET 9x_bitmaps)
set(FONTS_TARGET 9x_fonts_1bit)
set(LCD_DRIVER lcd_driver.cpp)
set(DISK_CACHE NO)
set(DBLKEYS OFF)

set(HSE_VALUE 12000000)
set(SDCARD YES)
set(EEPROM EEPROM_RLC)
set(TARGET_DIR i8)
set(PPM_LIMITS_SYMETRICAL YES)

if(SPLASH STREQUAL OFF)
  message(FATAL_ERROR "Option SPLASH=OFF not supported on i8")
endif()

add_definitions(-DPCBI8 -DPPM_PIN_TIMER)
add_definitions(-DAUDIO -DVOICE -DRTCLOCK -DSOFTWARE_VOLUME)

set(GUI_SRC ${GUI_SRC}
  model_input_edit.cpp
  model_mix_edit.cpp
  model_display.cpp
  radio_diagkeys.cpp
  radio_diaganas.cpp
  radio_hardware.cpp
  view_channels.cpp
  view_telemetry.cpp
  view_about.cpp
  bmp.cpp
  ../screenshot.cpp
  )

set(LED_DRIVER led_driver.cpp)

set(TARGET_SRC
  ${TARGET_SRC}
  board.cpp
  ${LED_DRIVER}
  backlight_driver.cpp
  extmodule_driver.cpp
  trainer_driver.cpp
  gimbals_driver.cpp
  ../common/arm/stm32/intmodule_serial_driver.cpp
  ../common/arm/stm32/audio_dac_driver.cpp
  ../common/arm/stm32/adc_driver.cpp
  )

set(FIRMWARE_TARGET_SRC
  ${FIRMWARE_TARGET_SRC}
  startup_stm32f40_41xxx.s
  ../common/arm/stm32/sdio_sd.c
  ${LCD_DRIVER}
  ../common/arm/stm32/i2c_driver.cpp
  ../common/arm/stm32/eeprom_driver.cpp
  pwr_driver.cpp
  ../common/arm/loadboot.cpp
  )

set(STM32LIB_SRC
  STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.c
  )

if(LCD_DUAL_BUFFER)
  add_definitions(-DLCD_DUAL_BUFFER)
endif()

set(SERIAL2_DRIVER ../common/arm/stm32/serial2_driver.cpp)

set(FIRMWARE_DEPENDENCIES ${FIRMWARE_DEPENDENCIES} ${BITMAPS_TARGET})
