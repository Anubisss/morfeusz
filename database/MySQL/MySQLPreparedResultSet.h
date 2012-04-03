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
 *  @file    MySQLPreparedResultSet.h
 *  @brief
 *  @author  Albator
 *  @date    2009-07-22
 *
 */

#ifndef MYSQLPREPAREDRESULTSET_H_
#define MYSQLPREPAREDRESULTSET_H_

#include "MySQLInclude.h"
#include "../ResultSet.h"
#include <memory>

namespace Morpheus
{
namespace SQL
{

class MySQLPreparedStatement;

class MySQLPreparedResultSet : public ResultSet
{

public:

    MySQLPreparedResultSet(MySQLPreparedStatement* stmt, MYSQL_STMT* mysqlStmt);

    ~MySQLPreparedResultSet();

    bool getBool(uint8 idx) const;

    uint8 getUint8(uint8 idx) const;

    uint16 getUint16(uint8 idx) const;

    uint32 getUint32(uint8 idx) const;

    int8 getInt8(uint8 idx) const;

    int16 getInt16(uint8 idx) const;

    int32 getInt32(uint8 idx) const;

    double getDouble(uint8 idx) const;

    float getFloat(uint8 idx) const;

    uint64 getUint64(uint8 idx) const;

    int64 getInt64(uint8 idx) const;

    std::string getString(uint8 idx) const;

    /**
     * move to the next row
     */
    bool next();

    /**
     * move to the first row
     */
    //bool first();

    /**
     * move before the first row
     */
    //bool beforeFirst();


    /**
     * move after last row
     */
    //bool afterLast();

    /**
     * Check if the current row is the first
     */
    bool isFirst() const;

    uint32 rowsCount() const;
private:

    void checkValidity(uint8 idx) const;

    void fillBindResult();

    MySQLPreparedStatement* stmt;
    MYSQL_STMT* mysqlStmt;
    MYSQL_RES* metaResult;
    MYSQL_BIND* bindResult;
    uint32 rowPosition;
    uint32 numRows;
    uint32 numFields;
    my_bool* isNull;
    my_bool* error;
    unsigned long* length;
};

};
};

#endif /* MYSQLPREPAREDRESULTSET_H_ */
