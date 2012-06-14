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
 *  @file    SQLiteConnection.cpp
 *  @brief
 *  @author  Albator
 *  @date    2009-07-22
 *
 */

#include "SQLiteConnection.h"
#include "SQLiteStatement.h"
#include "SQLitePreparedStatement.h"
#include "SQLiteException.h"

#include "Util.h"

#include <string>
#include <vector>
#include <ace/OS.h>

namespace Morpheus
{

namespace SQL
{

SQLiteConnection::SQLiteConnection(const std::string& url) :
        Connection(),
        sqlite(NULL)
{
    Connection::closed = true;
    // parse the connect string
    std::vector<std::string>::iterator it;
    std::vector<std::string> tokens = Utils::StrSplit(url,";");


    std::string database;
    int port;

    if (tokens.size() < 1) // not enough param
        throw SQLiteException("SQLiteConnection: Invalid number of param");

    it = tokens.begin();
    if (it!=tokens.end()) {
        database = *it;
        ++it;
    }

    if (sqlite3_threadsafe() == 0)
        throw SQLiteException("SQLiteConnection: the sqlite lib doesn't support multi-threading");



    int rc = sqlite3_open_v2(database.c_str(),&sqlite,SQLITE_OPEN_FULLMUTEX,NULL);
    if (rc != SQLITE_OK) {
        //sqlite3_close(sqlite);
        throw SQLiteException("SQLiteConnection: error on sqlite3_open (check the params)",sqlite);
    }

    Connection::closed = false;
}

SQLiteConnection::~SQLiteConnection()
{
    if (!closed)
        close();
}

void SQLiteConnection::close()
{
    sqlite3_close(sqlite);
    sqlite = NULL;
    closed = true;
}

void SQLiteConnection::commit()
{
    checkClosed();
    // nothing as sqlite doesn't support transaction
}

void SQLiteConnection::rollback()
{
    checkClosed();
    // nothing as sqlite doesn't support transaction
}

Statement* SQLiteConnection::createStatement()
{
    checkClosed();
    return new SQLiteStatement(this);
}

PreparedStatement* SQLiteConnection::prepareStatement(const std::string& sql)
{
    checkClosed();
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(sqlite,sql.c_str(),static_cast<int>(sql.length()),&stmt, NULL);
    if (rc!=SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw SQLiteException("SQLiteConnection: error on sqlite3_prepare_v2",sqlite);
    }

    return new SQLitePreparedStatement(this,stmt);
}

void SQLiteConnection::setAutoCommit(bool autoCommit)
{
    checkClosed();
    this->autoCommit = autoCommit;
}

void SQLiteConnection::checkClosed()
{
    if (Connection::closed)
        throw SQLiteException("SQLiteConnection: connection closed");
}

};
};

