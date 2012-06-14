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
 *  @file    MySQLConnection.cpp
 *  @brief
 *  @author  Albator
 *  @date    2009-07-22
 *
 */

#include <string>
#include <vector>
#include <Util.h>
#include <ace/OS.h>

#include "MySQLConnection.h"
#include "MySQLStatement.h"
#include "MySQLPreparedStatement.h"
#include "MySQLException.h"

namespace Morpheus
{

namespace SQL
{

MySQLConnection::MySQLConnection(const std::string& url) :
        Connection(),
        mysql(NULL)
{
    Connection::closed = true;
    // parse the connect string
    std::vector<std::string>::iterator it;
    std::vector<std::string> tokens = Utils::StrSplit(url,";");


    std::string host, user, password, schema;
    int port;

    if (tokens.size() < 5) { // not enough param
        throw MySQLException("MySQLConnection: Invalid number of param");
    }


    it = tokens.begin();
    if (it!=tokens.end()) {
        host = *it;
        ++it;
    }

    if (it!=tokens.end()) {
        port = ACE_OS::atoi((*it).c_str());;
        ++it;
    }

    if (it!=tokens.end()) {
        user = *it;
        ++it;
    }

    if (it!=tokens.end()) {
        password = *it;
        ++it;
    }

    if (it!=tokens.end()) {
        schema = *it;
        ++it;
    }

    // init mysql
    if (!(mysql = mysql_init(NULL))) {
        throw MySQLException("MySQLConnection: error on mysql_init");
    }

    // set charset - default utf8
    my_bool tmp = true;
    mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8");
    mysql_options(mysql, MYSQL_OPT_RECONNECT, &tmp);

    if (!mysql_real_connect(mysql,
                            host.c_str(),
                            user.c_str(),
                            password.c_str(),
                            schema.c_str(),
                            port,
                            NULL,
                            0))
    {
        mysql_close(mysql);
        throw MySQLException("MySQLConnection: error on mysql_real_connect (check the params)",mysql);
    }

    Connection::closed = false;
    mysql_set_server_option(mysql, MYSQL_OPTION_MULTI_STATEMENTS_ON);
    mysql_autocommit(mysql, 1); // auto commit ON

}

MySQLConnection::~MySQLConnection()
{
    if (!closed) {
        close();
    }
}

void MySQLConnection::close()
{
    mysql_close(mysql);
    mysql = NULL;
    closed = true;

}

void MySQLConnection::commit()
{
    checkClosed();
    mysql_commit(mysql);
}

void MySQLConnection::rollback()
{
    checkClosed();
    mysql_rollback(mysql);
}

Statement* MySQLConnection::createStatement()
{
    checkClosed();
    return new MySQLStatement(this);
}

PreparedStatement* MySQLConnection::prepareStatement(const std::string& sql)
{
    checkClosed();
    MYSQL_STMT * stmt = mysql_stmt_init(mysql);

    if (!stmt) {
        throw MySQLException("MySQLConnection: mysql_stmt_init error",mysql);
    }

    if (mysql_stmt_prepare(stmt, sql.c_str(), static_cast<unsigned long>(sql.length()))) {
        mysql_stmt_close(stmt);

        std::string error;
        error += "MySQLConnection: mysql_stmt_prepare error:\n";
        error += mysql_stmt_error(stmt);
        error += "\n query was:\n" + sql;
        throw MySQLException(error.c_str(), mysql);
    }

    return new MySQLPreparedStatement(this,stmt);
}

void MySQLConnection::setAutoCommit(bool autoCommit)
{
    checkClosed();
    if (autoCommit)
        mysql_autocommit(mysql, 1);
    else
        mysql_autocommit(mysql, 0);

    this->autoCommit = autoCommit;

}

void MySQLConnection::checkClosed()
{
    if (Connection::closed) {
        throw MySQLException("MySQLConnection: connection closed");
    }
}

};
};

