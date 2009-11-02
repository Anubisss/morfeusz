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
 *  @file   PreparedStatement.h
 *  @brief
 *  @author  Albator
 *  @date    2009-07-22
 *
 */

#ifndef PREPAREDSTATEMENT_H_

#include <string>
#include "Statement.h"
#include "ResultSet.h"

namespace Trinity
{
namespace SQL
{

class PreparedStatement : public Statement
{
public:

    virtual void clearParameters() = 0;
    /**
     * Functions to bind data into prepared statement
     */
    virtual void setBool(const uint8 index,const bool value) = 0;
    virtual void setUint8(const uint8 index, const uint8 value) = 0;
    virtual void setUint16(const uint8 index, const uint16 value) = 0;
    virtual void setUint32(const uint8 index, const uint32 value) = 0;
    virtual void setUint64(const uint8 index, const uint64 value) = 0;
    virtual void setInt16(const uint8 index, const int16 value) = 0;
    virtual void setInt32(const uint8 index, const int32 value) = 0;
    virtual void setInt64(const uint8 index, const int64 value) = 0;
    virtual void setDouble(const uint8 index, const double value) = 0;
    virtual void setString(const uint8 index, const std::string& value) = 0;

    virtual bool execute() = 0;

    virtual ResultSet* executeQuery() = 0;

    virtual int executeUpdate() = 0;

    virtual uint32 parameterCount() const = 0;


};

}
}

#define PREPAREDSTATEMENT_H_


#endif /* PREPAREDSTATEMENT_H_ */
