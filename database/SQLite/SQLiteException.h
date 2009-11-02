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
 *  @file    SQLiteException.h
 *  @brief
 *  @author  Albator
 *  @date    2009-07-23
 *
 */
#ifndef SQLITEEXCEPTION_H_
#define SQLITEEXCEPTION_H_

#include "../SQLException.h"
#include "SQLiteInclude.h"
#include <string>

namespace Trinity
{
namespace SQL
{

class SQLiteException : public SQLException
{
public:
    SQLiteException(const std::string& message, sqlite3* sqlite);
    SQLiteException(const std::string& message = "");
    virtual ~SQLiteException() throw();


};

}
}

#endif /* SQLITEEXCEPTION_H_ */
