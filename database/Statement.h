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
 *  @file    Statement.h
 *  @brief	 JDBC-like (and lite) interface for statement
 *  @author  Albator
 *  @date    2009-07-22
 *
 */

#ifndef STATEMENT_H_
#define STATEMENT_H_

#include <string>

namespace Morpheus
{

namespace SQL
{

class Connection;
class ResultSet;

class Statement
{

public:

    virtual void close() = 0;

    virtual bool execute(const std::string& sql) = 0;

    virtual ResultSet* executeQuery(const std::string& sql) = 0;

    virtual int executeUpdate(const std::string& sql) = 0;

    virtual Connection* getConnection() = 0;

    virtual ResultSet* getResultSet() = 0;

    virtual const bool isClosed() const = 0;
};

};
};

#endif /* STATEMENT_H_ */
