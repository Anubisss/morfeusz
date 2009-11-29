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
 *  @file    MySQLPreparedResultSet.cpp
 *  @brief
 *  @author  Albator
 *  @date    2009-07-22
 *
 */

#include <iostream>
#include <sstream>
#include <ace/OS.h>

#include "MySQLPreparedResultSet.h"
#include "MySQLException.h"
#include "MySQLConnection.h"
#include "MySQLPreparedStatement.h"




namespace Trinity
{

namespace SQL
{

MySQLPreparedResultSet::MySQLPreparedResultSet(MySQLPreparedStatement* stmt, MYSQL_STMT* mysqlStmt) :
        stmt(stmt),
        mysqlStmt(mysqlStmt),
        rowPosition(0),
        numRows(0),
        numFields(0)
{

    metaResult = mysql_stmt_result_metadata(mysqlStmt);

    if (!metaResult)
    {
        MYSQL* mysql = stmt->getMySQLHandle();
        throw MySQLException("MySQLPreparedResultSet, can't get meta data (mysql_stmt_result_metadata)",mysql);
    }
    numFields = mysql_stmt_field_count(mysqlStmt);

    if(mysqlStmt->bind_result_done )
      {
	delete[] mysqlStmt->bind->length;
	delete[] mysqlStmt->bind->is_null;
      }
    
    bindResult =new MYSQL_BIND[numFields];
    isNull = new my_bool[numFields];
    length = new unsigned long[numFields]; 
    
    ACE_OS::memset(bindResult,0,numFields*sizeof(MYSQL_BIND));  
    ACE_OS::memset(isNull,0,numFields*sizeof(my_bool));
    ACE_OS::memset(length, 0,numFields*sizeof(unsigned long));


    if(mysql_stmt_store_result(mysqlStmt) )
      throw MySQLException(std::string( mysql_stmt_error(mysqlStmt)));
    fillBindResult();

    // now bind results
    if (mysql_stmt_bind_result(mysqlStmt, bindResult))
    {
        delete[] bindResult;
        delete[] isNull;
        delete[] length;
        throw MySQLException("MySQLPreparedResultSet, can't bind result (mysql_stmt_bind_result error)");
    }

    numRows = mysql_stmt_num_rows(mysqlStmt);
}

inline uint32 typeToSize(enum_field_types t)
{
    switch (t)
    {

    case MYSQL_TYPE_TINY:
        return 1; // char
    case MYSQL_TYPE_SHORT:
        return 2; // short int
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_LONG:
        return 4; //long int / float
    case MYSQL_TYPE_DOUBLE:
    case MYSQL_TYPE_LONGLONG:
        return 8; // long long int / double

    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_DATETIME:
        return sizeof(MYSQL_TIME);

    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_BLOB:
        return 0;

    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_NEWDATE:
    case MYSQL_TYPE_BIT:
    case MYSQL_TYPE_NEWDECIMAL:
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_NULL:
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_GEOMETRY:
    default:
      throw MySQLException("MySQLPreparedResultSet, unsupported data type");
      return 0;
    }
}

void MySQLPreparedResultSet::fillBindResult()
{
    MYSQL_FIELD* field;
    uint32 i = 0;
    while ((field = mysql_fetch_field(metaResult)))
    {
        enum_field_types t = field->type;
        uint32 len = typeToSize(t);
        if (len == 0)
        {
            len = field->max_length+1;
        }
	else
	  len = field->max_length;

        bindResult[i].buffer_type = t;
        bindResult[i].buffer = ACE_OS::malloc(len);
	ACE_OS::memset(bindResult[i].buffer,0,len);
        bindResult[i].buffer_length = (len);
        bindResult[i].length = &length[i];
	bindResult[i].is_null = &isNull[i];
	bindResult[i].error = NULL;
        ++i;

        if (i > numFields)
        {
            throw MySQLException("Critical MySQL error: mysql_fetch_field != mysql_stmt_field_count");
        }

    }
}

MySQLPreparedResultSet::~MySQLPreparedResultSet()
{
  if (metaResult)
    mysql_free_result(metaResult);
  
    for (int i=0; i < numFields; i++)
    {
      ACE_OS::free(bindResult[i].buffer);

    }
    mysql_stmt_free_result(mysqlStmt);
    delete[] bindResult;

}

void MySQLPreparedResultSet::checkValidity(uint8 idx) const
{
    if (idx == 0 || idx > numFields)
    {
        throw MySQLException("MySQLPreparedResultSet, invalid index");
    }

    if (rowPosition == 0)
    {
        throw MySQLException("MySQLPreparedResultSet, before first row");
    }


}

bool MySQLPreparedResultSet::getBool(uint8 idx) const
{
    checkValidity(idx);

    if (getInt8(idx) > 0)
        return true;

    return false;


}

uint8 MySQLPreparedResultSet::getUint8(uint8 idx) const
{
    checkValidity(idx);
    if (*bindResult[idx -1].is_null)
        return 0;

    uint8 value = *reinterpret_cast<uint8*>(bindResult[idx-1].buffer);
    return value;
}

uint16 MySQLPreparedResultSet::getUint16(uint8 idx) const
{
    checkValidity(idx);
    if (*bindResult[idx -1].is_null)
        return 0;

    uint16 value = *reinterpret_cast<uint16*>(bindResult[idx-1].buffer);
    return value;

}

uint32 MySQLPreparedResultSet::getUint32(uint8 idx) const
{
    checkValidity(idx);
    if (*bindResult[idx -1].is_null == 1)
        return 0;

    uint32 value = *reinterpret_cast<uint32*>(bindResult[idx-1].buffer);
    return value;

}

int8 MySQLPreparedResultSet::getInt8(uint8 idx) const
{
    checkValidity(idx);
    if (*bindResult[idx -1].is_null)
        return 0;

    int8 value = *reinterpret_cast<int8*>(bindResult[idx-1].buffer);
    return value;

}

int16 MySQLPreparedResultSet::getInt16(uint8 idx) const
{
    checkValidity(idx);
    if (*bindResult[idx -1].is_null)
        return 0;

    int16 value = *reinterpret_cast<int16*>(bindResult[idx-1].buffer);
    return value;

}

int32 MySQLPreparedResultSet::getInt32(uint8 idx) const
{
    checkValidity(idx);
    if (isNull[idx -1])
        return 0;

    int32 value = *reinterpret_cast<int32*>(bindResult[idx-1].buffer);
    return value;
}


double MySQLPreparedResultSet::getDouble(uint8 idx) const
{
    checkValidity(idx);
    if (*bindResult[idx -1].is_null)
        return 0;

    double value = *reinterpret_cast<double*>(bindResult[idx-1].buffer);
    return value;

}

uint64 MySQLPreparedResultSet::getUint64(uint8 idx) const
{
    checkValidity(idx);
    if (*bindResult[idx -1].is_null)
        return 0;
    uint64 value = *reinterpret_cast<uint64*>(bindResult[idx-1].buffer);
    return value;


}

int64 MySQLPreparedResultSet::getInt64(uint8 idx) const
{
    checkValidity(idx);
    if (*bindResult[idx -1].is_null)
        return 0;
    int64 value = *reinterpret_cast<int64*>(bindResult[idx-1].buffer);
    return value;


}

std::string MySQLPreparedResultSet::getString(uint8 idx) const
{
    checkValidity(idx);
    if (*bindResult[idx -1].is_null)
        return std::string("");
    return std::string(static_cast<char *>(bindResult[idx - 1].buffer), *bindResult[idx - 1].length);

}

float MySQLPreparedResultSet::getFloat(uint8 idx) const
{
  uint32 temp;
  float ret;
  temp = this->getUint32(idx);
  memcpy(&ret, &temp, 4);
  return ret;
}

bool MySQLPreparedResultSet::next()
{
    bool ret = false;
    int res = mysql_stmt_fetch(mysqlStmt);
    ++rowPosition;

    if (!res || res == MYSQL_DATA_TRUNCATED)
      return true;
    
    if (res == MYSQL_NO_DATA)
      return false;

    return ret;


}

bool MySQLPreparedResultSet::isFirst() const
{
    return rowPosition == 1;

}

uint32 MySQLPreparedResultSet::rowsCount() const
{
    return numRows;
}

/*
bool MySQLPreparedResultSet::first()
{
	rowPosition = 1;

	return numRows > 0;
}

bool MySQLPreparedResultSet::beforeFirst()
{
	rowPosition = 0;
}

bool MySQLPreparedResultSet::afterLast()
{
	rowPosition = numRows + 1;
}

*/



}
}


