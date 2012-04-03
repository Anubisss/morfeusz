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
 *  @file    Connection.h
 *  @brief
 *  @author  Albator
 *  @date    2009-07-22
 *
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include "Statement.h"
#include "PreparedStatement.h"
#include <string>

namespace Morpheus
{
namespace SQL
{

class Connection
{
public:
    virtual void close() = 0;

    virtual void commit() = 0;
    virtual void rollback() = 0;

    virtual Statement* createStatement() = 0;

    virtual PreparedStatement* prepareStatement(const std::string& sql) = 0;

    virtual void setAutoCommit(bool autoCommit) = 0;

    const bool getAutoCommit() const
    {
        return autoCommit;
    }

    const bool isClosed() const
    {
        return closed;
    }

protected:

    bool autoCommit;
    bool closed;
};

};
};

#endif /* CONNECTION_H_ */
