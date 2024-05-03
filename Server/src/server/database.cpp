#include "database.h"

Database::Database() : driver(nullptr), con(nullptr) {}

Database::~Database() {
    closeConnection();
}

bool Database::isConnectionValid(sql::Connection *con) {
    return con && con->isValid();
}

void Database::connect() {
    driver = sql::mysql::get_mysql_driver_instance();

    // Replace these with your actual database credentials
    std::string host = "192.168.1.3";
    int port = 3306;
    std::string user = "root";
    std::string password = "password";
    std::string database = "stockMarket";

    con = driver->connect("tcp://" + host + ":" + std::to_string(port), user, password);

    if (isConnectionValid(con)) {
        std::cout << "Connected to the database successfully!" << std::endl;
        con->setSchema(database);
    } else {
        std::cerr << "Failed to connect to the database." << std::endl;
        // Handle the failure, throw an exception, or exit the program as needed.
    }
}

sql::Connection *Database::getConnection() {
    return con;
}

void Database::closeConnection() {
    if (con) {
        con->close();
        delete con;
    }
}
