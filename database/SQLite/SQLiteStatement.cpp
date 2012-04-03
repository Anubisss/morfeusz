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
 *  @file    SQLiteStatement.cpp
 *  @brief
 *  @author  Albator
 *  @date    2009-07-22
 *
 */

#include "SQLiteStatement.h"
#include "SQLiteResultSet.h"
#include "SQLiteInclude.h"
#include "SQLiteException.h"
#include <string>

namespace Morpheus
{

namespace SQL
{

SQLiteStatement::SQLiteStatement(SQLiteConnection* conn) : sqliteConn(conn), closed(false), stmt(NULL)
{

}

SQLiteStatement::~SQLiteStatement()
{
    if (stmt)
        sqlite3_finalize(stmt);
}

sqlite3* SQLiteStatement::getSQLiteHandle() const
{
    return sqliteConn->getSQLiteHandle();
}

void SQLiteStatement::close()
{
    checkClosed();
    closed = true;
}

bool SQLiteStatement::execute(const std::string& sql)
{
    doQuery(sql.c_str(), sql.length());
    return sqlite3_column_count(stmt) > 0; // check if the query return something
}

int SQLiteStatement::executeUpdate(const std::string& sql)
{
    doQuery(sql.c_str(), sql.length());
    if (sqlite3_column_count(stmt) > 0) // check if the query return something
        throw SQLiteException("SQLiteStatement::executeUpdate, error the return something");

    sqlite3* sqlite = sqliteConn->getSQLiteHandle();

    // execute the query to get the number of changes
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
        throw SQLiteException("SQLiteStatement::executeUpdate, sqlite3_step invalid return value",sqlite);

    return sqlite3_changes(sqlite);
}


ResultSet* SQLiteStatement::executeQuery(const std::string& sql)
{
    doQuery(sql.c_str(), sql.length());
    return getResultSet();
}

void SQLiteStatement::checkClosed()
{
    if (closed) {
        sqlite3* sqlite = sqliteConn->getSQLiteHandle();
        throw SQLiteException("SQLiteStatement::checkClosed, statement closed",sqlite);

    }
}

ResultSet* SQLiteStatement::getResultSet()
{
    checkClosed();
    return new SQLiteResultSet(this);
}

void SQLiteStatement::doQuery(const char* sql, uint32 length)
{
    checkClosed();
    sqlite3* sqlite = sqliteConn->getSQLiteHandle();

    int rc = sqlite3_prepare_v2(sqlite,sql,static_cast<int>(length),&stmt,NULL);
    if (rc!=SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw SQLiteException("SQLiteStatement: sqlite3_prepare_v2 error",sqlite);
    }
}

};
};

