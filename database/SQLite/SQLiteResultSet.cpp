/*
 * Copyright (C) 2009 Dawn Of Reckoning
 * Copyright (C) 2012 Morpheus
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
 *  @file    SQLiteResultSet.cpp
 *  @brief
 *  @author  Albator
 *  @date    2009-07-22
 *
 */

#include "SQLiteResultSet.h"
#include "SQLiteStatement.h"
#include "SQLitePreparedStatement.h"
#include "SQLiteException.h"
#include <iostream>
#include <sstream>
#include <ace/OS.h>

namespace Morpheus
{

namespace SQL
{

SQLiteResultSet::SQLiteResultSet(SQLiteStatement* stmt) :
        stmt(stmt),
        pStmt(NULL),
        rowPosition(0),
        numRows(0),
        numFields(0)
{
    sqliteStmt = stmt->getSQLiteStatement();
    sqlite = stmt->getSQLiteHandle();
    numFields = static_cast<uint32>(sqlite3_column_count(sqliteStmt));
}

SQLiteResultSet::SQLiteResultSet(SQLitePreparedStatement* stmt) :
        stmt(NULL),
        pStmt(stmt),
        rowPosition(0),
        numRows(0),
        numFields(0)
{
    sqliteStmt = pStmt->getSQLiteStatement();
    sqlite = pStmt->getSQLiteHandle();
    numFields = static_cast<uint32>(sqlite3_column_count(sqliteStmt));

}

SQLiteResultSet::~SQLiteResultSet()
{
}

void SQLiteResultSet::checkValidity(uint8 idx) const
{
    if (idx == 0 || idx > numFields)
        throw SQLiteException("SQLiteResultSet: invalid index",sqlite);

    if (rowPosition == 0)
        throw SQLiteException("SQLiteResultSet: invalid row position (before first)",sqlite);


}

bool SQLiteResultSet::getBool(uint8 idx) const
{
    if (getInt32(idx) > 0)
        return true;

    return false;
}

uint8 SQLiteResultSet::getUint8(uint8 idx) const
{
    return static_cast<uint8>(getInt32(idx));
}

uint16 SQLiteResultSet::getUint16(uint8 idx) const
{
    return static_cast<uint16>(getInt32(idx));
}

uint32 SQLiteResultSet::getUint32(uint8 idx) const
{
    return static_cast<uint32>(getInt32(idx));
}

int8 SQLiteResultSet::getInt8(uint8 idx) const
{
    return static_cast<int8>(getInt32(idx));
}

int16 SQLiteResultSet::getInt16(uint8 idx) const
{
    return static_cast<int16>(getInt32(idx));
}

int32 SQLiteResultSet::getInt32(uint8 idx) const
{
    checkValidity(idx);
    int value = sqlite3_column_int(sqliteStmt, static_cast<int>(idx));
    return static_cast<int32>(value);
}


double SQLiteResultSet::getDouble(uint8 idx) const
{
    checkValidity(idx);
    double value = sqlite3_column_double(sqliteStmt, static_cast<int>(idx));
    return static_cast<int32>(value);
}

float SQLiteResultSet::getFloat(uint8 idx) const
{
    uint32 temp;
    float ret;
    temp = this->getUint32(idx);
    memcpy(&ret, &temp,4);
    return ret;
}

uint64 SQLiteResultSet::getUint64(uint8 idx) const
{
    return static_cast<uint64>(getInt64(idx));
}

int64 SQLiteResultSet::getInt64(uint8 idx) const
{
    checkValidity(idx);
    sqlite_int64 value = sqlite3_column_int64(sqliteStmt, static_cast<int>(idx));
    return static_cast<int64>(value);
}

std::string SQLiteResultSet::getString(uint8 idx) const
{
    checkValidity(idx);
    int len = sqlite3_column_bytes(sqliteStmt, static_cast<int>(idx));
    return std::string(reinterpret_cast<const char*>(sqlite3_column_text(sqliteStmt,static_cast<int>(idx))), len);
}

bool SQLiteResultSet::next()
{
    int rc = sqlite3_step(sqliteStmt);

    switch (rc)
    {
    case SQLITE_ROW:
        ++rowPosition;
        ++numRows;
        return true;
    case SQLITE_DONE:
        return false;
    default:
        throw SQLiteException("SQLiteResultSet: Error while executing or fetching data",sqlite);
        break;
    }

    return false;
}

bool SQLiteResultSet::isFirst() const
{
    return rowPosition == 1;
}

uint32 SQLiteResultSet::rowsCount() const
{
    return numRows;
}

};
};
