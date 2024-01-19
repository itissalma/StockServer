// database.h
#ifndef DATABASE_H
#define DATABASE_H

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h> 


class Database {
public:
    Database();
    ~Database();
    void connect();
    void closeConnection();
    bool isConnectionValid(sql::Connection *con);
    sql::Connection *getConnection();  // Add this method


private:
    sql::mysql::MySQL_Driver *driver;
    sql::Connection *con;
};

#endif // DATABASE_H
