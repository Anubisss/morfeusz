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
 *  @brief   Proxies' database access classes.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-24
 *
 */

#ifndef _PROXY_DATABASE_H
#define _PROXY_DATABASE_H
#include "Database.h"
#include "Callback.h"

namespace Trinity
{
namespace DatabaseAccess
{

class ProxyDatabaseConnection : protected DatabaseConnection
{
 public:
 ProxyDatabaseConnection(ACE_Activation_Queue* q): DatabaseConnection(q){};
  ~ProxyDatabaseConnection(){if(worker) delete worker;};
  bool open(const std::string& driver, const std::string& url);
};

class ProxyDB : protected DatabaseHolder<ProxyDatabaseConnection>
{
 public:
 ProxyDB(int c): DatabaseHolder<ProxyDatabaseConnection>(c){}
};

};
};
#endif //_PROXY_DATABASE_H