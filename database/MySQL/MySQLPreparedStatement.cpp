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
 *  @file    MySQLPreparedStatement.cpp
 *  @brief
 *  @author  Albator
 *  @date    2009-07-22
 *
 */

#include <ace/OS.h>

#include "MySQLPreparedStatement.h"
#include "MySQLPreparedResultSet.h"
#include "MySQLException.h"
#include "MySQLInclude.h"

namespace Morpheus
{
namespace SQL
{

MySQLPreparedStatement::MySQLPreparedStatement(MySQLConnection* conn, MYSQL_STMT* stmt) :
        mysqlConn(conn),
        closed(false),
        stmt(stmt),
        bind(NULL)
{
    paramCount = mysql_stmt_param_count(stmt);
    paramsSet.assign(paramCount, false);
    bind = new MYSQL_BIND[paramCount];
    ACE_OS::memset(bind, 0, sizeof(MYSQL_BIND) * paramCount);
    my_bool	bool_tmp=1;
    mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, &bool_tmp);
}

MySQLPreparedStatement::~MySQLPreparedStatement()
{
    // delete all params
    clearParameters();
    mysql_stmt_close(stmt);
    delete[] bind;
}

MYSQL* MySQLPreparedStatement::getMySQLHandle() const
{
    return mysqlConn->getMySQLHandle();
}

void MySQLPreparedStatement::clearParameters()
{
    for (int i=0; i < paramCount; i++) {
        delete(char*) bind[i].length;
        bind[i].length = NULL;
        delete[](char*) bind[i].buffer;
        bind[i].buffer = NULL;
        paramsSet[i] = false;
    }

}

void MySQLPreparedStatement::close()
{
    checkClosed();
    closed = true;
}

bool MySQLPreparedStatement::execute()
{
    doQuery();
    my_bool	bool_tmp=1;
    mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, &bool_tmp);
    //mysql_stmt_store_result(stmt);
    return (mysql_stmt_field_count(stmt) > 0);
}

int MySQLPreparedStatement::executeUpdate()
{
    doQuery();
    return static_cast<int>(mysql_stmt_affected_rows(stmt));
}

ResultSet* MySQLPreparedStatement::executeQuery()
{
    //mysql_stmt_store_result(stmt);
    doQuery();
    
    return new MySQLPreparedResultSet(this, stmt);
}

void MySQLPreparedStatement::checkClosed()
{
    if (closed)
        throw MySQLException("MySQLPreparedStatement::checkClosed() : statement closed");
}

ResultSet* MySQLPreparedStatement::getResultSet()
{
    checkClosed();

    if (mysql_more_results(stmt->mysql))
        mysql_next_result(stmt->mysql);

    return new MySQLPreparedResultSet(this, stmt);
}

void MySQLPreparedStatement::doQuery()
{
    checkClosed();
    MYSQL* mysql = mysqlConn->getMySQLHandle();

    if (mysql_stmt_bind_param(stmt, bind))
        throw MySQLException("MySQLPreparedStatement::doQuery, mysql_stmt_bind_param error",mysql);

    if (mysql_stmt_execute(stmt))
        throw MySQLException("MySQLPreparedStatement::doQuery, mysql_stmt_execute error",mysql);
}

void MySQLPreparedStatement::checkValidity(const uint8 idx)
{
    if (idx == 0 || idx > paramCount)
        throw MySQLException("MySQLPreparedStatement::checkValidity(), invalid index");
}

bool MySQLPreparedStatement::isAllSet()
{
    std::vector<bool>::iterator it;
    for (it = paramsSet.begin(); it != paramsSet.end(); ++it) {
        if (!(*it))
            return false;
    }

    return true;
}

void MySQLPreparedStatement::setValue(MYSQL_BIND* param, const void* value, uint32 len)
{
    delete [] static_cast<char *>(param->buffer);
    param->buffer = new char[len];
    param->buffer_length = 0;
    param->is_null_value = 0;
    delete param->length;
    param->length	= NULL;

    ACE_OS::memcpy(param->buffer, value, len);
}

void MySQLPreparedStatement::setBool(const uint8 index,const bool value)
{
    setUint32(index,value);
}

void MySQLPreparedStatement::setUint8(const uint8 index, const uint8 value)
{
    setUint32(index,value);
}

void MySQLPreparedStatement::setUint16(const uint8 index, const uint16 value)
{
    setUint32(index,value);
}

void MySQLPreparedStatement::setUint32(const uint8 index, const uint32 value)
{
    checkValidity(index);
    paramsSet[index - 1] = true;
    MYSQL_BIND * param = &bind[index-1];
    param->buffer_type	= MYSQL_TYPE_LONG;
    setValue(param,&value,4);
}

void MySQLPreparedStatement::setUint64(const uint8 index, const uint64 value)
{
    checkValidity(index);
    paramsSet[index - 1] = true;
    MYSQL_BIND * param = &bind[index-1];
    param->buffer_type	= MYSQL_TYPE_LONGLONG;
    setValue(param,&value,8);
}

void MySQLPreparedStatement::setInt16(const uint8 index, const int16 value)
{
    setInt32(index,value);
}

void MySQLPreparedStatement::setInt32(const uint8 index, const int32 value)
{
    checkValidity(index);
    paramsSet[index - 1] = true;
    MYSQL_BIND * param = &bind[index-1];
    param->buffer_type	= MYSQL_TYPE_LONG;
    setValue(param,&value,4);
}

void MySQLPreparedStatement::setInt64(const uint8 index, const int64 value)
{
    checkValidity(index);
    paramsSet[index - 1] = true;
    MYSQL_BIND * param = &bind[index-1];
    param->buffer_type	= MYSQL_TYPE_LONGLONG;
    setValue(param,&value,8);
}

void MySQLPreparedStatement::setDouble(const uint8 index, const double value)
{
    checkValidity(index);
    paramsSet[index - 1] = true;
    MYSQL_BIND * param = &bind[index-1];
    param->buffer_type	= MYSQL_TYPE_DOUBLE;
    setValue(param,&value,8);
}

void MySQLPreparedStatement::setString(const uint8 index,
                                       const std::string& value)
{
    checkValidity(index);
    paramsSet[index - 1] = true;
    MYSQL_BIND * param = &bind[index-1];

    delete[] static_cast<char *>(param->buffer);

    param->buffer_type = MYSQL_TYPE_VAR_STRING;
    param->buffer = memcpy(new char[value.length() + 1], value.c_str(),
                           value.length() + 1);
    param->buffer_length = static_cast<unsigned long>(value.length()) + 1;
    param->is_null_value = 0;

    delete param->length;
    param->length = new unsigned long(
        static_cast<unsigned long>(value.length()));
}

//////////////////////////////////////
// unimplemented functions
//////////////////////////////////////
bool MySQLPreparedStatement::execute(const std::string& sql)
{
    throw MySQLException("MySQLPreparedStatement prepared statement doesn't support this operation execute(const std::string& sql)");
    return false;
}

int MySQLPreparedStatement::executeUpdate(const std::string& sql)
{
    throw MySQLException("MySQLPreparedStatement prepared statement doesn't support this operation executeUpdate(const std::string& sql)");
    return 0;
}


ResultSet* MySQLPreparedStatement::executeQuery(const std::string& sql)
{
    throw MySQLException("MySQLPreparedStatement prepared statement doesn't support this operation executeQuery(const std::string& sql)");
    return NULL;
}

};
};
