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
 *  @file    MySQLResultSet.cpp
 *  @brief
 *  @author  Albator
 *  @date    2009-07-22
 *
 */

#include "MySQLResultSet.h"
#include "MySQLException.h"
#include "MySQLConnection.h"
#include <iostream>
#include <sstream>
#include <ace/OS.h>



namespace Trinity
{

namespace SQL
{

MySQLResultSet::MySQLResultSet(MySQLStatement* stmt, MYSQL_RES* res) :
        stmt(stmt),
        res(res),
        row(NULL),
        rowPosition(0),
        numRows(0),
        numFields(0)
{
    numRows = mysql_num_rows(res);
    numFields = mysql_num_fields(res);
}

MySQLResultSet::~MySQLResultSet()
{
    if (res)
        mysql_free_result(res);
}

void MySQLResultSet::checkValidity(uint8 idx) const
{
    if (idx == 0 || idx > numFields)
    {
        throw MySQLException("MysqlResultSet: invalid index");
    }

    if (rowPosition == 0)
    {
        throw MySQLException("MysqlResultSet: invalid row (before first");
    }

    if (rowPosition > numRows)
    {
        throw MySQLException("MysqlResultSet: invalid row (after last");
    }

}

bool MySQLResultSet::getBool(uint8 idx) const
{
    checkValidity(idx);
    if (ACE_OS::atoi(row[idx -1]) > 0)
        return true;

    return false;

}

uint8 MySQLResultSet::getUint8(uint8 idx) const
{
    checkValidity(idx);
    return static_cast<uint8>(ACE_OS::strtol(row[idx -1],NULL,10));
}

uint16 MySQLResultSet::getUint16(uint8 idx) const
{
    checkValidity(idx);
    return static_cast<uint16>(ACE_OS::strtol(row[idx -1],NULL,10));
}

uint32 MySQLResultSet::getUint32(uint8 idx) const
{
    checkValidity(idx);
    return static_cast<uint32>(ACE_OS::strtol(row[idx -1],NULL,10));
}

int8 MySQLResultSet::getInt8(uint8 idx) const
{
    checkValidity(idx);
    return static_cast<int8>(ACE_OS::strtol(row[idx -1],NULL,10));
}

int16 MySQLResultSet::getInt16(uint8 idx) const
{
    checkValidity(idx);
    return static_cast<int16>(ACE_OS::strtol(row[idx -1],NULL,10));
}

int32 MySQLResultSet::getInt32(uint8 idx) const
{
    checkValidity(idx);
    return static_cast<int32>(ACE_OS::strtol(row[idx -1],NULL,10));
}


double MySQLResultSet::getDouble(uint8 idx) const
{
    checkValidity(idx);
    return ACE_OS::strtod(row[idx -1], NULL);
}

float MySQLResultSet::getFloat(uint8 idx) const
{
  uint32 temp;
  float ret;
  temp = this->getUint32(idx);
  memcpy(&ret, &temp,4);
  return ret;

}

uint64 MySQLResultSet::getUint64(uint8 idx) const
{
    checkValidity(idx);
    uint64 value;
    std::istringstream is;
    is.str(row[idx -1 ]);
    is >> value;
    return value;

}

int64 MySQLResultSet::getInt64(uint8 idx) const
{
    checkValidity(idx);
    int64 value;
    std::istringstream is;
    is.str(row[idx -1 ]);
    is >> value;
    return value;

}

std::string MySQLResultSet::getString(uint8 idx) const
{
    checkValidity(idx);
    std::size_t len = mysql_fetch_lengths(res)[idx -1];
    return std::string(row[idx -1], len);

}

bool MySQLResultSet::next()
{
    row = mysql_fetch_row(res);
    ++rowPosition;
    return row != NULL;
}

bool MySQLResultSet::isFirst() const
{
    return rowPosition == 1;

}

uint32 MySQLResultSet::rowsCount() const
{
    return numRows;
}

/*
bool MySQLResultSet::first()
{
	rowPosition = 1;

	return numRows > 0;
}

bool MySQLResultSet::beforeFirst()
{
	rowPosition = 0;
}

bool MySQLResultSet::afterLast()
{
	rowPosition = numRows + 1;
}

*/



}
}
