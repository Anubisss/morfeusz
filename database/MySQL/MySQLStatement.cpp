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
 *  @file    MySQLStatement.cpp
 *  @brief
 *  @author  Albator
 *  @date    2009-07-22
 *
 */

#include "MySQLStatement.h"
#include "MySQLResultSet.h"
#include "MySQLInclude.h"
#include "MySQLException.h"
#include <string>

namespace Morpheus
{
namespace SQL
{

MySQLStatement::MySQLStatement(MySQLConnection* conn) : mysqlConn(conn), closed(false)
{

}

void MySQLStatement::close()
{
    checkClosed();
    closed = true;
}

bool MySQLStatement::execute(const std::string& sql)
{
    doQuery(sql.c_str(), sql.length());
    MYSQL* mysql = mysqlConn->getMySQLHandle();
    return mysql_field_count(mysql) > 0; // check if the query return something
}

int MySQLStatement::executeUpdate(const std::string& sql)
{
    doQuery(sql.c_str(), sql.length());
    MYSQL* mysql = mysqlConn->getMySQLHandle();
    if (mysql_field_count(mysql) > 0) { // check if the query return something
        throw MySQLException("MySQLStatement::executeUpdate, error the return something",mysql);
    }

    return static_cast<int>(mysql_affected_rows(mysql));
}

ResultSet* MySQLStatement::executeQuery(const std::string& sql)
{
    doQuery(sql.c_str(), sql.length());
    MYSQL* mysql = mysqlConn->getMySQLHandle();
    return getResultSet();
}

void MySQLStatement::checkClosed()
{
    if (closed) {
        MYSQL* mysql = mysqlConn->getMySQLHandle();
        throw MySQLException("MySQLStatement::checkClosed, statement closed",mysql);
    }
}

ResultSet* MySQLStatement::getResultSet()
{
    checkClosed();
    MYSQL* mysql = mysqlConn->getMySQLHandle();
    MYSQL_RES* res = NULL;

    res = mysql_store_result(mysql);

    if (!res) {
        // check possible error
        if (mysql_errno(mysql) !=0) // an error occurred
            throw MySQLException("MySQLStatement::getResultSet mysql_store_result error",mysql);

        // INSERT, UPDATE, DELETE : no result set
        return NULL;
    }

    return new MySQLResultSet(this, res);
}

void MySQLStatement::doQuery(const char* sql, uint32 length)
{
    checkClosed();
    MYSQL* mysql = mysqlConn->getMySQLHandle();

    if (mysql_real_query(mysql, sql, static_cast<unsigned long>(length)) && mysql_errno(mysql))
        throw MySQLException("MySQLStatement::doQuery, mysql_real_query error",mysql);
}

};
};
