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
 *  @file    SQLException.h
 *  @brief
 *  @author  Albator
 *  @date    2009-07-23
 *
 */
#ifndef EXCEPTION_H_
#define EXCEPTION_H_


#include <stdexcept>
#include <string>

namespace Trinity
{
namespace SQL
{

class SQLException : public std::exception
{
public:

    SQLException(const std::string& mess) : std::exception(), message(mess)
    {

    }

    virtual ~SQLException() throw()
    {

    }

    const char* what() const throw()
    {
        return message.c_str();
    }

protected:
    std::string message;

};

}
}


#endif /* EXCEPTION_H_ */
