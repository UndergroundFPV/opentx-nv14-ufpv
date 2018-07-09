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

#ifndef BITFIELD_H
#define BITFIELD_H

// A set of bitfield handling macros
#define BF_BIT(n)                  ( 1<<(n) )
#define BF_BIT_SET(y, mask)        ( y |=  (mask) )
#define BF_BIT_CLEAR(y, mask)      ( y &= ~(mask) )
#define BF_BIT_FLIP(y, mask)       ( y ^=  (mask) )

//! Create a bitmask of length \a len.
#define BF_BITMASK(len)           ( BF_BIT(len)-1 )

//! Create a bitfield mask of length \a starting at bit \a start.
#define BF_MASK(start, len)     ( BF_BITMASK(len)<<(start) )

//! Prepare a bitmask for insertion or combining.
#define BF_PREP(x, start, len)  ( ((x)&BF_BITMASK(len)) << (start) )

//! Extract a bitfield of length \a len starting at bit \a start from \a y.
#define BF_GET(y, start, len)   ( ((y)>>(start)) & BF_BITMASK(len) )

//! Insert a new bitfield value \a x into \a y.
#define BF_SET(y, x, start, len)    \
    ( y= ((y) &~ BF_MASK(start, len)) | BF_PREP(x, start, len) )

#endif //BITFIELD_H
