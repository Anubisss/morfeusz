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
 *  @file    Callback.h
 *  @brief   Contains generic callback class
 *  @author  albator
 *  @date    2009-07-02
 *
 */

#include "Common.h"
#include <ace/Refcounted_Auto_Ptr.h>
#include <ace/Null_Mutex.h>

#pragma once


template<class C, typename T>
class Callback
{
public:
    typedef void (C::*callback_func)(T data);

    Callback(ACE_Refcounted_Auto_Ptr<C, ACE_Recursive_Thread_Mutex> obj, callback_func f) : object(obj), method(f)
      , raw_ptr(NULL)
    {

    }

 Callback(C* p, callback_func f): raw_ptr(p), method(f){}

    void call(T value)
    {
      if(raw_ptr)
	{
	  (raw_ptr->*method)(value);
	}
      else
	{
	  C* objPtr = NULL;
	  objPtr = object.get();
	  if(objPtr)
	    (objPtr->*method)(value);
	}
    }
    C* get_obj(){return object.get();}
private:
    callback_func method;
    ACE_Refcounted_Auto_Ptr<C, ACE_Recursive_Thread_Mutex> object;
    C* raw_ptr;
};
