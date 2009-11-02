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
 *  @file    DriverManager.cpp
 *  @brief
 *  @author  Albator
 *  @date    2009-07-22
 *
 */


#include "DriverManager.h"
#include <string>
#include <algorithm>
#include <iostream>

#ifdef HAVE_MYSQL
#include "MySQL/MySQLDriver.h"
#endif

#ifdef HAVE_SQLITE
#include "SQLite/SQLiteDriver.h"
#endif



namespace Trinity
{
namespace SQL
{

Driver* DriverManager::getDriver(const std::string& driverName)
{
    std::string dname = driverName;
    std::transform(driverName.begin(),driverName.end(),dname.begin(), ::tolower);

#ifdef HAVE_MYSQL
    if (dname.compare("mysql") == 0)
    {
        return new MySQLDriver();
    }
#endif
#ifdef HAVE_SQLITE
    if (dname.compare("sqlite") == 0)
    {
        return new SQLiteDriver();
    }
#endif

    //throw
    return NULL;
}

Connection* DriverManager::connect(const std::string& driverName, const std::string& url)
{
    Driver* d = getDriver(driverName);
    Connection* c = d->connect(url);
    return c;
}

}
}
