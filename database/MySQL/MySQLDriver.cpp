/*
 * Copyright (C) 2009 Dawn Of Reckoning
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

/*
 *  @file    MySQLDriver.cpp
 *  @brief
 *  @author  Albator
 *  @date    2009-07-22
 *
 */

#include "MySQLDriver.h"
#include "MySQLConnection.h"



namespace Trinity
{
namespace SQL
{

const uint32 MySQLDriver::majorVersion = 1;
const uint32 MySQLDriver::minorVersion = 0;

Connection* MySQLDriver::connect(const std::string& url)
{
    return new MySQLConnection(url);
}

const uint32 MySQLDriver::getMajorVersion() const
{
    return majorVersion;
}


const uint32 MySQLDriver::getMinorVersion() const
{
    return minorVersion;
}


}
}


