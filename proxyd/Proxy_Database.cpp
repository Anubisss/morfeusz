/* -*- C++ -*-
 * Copyright (C) 2009 Trinity Core <http://www.trinitycore.org>
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

/**
 *  @file
 *  @brief   Proxy database implementation.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-24
 *
 */

#include "Proxy_Database.h"

namespace Trinity
{
namespace DatabaseAccess
{

bool
ProxyDatabaseConnection::open(const std::string& driver, const std::string& url)
  try
    {
      this->driver = Trinity::SQL::DriverManager::getDriver(driver);
      this->connection = this->driver->connect(url);
    
      this->worker = new DatabaseWorker(this->query_queue, this);
    }
  catch(SQL::SQLException& e)
    {
	ACE_ERROR((LM_ERROR,"Error when setting up connection: %s\n",e.what() ));
	ACE_OS::exit(-1);
    }

};
};
