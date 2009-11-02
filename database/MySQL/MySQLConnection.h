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
 *  @file    MySQLConnection.h
 *  @brief
 *  @author  Albator
 *  @date    2009-07-22
 *
 */

#ifndef MYSQLCONNECTION_H_
#define MYSQLCONNECTION_H_

#include "MySQLInclude.h"
#include "../Connection.h"
#include <string>


namespace Trinity
{
namespace SQL
{

class Statement;
class PreparedStatement;

class MySQLConnection: public Connection
{
public:

    MySQLConnection(const std::string& url);

    virtual ~MySQLConnection();

    void close();

    void commit();
    void rollback();

    Statement* createStatement();

    PreparedStatement* prepareStatement(const std::string& sql);

    void setAutoCommit(bool autoCommit);

    void checkClosed();

    MYSQL* getMySQLHandle() const
    {
        return mysql;
    }

private:
    MYSQL* mysql;



};

}
}

#endif /* MYSQLCONNECTION_H_ */
