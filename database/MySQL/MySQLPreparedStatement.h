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
 *  @file    MySQLPreparedStatement.h
 *  @brief    Mysql impl of PreparedStatement.h
 *  @author  Albator
 *  @date    2009-07-22
 *
 */

#ifndef MYSQLPREPAREDSTATEMENT_H_
#define MYSQLPREPAREDSTATEMENT_H_

#include "MySQLInclude.h"
#include "../PreparedStatement.h"
#include "MySQLConnection.h"
#include <vector>

namespace Morpheus
{
namespace SQL
{

class MySQLPreparedStatement : public PreparedStatement
{

public:

    MySQLPreparedStatement(MySQLConnection* conn, MYSQL_STMT* stmt);

    virtual ~MySQLPreparedStatement();

    void close();

    bool execute(const std::string& sql);

    ResultSet* executeQuery(const std::string& sql);

    int executeUpdate(const std::string& sql);

    bool execute();

    ResultSet* executeQuery();

    int executeUpdate();

    Connection* getConnection()
    {
        return mysqlConn;
    }

    ResultSet* getResultSet();

    const bool isClosed() const
    {
        return closed;
    }

    void clearParameters();

    void setBool(const uint8 index,const bool value);
    void setUint8(const uint8 index, const uint8 value);
    void setUint16(const uint8 index, const uint16 value);
    void setUint32(const uint8 index, const uint32 value);
    void setUint64(const uint8 index, const uint64 value);
    void setInt16(const uint8 index, const int16 value);
    void setInt32(const uint8 index, const int32 value);
    void setInt64(const uint8 index, const int64 value);
    void setDouble(const uint8 index, const double value);
    void setString(const uint8 index, const std::string& value);

    uint32 parameterCount() const
    {
        return this->paramCount;
    }

    MYSQL* getMySQLHandle() const;

private :

    void doQuery();

    void checkClosed();

    void checkValidity(const uint8 idx);

    bool isAllSet();

    void setValue(MYSQL_BIND* param, const void* value, uint32 len);

    MySQLConnection* mysqlConn;
    bool closed;
    MYSQL_STMT* stmt;
    uint32 paramCount;
    std::vector<bool> paramsSet;
    MYSQL_BIND* bind;
};

};
};

#endif /* MYSQLPREPAREDSTATEMENT_H_ */
