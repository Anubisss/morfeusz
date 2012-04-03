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
 *  @file    MySQLStatement.h
 *  @brief	Mysql impl of Statement.h
 *  @author  Albator
 *  @date    2009-07-22
 *
 */
#ifndef MYSQLSTATEMENT_H_
#define MYSQLSTATEMENT_H_

#include "../Statement.h"
#include "MySQLConnection.h"

namespace Morpheus
{
namespace SQL
{

class MySQLStatement : public Statement
{
public:
    MySQLStatement(MySQLConnection* conn);

    void close();

    /**
     * @return true
     */
    bool execute(const std::string& sql);

    ResultSet* executeQuery(const std::string& sql);

    int executeUpdate(const std::string& sql);

    Connection* getConnection()
    {
        return mysqlConn;
    }

    ResultSet* getResultSet();

    const bool isClosed() const
    {
        return closed;
    }

private:

    void doQuery(const char* sql, uint32 length);
    void checkClosed();
    MySQLConnection* mysqlConn;
    bool closed;
};

};
};

#endif /* MYSQLSTATEMENT_H_ */
