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
 *  @file    Database.cpp
 *  @brief   Contains implementation of Database Access functions
 *  @author  raczman <raczman@gmail.com>, albator
 *  @date    2009-07-02
 *
 */

#include "Database.h"

#if HAVE_MYSQL
    #if PLATFORM == PLATFORM_WIN32
        #include <mysql.h>
    #else
        #include <mysql/mysql.h>
    #endif
#endif

namespace Morpheus
{

namespace DatabaseAccess
{

////////////////////////////////////////////////////////
// SqlOperationBase implementation
////////////////////////////////////////////////////////

SqlOperationBase::SqlOperationBase(uint32 statement_id) : statement(statement_id), has_result(false)
{

}

SqlOperationBase::SqlOperationBase(uint32 statement_id, ACE_Future<SQL::ResultSet*> res): statement(statement_id), result(res), has_result(true)
{}

void SqlOperationBase::add_bool(uint8 index, bool value)
{
    if (statement_data.size() < index)
        statement_data.resize(index);

    statement_data[--index].data.boolean = value;
    statement_data[index].type = TYPE_BOOL;
}

void SqlOperationBase::add_uint8(uint8 index, uint8 value)
{
    if (statement_data.size() < index)
        statement_data.resize(index);

    statement_data[--index].data.u32 = value;
    statement_data[index].type = TYPE_U8;
}

void SqlOperationBase::add_uint16(uint8 index, uint16 value)
{
    if (statement_data.size() < index)
        statement_data.resize(index);

    statement_data[--index].data.u32 = value;
    statement_data[index].type = TYPE_U16;
}

void SqlOperationBase::add_uint32(uint8 index, uint32 value)
{
    if (statement_data.size() < index)
        statement_data.resize(index);

    statement_data[--index].data.u32 = value;
    statement_data[index].type = TYPE_U32;
}

void SqlOperationBase::add_uint64(uint8 index, uint64 value)
{
    if (statement_data.size() < index)
        statement_data.resize(index);

    statement_data[--index].data.u64 = value;
    statement_data[index].type = TYPE_U64;
}

void SqlOperationBase::add_float(uint8 index, float value)
{
    if (statement_data.size() < index)
        statement_data.resize(index);

    statement_data[--index].data.f = value;
    statement_data[index].type = TYPE_FLOAT;
}

void SqlOperationBase::add_string(uint8 index, const char* value)
{
    if (statement_data.size() < index)
        statement_data.resize(index);

    statement_data[--index].str.append(value);
    statement_data[index].type = TYPE_STRING;

}

int SqlOperationBase::execute()
{
    try {
        for (int i = 0; i < statement_data.size(); i++) {
            switch (statement_data[i].type) {
            case TYPE_BOOL:
                db->get_prep_stmt(this->statement)->setBool(i + 1, statement_data[i].data.boolean);
                break;
            case TYPE_U8:
            case TYPE_U16:
            case TYPE_U32:
                db->get_prep_stmt(this->statement)->setUint32(i + 1 , statement_data[i].data.u32);
                break;
            case TYPE_U64:
                db->get_prep_stmt(this->statement)->setUint64(i + 1, statement_data[i].data.u64);
                break;
            case TYPE_FLOAT:
                db->get_prep_stmt(this->statement)->setDouble(i + 1, statement_data[i].data.f);
                break;
            case TYPE_STRING:
                db->get_prep_stmt(this->statement)->setString(i + 1, statement_data[i].str);
                break;
            }
        }

        if (has_result) {
            SQL::ResultSet* result = db->get_prep_stmt(this->statement)->executeQuery();
            this->result.set(result);
        }
        else
            db->get_prep_stmt(this->statement)->execute();

        db->get_prep_stmt(this->statement)->clearParameters();
    }
    catch (SQL::SQLException &e) {
        ACE_ERROR((LM_ERROR,"Error while executing statement: %s \n", e.what()));
        return -1;
    }

    return 0;
}

////////////////////////////////////////////////////////
// SqlOperationRequest implementation
////////////////////////////////////////////////////////

SqlOperationRequest::SqlOperationRequest(uint32 statement_id) : SqlOperationBase(statement_id)
{

}

SqlOperationRequest::SqlOperationRequest(uint32 statement_id, ACE_Future<SQL::ResultSet*> res): SqlOperationBase(statement_id, res)
{

}

int SqlOperationRequest::call()
{
    if (execute() == -1)
        return -1;

    return 0;
}

////////////////////////////////////////////////////////
// SqlOperationBase implementation
////////////////////////////////////////////////////////

int DatabaseWorker::svc(void)
{
#if HAVE_MYSQL
  mysql_thread_init();
#endif
    if (!queue)
        return -1;

    SqlOperationBase* request;

    while (1) {
        request = (SqlOperationBase*)this->queue->dequeue();

        if (!request)
            break;

        request->set_database(this->db);
        request->call();
        delete request;
    }
#if HAVE_MYSQL
    mysql_thread_end();
#endif
    delete this;
}

};
};
