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
 *  @file    ResultSet.h
 *  @brief   common interface for resultset. Abstract class.
 *  @author  Albator
 *  @date    2009-07-22
 *
 */

#ifndef RESULTSET_H_
#define RESULTSET_H_

#include <Common.h>
#include <string>

namespace Morpheus
{

namespace SQL
{

class ResultSet
{

public:

    virtual ~ResultSet() {}

    virtual bool getBool(uint8 idx) const = 0;

    virtual uint8 getUint8(uint8 idx) const = 0;

    virtual uint16 getUint16(uint8 idx)const  = 0;

    virtual uint32 getUint32(uint8 idx) const = 0;

    virtual int8 getInt8(uint8 idx) const = 0;

    virtual int16 getInt16(uint8 idx) const = 0;

    virtual int32 getInt32(uint8 idx) const = 0;

    virtual double getDouble(uint8 idx) const = 0;

    virtual float getFloat(uint8 idx) const = 0;

    virtual uint64 getUint64(uint8 idx) const  = 0;

    virtual int64 getInt64(uint8 idx) const = 0;

    virtual std::string getString(uint8 idx) const = 0;

    /**
     * move to the next row
     */
    virtual bool next() = 0;

    /*
    /**
     * move to the first row
     */
    // bool first() = 0;

    /**
     * move before the first row
     */
    // bool beforeFirst() = 0;


    /**
     * move after last row
     */
    // bool afterLast() = 0;


    /**
     * Check if the current row is the first
     */
    virtual bool isFirst() const = 0;

    /**
     * @return
     */
    virtual uint32 rowsCount() const = 0;
};

};
};

#endif /* RESULTSET_H_ */
