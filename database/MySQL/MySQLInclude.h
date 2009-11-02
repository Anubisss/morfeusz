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
 *  @file    MySQLInclude.h
 *  @brief	Include all files related to libmysql
 *  @author  Albator
 *  @date    2009-07-22
 *
 */
#ifndef MYSQLINCLUDE_H_
#define MYSQLINCLUDE_H_


#include <Common.h>

#if PLATFORM != PLATFORM_WIN32
extern "C"
{
#endif

#undef _REENTRANT

#if PLATFORM == PLATFORM_WIN32
#include <my_global.h>
#endif

#include <errmsg.h>
#include <mysql.h>

#if PLATFORM != PLATFORM_WIN32
}
#endif





#endif /* MYSQLINCLUDE_H_ */
