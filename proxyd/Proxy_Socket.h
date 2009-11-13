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
 *  @brief   Proxy_Socket class definition
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-13
 *
 */
#ifndef PROXY_SOCKET_H
#define PROXY_SOCKET_H

#include <ace/Svc_Handler.h>
#include <ace/SOCK_Stream.h>
#include <ace/Refcounted_Auto_Ptr.h>

namespace Trinity
{
namespace Proxyd
{

class Proxy_Socket : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>
{

};

};
};

#endif //PROXY_SOCKET_H
