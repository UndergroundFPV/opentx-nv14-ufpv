---- #########################################################################
---- #                                                                       #
---- # Copyright (C) OpenTX                                                  #
-----#                                                                       #
---- # License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html               #
---- #                                                                       #
---- # This program is free software; you can redistribute it and/or modify  #
---- # it under the terms of the GNU General Public License version 2 as     #
---- # published by the Free Software Foundation.                            #
---- #                                                                       #
---- # This program is distributed in the hope that it will be useful        #
---- # but WITHOUT ANY WARRANTY; without even the implied warranty of        #
---- # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
---- # GNU General Public License for more details.                          #
---- #                                                                       #
---- #########################################################################
local devices = { }
local lineIndex = 0
local lineToggled = 0
local pageOffset = 0
local leftImg = Bitmap.open("mask_left.png")
local rightImg = Bitmap.open("mask_right.png")

local curTouch = {
    touchX,
    touchY
}

local curSlide = {
    startX,
    startY,
    endX,
    endY
}

local function createDevice(id, name)
  local device = {
    id = id,
    name = name,
    timeout = 0
  }
  return device
end

local function getDevice(name)
  for i=1, #devices do
    if devices[i].name == name then
      return devices[i]
    end
  end
  return nil
end

local function parseDeviceInfoMessage(data)
  local id = data[2]
  local name = ""
  local i = 3
  while data[i] ~= 0 do
    name = name .. string.char(data[i])
    i = i + 1
  end
  local device = getDevice(name)
  if device == nil then
    device = createDevice(id, name)
    devices[#devices + 1] = device
  end
  local time = getTime()
  device.timeout = time + 3000 -- 30s
  if lineIndex == 0 then
    lineIndex = 1
  end
end

local devicesRefreshTimeout = 0
local function refreshNext()
  local command, data = crossfireTelemetryPop()
  if command == nil then
    local time = getTime()
    if time > devicesRefreshTimeout then
      devicesRefreshTimeout = time + 100 -- 1s
      crossfireTelemetryPush(0x28, { 0x00, 0xEA })
    end
  elseif command == 0x29 then
    parseDeviceInfoMessage(data)
  end
end

local function selectDevice(step)
  lineIndex = 1 + ((lineIndex + step - 1 + #devices) % #devices)
end

-- Init
local function init()
  lineIndex = 0
  pageOffset = 0
end


local function onTouchValid(startX, startY, endX, endY)

  if (startX < curTouch.touchX and curTouch.touchX < endX) and (startY < curTouch.touchY and curTouch.touchY < endY) then
    return 1
  end

  return 0
end

NoCross = { 30, 375, "Waiting for Crossfire devices...", TEXT_COLOR + INVERS + BLINK }

-- Main
local function run(...)
  local attr = 0
  local args = {...}
  local event = args[1]


  if ... == nil then
    error("Cannot be run as a model script!")
    return 2

  elseif event == EVT_TOUCH_UP  then
    curTouch.touchX = args[2]
    curTouch.touchY = args[3] 

    if onTouchValid(5, 188, 110, 210) == 1 then
      if lineIndex == 1 then
        lineToggled = 1
      else
        lineIndex = 1
        curTouch.touchX = 0
        curTouch.touchY = 0
      end
    elseif onTouchValid(5, 230, 110, 250) == 1 then
      if lineIndex == 2 then
        lineToggled = 1
      else
        lineIndex = 2
        curTouch.touchX = 0
        curTouch.touchY = 0
      end
    end

  elseif event == EVT_TOUCH_SLIDE then
    curSlide.startX = args[2]
    curSlide.startY = args[3] 
    curSlide.endX = args[4] 
    curSlide.endY = args[5] 
  end


  local touchField = {
    startX,
    startY,
    endX,
    endY
  }


  lcd.runMainWindow()

  --lcd.clear()

  lcd.drawText(70, 0,"CROSSFIRE SETUP", TEXT_COLOR)
  --lcd.drawBitmap(leftImg, 80, 80)
  --lcd.drawBitmap(rightImg, 100, 80)

  if #devices == 0 then
    lcd.drawText(NoCross[1],NoCross[2],NoCross[3],NoCross[4])
  else
    for i=1, #devices do
      local attr = (lineIndex == i and INVERS or 0)
        if lineToggled == 1 and attr == INVERS then
            attr =  attr + BLINK + TEXT_COLOR
            crossfireTelemetryPush(0x28, { devices[i].id, 0xEA })
            return "device.lua"
        else
            
            attr =  attr + TEXT_COLOR
        end

      lcd.drawText(5, i*42+60, devices[i].name, attr) 
    end
  end

  refreshNext()

  return 0
end

return { init=init, run=run }
