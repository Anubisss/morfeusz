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
 *  @file    SQLitePreparedStatement.cpp
 *  @brief
 *  @author  Albator
 *  @date    2009-07-22
 *
 */

#include "SQLitePreparedStatement.h"
#include "SQLiteResultSet.h"
#include "SQLiteException.h"
#include "SQLiteInclude.h"
#include "ace/OS.h"


namespace Trinity
{
namespace SQL
{

SQLitePreparedStatement::SQLitePreparedStatement(SQLiteConnection* conn, sqlite3_stmt* stmt) :
        sqliteConn(conn),
        closed(false),
        stmt(stmt)
{
    paramCount = static_cast<uint32>(sqlite3_bind_parameter_count(stmt));
    paramsSet.assign(paramCount, false);
}

SQLitePreparedStatement::~SQLitePreparedStatement()
{
    // delete all params
    if (stmt)
        sqlite3_finalize(stmt);

}

sqlite3* SQLitePreparedStatement::getSQLiteHandle() const
{
    return sqliteConn->getSQLiteHandle();
}

void SQLitePreparedStatement::clearParameters()
{
    for (int i=0; i < paramCount; i++)
    {
        paramsSet[i] = false;
    }

    sqlite3_clear_bindings(stmt);

}
void SQLitePreparedStatement::close()
{
    checkClosed();
    closed = true;
}




bool SQLitePreparedStatement::execute()
{
    doQuery();
    return sqlite3_column_count(stmt) > 0; // check if the query return something
}

int SQLitePreparedStatement::executeUpdate()
{
    doQuery();
    sqlite3* sqlite = sqliteConn->getSQLiteHandle();
    if (sqlite3_column_count(stmt) > 0) // check if the query return something
    {
        throw SQLiteException("SQLiteStatement::executeUpdate, error the return something",sqlite);
    }

    // execute the query to get the number of changes
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        throw SQLiteException("SQLiteStatement::executeUpdate, sqlite3_step invalid return value",sqlite);
    }


    return sqlite3_changes(sqlite);
}


ResultSet* SQLitePreparedStatement::executeQuery()
{
    doQuery();
    return getResultSet();
}

void SQLitePreparedStatement::checkClosed()
{
    if (closed)
    {
        sqlite3* sqlite = sqliteConn->getSQLiteHandle();
        throw SQLiteException("SQLitePreparedStatement::checkClosed() : statement closed",sqlite);
    }
}

ResultSet* SQLitePreparedStatement::getResultSet()
{
    checkClosed();
    return new SQLiteResultSet(this);
}

void SQLitePreparedStatement::doQuery()
{
    checkClosed();
    if (!isAllSet())
    {
        sqlite3* sqlite = sqliteConn->getSQLiteHandle();
        throw SQLiteException("SQLitePreparedStatement: some parameters are not set",sqlite);
    }
}

void SQLitePreparedStatement::checkValidity(const uint8 idx)
{
    if (idx == 0 || idx > paramCount)
    {
        sqlite3* sqlite = sqliteConn->getSQLiteHandle();
        throw SQLiteException("SQLitePreparedStatement::checkValidity(), invalid index",sqlite);
    }
}

bool SQLitePreparedStatement::isAllSet()
{
    std::vector<bool>::iterator it;
    for (it = paramsSet.begin(); it != paramsSet.end(); ++it)
    {
        if (!(*it))
            return false;
    }

    return true;
}


void SQLitePreparedStatement::setBool(const uint8 index,const bool value)
{
    setUint32(index,value);
}

void SQLitePreparedStatement::setUint8(const uint8 index, const uint8 value)
{
    setUint32(index,value);
}


void SQLitePreparedStatement::setUint16(const uint8 index, const uint16 value)
{
    setUint32(index,value);
}


void SQLitePreparedStatement::setUint32(const uint8 index, const uint32 value)
{
    setInt32(index,value);
}


void SQLitePreparedStatement::setUint64(const uint8 index, const uint64 value)
{
    setInt64(index,static_cast<int64>(value));
}


void SQLitePreparedStatement::setInt16(const uint8 index, const int16 value)
{
    setInt32(index,value);
}


void SQLitePreparedStatement::setInt32(const uint8 index, const int32 value)
{
    checkValidity(index);
    paramsSet[index - 1] = true;
    int rc =  sqlite3_bind_int(stmt, static_cast<int>(index), static_cast<int>(value));
    if (rc!=SQLITE_OK)
    {
        sqlite3* sqlite = sqliteConn->getSQLiteHandle();
        throw SQLiteException("SQLitePreparedStatement: can't bind value",sqlite);
    }

}


void SQLitePreparedStatement::setInt64(const uint8 index, const int64 value)
{
    checkValidity(index);
    paramsSet[index - 1] = true;
    int rc = sqlite3_bind_int64(stmt, static_cast<int>(index),
                                static_cast<sqlite3_int64>(value));
    if (rc != SQLITE_OK)
    {
        sqlite3* sqlite = sqliteConn->getSQLiteHandle();
        throw SQLiteException("SQLitePreparedStatement: can't bind value",sqlite);
    }

}


void SQLitePreparedStatement::setDouble(const uint8 index, const double value)
{
    checkValidity(index);
    paramsSet[index - 1] = true;
    int rc = sqlite3_bind_double(stmt, static_cast<int>(index),
                                 static_cast<double>(value));
    if (rc != SQLITE_OK)
    {
        sqlite3* sqlite = sqliteConn->getSQLiteHandle();
        throw SQLiteException("SQLitePreparedStatement: can't bind value",sqlite);
    }

}



void SQLitePreparedStatement::setString(const uint8 index,
                                        const std::string& value)
{
    checkValidity(index);
    paramsSet[index - 1] = true;
    int rc = sqlite3_bind_text(stmt, static_cast<int>(index),
                               value.c_str(), static_cast<int>(value.length()),SQLITE_TRANSIENT);
    if (rc != SQLITE_OK)
    {
        sqlite3* sqlite = sqliteConn->getSQLiteHandle();
        throw SQLiteException("SQLitePreparedStatement: can't bind value",sqlite);
    }
}




//////////////////////////////////////
// unimplemented functions
//////////////////////////////////////
bool SQLitePreparedStatement::execute(const std::string& sql)
{
    sqlite3* sqlite = sqliteConn->getSQLiteHandle();
    throw SQLiteException("SQLitePreparedStatement prepared statement doesn't support this operation execute(const std::string& sql)",sqlite);
    return false;
}

int SQLitePreparedStatement::executeUpdate(const std::string& sql)
{
    sqlite3* sqlite = sqliteConn->getSQLiteHandle();
    throw SQLiteException("SQLitePreparedStatement prepared statement doesn't support this operation executeUpdate(const std::string& sql)",sqlite);
    return 0;
}


ResultSet* SQLitePreparedStatement::executeQuery(const std::string& sql)
{
    sqlite3* sqlite = sqliteConn->getSQLiteHandle();
    throw SQLiteException("SQLitePreparedStatement prepared statement doesn't support this operation executeQuery(const std::string& sql)",sqlite);
    return NULL;
}

}
}
