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

#ifndef _WINDOWS_HELPERS_H_
#define _WINDOWS_HELPERS_H_

#define GET_DEFAULT(value)      [=]() -> int32_t { return value; }
#define GET_INVERTED(value)     [=]() -> uint8_t { return !value; }

#define SET_VALUE(value, _newValue) [=](int32_t newValue) -> void { value = _newValue; SET_DIRTY(); }
#define SET_DEFAULT(value)          [=](int32_t newValue) -> void { value = newValue; SET_DIRTY(); }
#define SET_INVERTED(value)         [=](uint8_t newValue) -> void { value = !newValue; }

#define GET_SET_DEFAULT(value)  GET_DEFAULT(value), SET_DEFAULT(value)
#define GET_SET_INVERTED(value) GET_INVERTED(value), SET_INVERTED(value)

#endif // _WINDOWS_HELPERS_H_

