/*
 * test_sqlite.cpp
 *
 *  Created on: 23 juil. 2009
 *      Author: albator
 */


#include <DriverManager.h>
#include <Driver.h>
#include <Connection.h>
#include <SQLException.h>
#include <Statement.h>
#include <PreparedStatement.h>
#include <ResultSet.h>
#include <iostream>
#include <memory>

using namespace std;
using namespace DOR::SQL;

void testStatement(Connection* c)
{
    auto_ptr<Statement> s(c->createStatement());

    // test 1 - execute then getResultSet
    s->execute("SELECT `id`,`username`,`password` FROM `account`");
    auto_ptr<ResultSet> r(s->getResultSet());

    int i = 0;
    while (r->next())
    {
        i++;
        cout << "id: " << r->getUint32(1) << " username: "
             << r->getString(2) << " password: " << r->getString(3) << endl;
    }


    cout << "total row " << r->rowsCount() << endl;

    if (r->rowsCount() == i)
    {
        cout << "Test Statement 1 success" << endl;
    }
}

void testPreparedStatement(Connection* c)
{
    auto_ptr<PreparedStatement> s(c->prepareStatement("SELECT `id`,`username`,`password` FROM `account` WHERE `id` = ?"));
    cout << "param count : " << s->parameterCount() << endl;
    s->setUint32(1, 1);

    s->execute();
    auto_ptr<ResultSet> r(s->getResultSet());

    cout << "total row " << r->rowsCount() << endl;

    while (r->next())
    {
        cout << "id: " << r->getUint32(1) << " username: "
             << r->getString(2) << " password: " << r->getString(3) << endl;
    }

    if (r->rowsCount() > 0)
    {
        cout << "Test PreparedStatement 1 success" << endl;
    }

}

void testLoginServerStatement(Connection* c)
{
    auto_ptr<PreparedStatement> s1(c->prepareStatement("SELECT 1 FROM account WHERE username=?"));
    auto_ptr<PreparedStatement> s2(c->prepareStatement("SELECT locked FROM account WHERE username=?"));
    auto_ptr<PreparedStatement> s3(c->prepareStatement("SELECT id,password FROM account WHERE username=?"));
    auto_ptr<PreparedStatement> s4(c->prepareStatement("SELECT id,password,token FROM account WHERE username=?"));
    auto_ptr<PreparedStatement> s5(c->prepareStatement("SELECT id, token FROM account WHERE username=?"));
    auto_ptr<PreparedStatement> s6(c->prepareStatement("UPDATE account SET last_login=CURRENT_TIMESTAMP, online=1, last_ip=? WHERE id=?"));
    auto_ptr<PreparedStatement> s7(c->prepareStatement("UPDATE account SET token=? WHERE username=?"));

    // s1
    s1->setString(1,"albator");
    s1->execute();

    auto_ptr<ResultSet> r1(s1->getResultSet());
    if (!r1.get())
    {
        cout << "r1, no result" << endl;
    }
    else
    {
        while (r1->next())
        {
            bool res = r1->getBool(1);
            cout << "res = " << res << endl;
        }
    }
    cout << "Login server S1 test success" << endl;

    //s2
    s2->setString(1,"albator");
    s2->execute();

    auto_ptr<ResultSet> r2(s2->getResultSet());
    if (!r2.get())
    {
        cout << "r2, no result" << endl;
    }
    else
    {
        while (r2->next())
        {
            bool res = r2->getBool(1);
            cout << "locked = " << res << endl;
        }
    }
    cout << "Login server S2 test success" << endl;

    //s3
    s3->setString(1,"albator");
    s3->execute();

    auto_ptr<ResultSet> r3(s3->getResultSet());
    if (!r3.get())
    {
        cout << "r3, no result" << endl;
    }
    else
    {
        while (r3->next())
        {
            cout << "id = " << r3->getUint32(1) << " password = " << r3->getString(2) << endl;
        }
    }
    cout << "Login server S3 test success" << endl;

    //s4
    s4->setString(1,"albator");
    s4->execute();

    auto_ptr<ResultSet> r4(s4->getResultSet());
    if (!r4.get())
    {
        cout << "r4, no result" << endl;
    }
    else
    {
        while (r4->next())
        {
            cout << "id = " << r4->getUint32(1) << " password = " << r4->getString(2)
                 << " token = " << r4->getString(3) << endl;
        }
    }
    cout << "Login server S4 test success" << endl;

    //s5
    s5->setString(1,"albator");
    s5->execute();

    auto_ptr<ResultSet> r5(s5->getResultSet());
    if (!r5.get())
    {
        cout << "r5, no result" << endl;
    }
    else
    {
        while (r5->next())
        {
            cout << "id = " << r5->getUint32(1)
                 << " token = " << r5->getString(2) << endl;
        }
    }
    cout << "Login server S5 test success" << endl;

    //s6
    s6->setString(1,"127.0.0.1");
    s6->setUint32(2,1);
    if (!s6->execute())
    {
        cout << "update done" << endl;
        cout << "Login server S6 test success" << endl;
    }
    else
    {
        cout << "weird bug >< it should not return anything" << endl;
        cout << "Login server S6 test fail" << endl;
    }

    //s7
    s7->setString(1,"tokentoken");
    s7->setString(2,"albator");
    if (!s7->execute())
    {
        cout << "update done" << endl;
        cout << "Login server S7 test success" << endl;
    }
    else
    {
        cout << "weird bug >< it should not return anything" << endl;
        cout << "Login server S7 test fail" << endl;
    }






}

Connection* testConnect(Driver* d)
{

    Connection* c = d->connect("realmd.dat");

    if (!c)
        cout << "testConnect fail" << endl;
    else
        cout << "testConnect success" << endl;

    return c;
}

int main()
{
    // test driver manager
    cout << "Test Driver manager" << endl;

    try
    {
        Driver* d = DriverManager::getDriver("Sqlite");
        if (d)
        {
            cout << "test1 ok" << endl;

            delete d;

            d = DriverManager::getDriver("SQLITE");

            if (d)
            {
                cout << "test2 OK" << endl;
                Connection* c = testConnect(d);
                testStatement(c);
                testPreparedStatement(c);
                testLoginServerStatement(c);

                delete c;
                delete d;
            }
            else
                cout << "test2 fail" << endl;
        }
        else
            cout << "test1 fail" << endl;
    }
    catch (SQLException& e)
    {
        cout << "FATAL ERROR (exception) : " << e.what() << endl;
    }

    return 0;
}

/*
 * test_sqlite.cpp
 *
 *  Created on: 24 juil. 2009
 *      Author: albator
 */

