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
 *  @file    MySQLException.cpp
 *  @brief
 *  @author  Albator
 *  @date    2009-07-23
 *
 */

#include "MySQLException.h"
#include <iostream>
#include <sstream>

namespace Morpheus
{
namespace SQL
{

MySQLException::MySQLException(const std::string& mess) : SQLException(mess)
{

}

MySQLException::MySQLException(const std::string& mess, MYSQL* mysql) : SQLException("")
{
    std::ostringstream o;
    o << mess;
    if (mysql) {
        if (*mysql_error(mysql)) {
            o << "\n" << mysql_errno(mysql) << ": " << mysql_error(mysql);
        }

    }

    SQLException::message = o.str();
}

MySQLException::~MySQLException() throw()
{

}

};
};
