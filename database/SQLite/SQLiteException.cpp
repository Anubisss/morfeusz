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
 *  @file    SQLiteException.cpp
 *  @brief
 *  @author  Albator
 *  @date    2009-07-23
 *
 */

#include "SQLiteException.h"
#include "SQLiteInclude.h"
#include <iostream>
#include <sstream>


namespace Trinity
{
namespace SQL
{

SQLiteException::SQLiteException(const std::string& mess, sqlite3* sqlite) : SQLException("")
{
    if (sqlite)
    {
        std::ostringstream o;
        o << mess << "\n" << sqlite3_errcode(sqlite) << ": " << sqlite3_errmsg(sqlite);
        message = o.str();
    }
    else
    {
        message = mess;
    }

}

SQLiteException::SQLiteException(const std::string& mess) : SQLException(mess)
{
}

SQLiteException::~SQLiteException() throw()
{

}

}
}
