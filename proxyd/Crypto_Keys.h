/* -*- C++ -*-
 * Copyright (C) 2009 Trinity Core <http://www.trinitycore.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 *  @file
 *  @brief   Contains encryption keys used by different client versions.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-14
 *
 */

#include "Common.h"

namespace Trinity
{
namespace Proxyd
{
#define SEED_KEY_SIZE 16

static uint8 tbc_encryption_key[SEED_KEY_SIZE] =   { 0x38, 0xA7, 0x83, 0x15, 0xF8, 0x92, 0x25, 0x30, 
						     0x71, 0x98, 0x67, 0xB1, 0x8C, 0x04, 0xE2, 0xAA };

static uint8 wlk_encryption_key[SEED_KEY_SIZE] = { 0x22, 0xBE, 0xE5, 0xCF, 0xBB, 0x07, 0x64, 0xD9, 
						   0x00, 0x45, 0x1B, 0xD0, 0x24, 0xB8, 0xD5, 0x45 };

static uint8 wlk_decryption_key[SEED_KEY_SIZE] =  { 0xF4, 0x66, 0x31, 0x59, 0xFC, 0x83, 0x6E, 0x31, 
						    0x31, 0x02, 0x51, 0xD5, 0x44, 0x31, 0x67, 0x98 };

};
};
