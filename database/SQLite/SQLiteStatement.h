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
 *  @file    SQLiteStatement.h
 *  @brief    sqlite impl of Statement.h
 *  @author  Albator
 *  @date    2009-07-22
 *
 */
#ifndef SQLITESTATEMENT_H_
#define SQLITESTATEMENT_H_

#include "../Statement.h"
#include "SQLiteConnection.h"

namespace Morpheus
{

namespace SQL
{

class SQLiteStatement : public Statement
{

public:

    SQLiteStatement(SQLiteConnection* conn);
    virtual ~SQLiteStatement();
    void close();

    /**
     * @return true
     */
    bool execute(const std::string& sql);

    ResultSet* executeQuery(const std::string& sql);

    int executeUpdate(const std::string& sql);

    Connection* getConnection()
    {
        return sqliteConn;
    }

    ResultSet* getResultSet();

    const bool isClosed() const
    {
        return closed;
    }

    sqlite3_stmt* getSQLiteStatement() const
    {
        return stmt;
    }

    sqlite3* getSQLiteHandle() const;

private :

    void doQuery(const char* sql, uint32 length);
    void checkClosed();
    SQLiteConnection* sqliteConn;
    sqlite3_stmt* stmt;
    bool closed;
};

};
};

#endif /* SQLITESTATEMENT_H_ */
