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
 *  @file    Database.h
 *  @brief   Contains base clases that will be used to access database.
 *  @author  raczman <raczman@gmail.com>, albator
 *  @date    2009-07-02
 *
 */

#ifndef _DATABASE_H
#define _DATABASE_H

#include "Common.h"
#include "Callback.h"
#include <vector>
#include <string>
#include <queue>
#include <ace/Method_Request.h>
#include <ace/Future.h>
#include <ace/Activation_Queue.h>
#include <ace/Task.h>
#include "Log.h"

#include "Driver.h"
#include "Statement.h"
#include "ResultSet.h"
#include "Connection.h"
#include "SQLException.h"
#include "PreparedStatement.h"
#include "DriverManager.h"

namespace Morpheus
{

/**
* @brief Abstraction layer for accessing database.
* @details This namespace contains classes and structures
*          used while accessing database, regardless of its
*          storage backend. If you are interested in
*          actual connectors, please see Morpheus::SQL
*          namespace.
*/
namespace DatabaseAccess
{

enum DatabaseTaskPriority
{
    PRIORITY_LOW = 0,  //Priority 0 is default priority, so no point in setting it explicitly, but it's here for sake of being.
    PRIORITY_MEDIUM,
    PRIORITY_HIGH
};

class DatabaseConnection;

/**
 * @brief Because we pass our prepared statements request into queue,
 *        we do not know which connection will get us out of queue,
 *        so we need to bind our data at later stage.
 *        this union holds data to be binded
 */
union PreparedStatementDataUnion
{
    bool boolean;
    uint8 u8;
    uint16 u16;
    uint32 u32;
    uint64 u64;
    float  f;
};

/**
 * @brief This enum helps us to differ data held in union.
 */
enum PreparedStatementValueType
{
    TYPE_BOOL,
    TYPE_U8,
    TYPE_U16,
    TYPE_U32,
    TYPE_U64,
    TYPE_FLOAT,
    TYPE_STRING
};

/**
 * @brief This struct holds data and Value type.
 */
struct PreparedStatementData
{
    PreparedStatementDataUnion data;
    PreparedStatementValueType type;
    std::string str;
};

/**
 * @brief Abstract base class for Sql operation
 *
 */
class SqlOperationBase : public ACE_Method_Request
{
public:

    SqlOperationBase() {}

    /**
     * @brief Constructor
     * @param statement_id Takes statement index from statements enum.
     */
    SqlOperationBase(uint32 statement_id);

    /**
    * @brief Constructor - This one should be used if query returns a result.
    * @param statement_id statement index
    * @param res a single write multiple read sql::ResultSet pointer, to retrieve the data from query.
    */
    SqlOperationBase(uint32 statement_id, ACE_Future<Morpheus::SQL::ResultSet*> res);

    /**
     * @brief function called by worker thread
     */
    virtual int call() = 0;

    /**
     * @brief execute the sql operation
     * @return -1 in case of SQLException
     */
    int execute();

    /**
     * @brief Following functions are used to bind data into prepared statement.
     */
    void add_bool(uint8 index,bool value);
    void add_uint8(uint8 index, uint8 value);
    void add_uint16(uint8 index, uint16 value);
    void add_uint32(uint8 index, uint32 value);
    void add_uint64(uint8 index, uint64 value);
    void add_float(uint8 index, float value);
    void add_string(uint8 index, const char* value);

    /**
     * @brief Sets database that will be used
     *
     */
    void set_database(DatabaseConnection* new_db)
    {
        db = new_db;
    }

    /**
     * @brief Returns database used
     */
    DatabaseConnection* get_database()
    {
        return db;
    }

    /**
     * @brief Assign the specified result to the sql op.
     *
     */
    void set_result(ACE_Future<SQL::ResultSet*>& future)
    {
        result = future;
        has_result = true;
    }

    /**
     * @brief Returns the associated statement, mainly for debugging purposes.
     */
    uint32 get_statement_id()
    {
        return statement;
    }

    ACE_Future<SQL::ResultSet*> result;

    bool has_result;

protected:

    uint32 statement;
    DatabaseConnection* db;
    std::vector< PreparedStatementData> statement_data;
};

/**
 * @brief This class holds derives of DatabaseConnection class.
 *        Apart from holding DatabaseConnections, it also handles adding new asynch tasks
 *        into its activation queue.
 */
template <class T>
class DatabaseHolder
{

public:

    /**
     * @brief Constructor
     * @param num Number of DatabaseConnections (and worker threads) to spawn
     */
    DatabaseHolder(int num):
            queue(new ACE_Activation_Queue(new ACE_Message_Queue<ACE_MT_SYNCH>))
    {
        if (num < 1 || num > 32) {
            ACE_DEBUG((LM_ERROR,"Database connections set to incorrect value (%i), get a grip on reality man!\n", num));
            num = 1;
        }

        connections.resize(num);

        for (int i = 0; i < num; i++)
            connections[i] = new T(this->queue);
    }

    /**
     * @brief Opens up child connections, but first it checks provided data for correctness.
     * @param driverName Name of database engine to use. Currently supported are sqlite and mysql.
     * @param url Location of database, depends on driver.
     * @return true on success.
     */
    bool open(const std::string& driverName, const std::string& url)
    {
        // We test the provided data first, before passing it to our DatabaseConnection pool
        try {
            SQL::Driver *driver = SQL::DriverManager::getDriver(driverName);
            SQL::Connection* conn;

            conn = driver->connect(url);

            delete conn;

        }
        catch (SQL::SQLException &e) {
            ACE_ERROR((LM_ERROR,"Error while initialising database pool: %s \n",e.what()));
            return false;
        }


        for (int i = 0; i < connections.size(); i++)
            connections[i]->open(driverName, url);

        return true;
    }

    /**
     * @brief this function enqueues NULL to shut down all worker threads.
     */
    bool close()
    {
        for (int i = 0; i < connections.size(); i++)
            this->enqueue(NULL);

        return true;
    }

    /**
     * @brief Adds SqlOperationRequest to the queue
     */
    void enqueue(SqlOperationBase* sql_op)
    {
        ACE_Guard<ACE_Recursive_Thread_Mutex> g(this->mtx);
        this->queue->enqueue(sql_op);
    }

    /**
     * @brief Adds SqlOperationRequest to the queue
     *        Waits for the results and returns pointer to it.
     */
    SQL::ResultSet* enqueue_synch_query(SqlOperationBase* sql_op, DatabaseTaskPriority prio = PRIORITY_LOW)
    {
        SQL::ResultSet* result;
        ACE_Future<SQL::ResultSet*>* future_result;
        bool allocated = false;

        if (sql_op->has_result)
            future_result = &sql_op->result;
        else {
            future_result = new ACE_Future<SQL::ResultSet*>;
            sql_op->set_result(*future_result);
            allocated = true;
        }

        if (prio == PRIORITY_LOW)
            this->enqueue(sql_op);
        else
            this->enqueue_with_priority(sql_op, prio);

        future_result->get(result);
        if (allocated)
            delete future_result;
        return result;
    }

    void enqueue_with_priority(SqlOperationBase* sql_op, DatabaseTaskPriority prio)
    {
        sql_op->priority(prio);
        this->queue->enqueue(sql_op);
    }

private:

    ACE_Activation_Queue* queue;
    std::vector<T*> connections;
    ACE_Recursive_Thread_Mutex mtx;
};

class DatabaseConnection;

class DatabaseWorker : protected ACE_Task_Base
{

public:

    DatabaseWorker(ACE_Activation_Queue* new_queue, DatabaseConnection* conn):queue(new_queue), db(conn) { this->activate(); }
    int svc(void);
    int activate()
    {
        ACE_Task_Base::activate(THR_NEW_LWP |THR_DETACHED , 1);
        return 0;
    }

private:

    DatabaseWorker():ACE_Task_Base() {}
    ACE_Activation_Queue* queue;
    DatabaseConnection* db;
};

class DatabaseConnection
{

public:

    /**
     * @brief Open connection to database.
     * @return true on success, false otherwise.
     */
    virtual bool open(const std::string& driverName, const std::string& url) = 0;

    /**
     * @brief Ctor. When creating DatabaseConnection we'd like to immediately get pointer to queue with tasks.
     */
    DatabaseConnection(ACE_Activation_Queue* queue):query_queue(queue) {}

    /**
     * @brief Statement Holder accessor. When used with care it should return us prep stmt we want to use.
     */
    SQL::PreparedStatement* get_prep_stmt(int index)
    {
        return statement_holder[index];
    }

    /**
     * @brief Rollback the current transaction. IMPORTANT: Don't forget that DB engine MUST support transactions to use this.
     * InnoDB does, MyISAM doesn't.
     */
    void rollback_transaction()
    {
        this->connection->rollback();
        //this->connection->setAutoCommit(true);
    }

    /**
     * @brief Commit the current transaction. IMPORTANT: Don't forget that DB engine MUST support transactions to use this.
     * InnoDB does, MyISAM doesn't.
     */
    void commit_transaction()
    {
        this->connection->commit();
        //this->connection->setAutoCommit(true);
    }

    bool getAutoCommit()
    {
        return connection->getAutoCommit();
    }

    void setAutoCommit(bool autoCommit)
    {
        connection->setAutoCommit(autoCommit);
    }

protected:

    /**
     * @brief We do not want this ctor to be called.
     */
    DatabaseConnection();

    /**
     * @brief Pointer to query queue which is shared across several
     */
    ACE_Activation_Queue* query_queue;

    /**
     * @brief Each connection has it's own holder with prepared statements.
     */
    std::vector< Morpheus::SQL::PreparedStatement* > statement_holder;
    Morpheus::SQL::Driver* driver;
    Morpheus::SQL::Connection* connection;

    /**
     * @brief Worker thread that executes queries in queue.
     */
    DatabaseWorker* worker;
};

/**
 * @brief Accessing database is done through this class.
 *        Given index value of prepared statement and data to
 *        be used we can enqueue the queries to be done asynchronously.
 */
class SqlOperationRequest : public SqlOperationBase
{
public:

    /**
     * @brief Constructor
     * @param statement_id Takes statement index from statements enum.
     */
    SqlOperationRequest(uint32 statement_id);

    /**
     * @brief Constructor - This one should be used if query returns a result.
     * @param statement_id statement index
     * @param res a single write multiple read SQL::ResultSet pointer, to retrieve the data from query.
     */
    SqlOperationRequest(uint32 statement_id, ACE_Future<SQL::ResultSet*> res);

    /**
     * @brief Worker threads of DB connection call this function.
     */
    int call();

};

/**
 * @brief Execute several queries in a row, in the given order, and
 *        revert all queries if one fails for some reason. Autocommit is restored
 *        in its initial state after the transaction is done.
 */
class SqlOperationTransaction : public SqlOperationBase
{
public:

    /**
     * @brief Constructor
     */
    SqlOperationTransaction()
    {
        //autocommit_state = db->getAutoCommit();
        printf("Beginning transaction.\n");
    }

    /**
     * @brief Destructor. Restore previous state of autocommit.
     */
    virtual ~SqlOperationTransaction()
    {
        for (int i = 0; i < requests.size(); i++) {
            SqlOperationRequest* req = requests.front();
            delete req;
            requests.pop();
        }

        db->setAutoCommit(autocommit_state);
    }

    /**
     * @brief Append the statement to the transaction.
     */
    void append(SqlOperationRequest* req)
    {
        requests.push(req);
    }

    /**
     * @brief Worker threads of DB connection call this function.
     */
    int call()
    {
        autocommit_state = db->getAutoCommit();
        db->setAutoCommit(false);
        bool error = false;
        uint32 size = requests.size();
        for (int i = 0; i < size; i++) {
            SqlOperationRequest* req = requests.front();

            req->set_database(this->get_database());
            if (req->call() == -1) {
                error = true;
                break;
            }

            delete req;
            requests.pop();
        }

        if (error) {
            sLog->outError(LOG_FILTER_DB, "Error in transaction, rolling back...\n");
            db->rollback_transaction();
            return -1;
        }
        else {
            sLog->outDetail(LOG_FILTER_DB, "Transaction successful, committing\n");
            db->commit_transaction();
            return 0;
        }

        return 0;
    }

private:

    bool autocommit_state;
    std::queue<SqlOperationRequest*> requests;
};

/**
 * @brief Observer base class
 *
 */
template<class C, typename T>
class SqlOperationObserver : public ACE_Future_Observer<SQL::ResultSet*>
{

public:

    SqlOperationObserver(Callback<C,T> method) : callback(method)
    {

    }

    virtual void update(const ACE_Future< SQL::ResultSet* > &future)=0;

protected:

    Callback<C,T> callback;
};

};
};

#endif
