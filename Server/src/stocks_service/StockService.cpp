#include "StockService.h"
#include <iostream>

StockService::StockService(): stopServiceThread(false), stopUpdateThread(false) {
    database.connect();

    // Initialize random seed
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Start the service thread
    std::thread serviceThread(&StockService::service_thread, this);

    // Start the update thread
    std::thread updateThread(&StockService::updateStockPricesPeriodically, this);

    serviceThread.detach();
    updateThread.detach();

}

StockService::~StockService() {
    // Cleanup or shutdown logic 
}


void StockService::updateStockPrices(int stockId, double percentageChange) {
    // Get a connection to the database
    sql::Connection *con = database.getConnection();

    if (!con) {
        std::cerr << "Error: Database connection is null." << std::endl;
        return;
    }

    // Get price of stock first and see if after the percetntage change it is not negative
    std::string query = "SELECT Price FROM Stock WHERE StockID = ?";

    sql::PreparedStatement *prepStmt = con->prepareStatement(query);

    // Set the parameters for the prepared statement
    prepStmt->setInt(1, stockId);

    // Execute the query
    sql::ResultSet *res = prepStmt->executeQuery();

    // Get the price of the stock
    double price = 0;
    if (res->next()) {
        price = res->getDouble("Price");
    }

    // Check if the price is not negative
    if (price * (1 + percentageChange) < 0) {
        // If it is negative, set the price to 0.1
        percentageChange = 0.1 / price - 1;
    }

    // Calculate the new price based on the percentage change
    std::string query2 = "UPDATE Stock SET Price = Price * (1 + ?) WHERE StockID = ?";
    sql::PreparedStatement *prepStmt2 = con->prepareStatement(query2);

    // Set the parameters for the prepared statement
    prepStmt2->setDouble(1, percentageChange);
    prepStmt2->setInt(2, stockId);

    // Execute the query
    prepStmt2->execute();
}


double StockService::generateRandomPercentageChange(double minChange, double maxChange) {
    // Generate a random value between -Z and Z
    double randomChange = ((double)rand() / RAND_MAX) * (maxChange - minChange) + minChange;
    return randomChange;
}

void StockService::updateStockPricesBasedOnDemand(int stockId, double demandFactor) {
    // Generate a random percentage change based on the demand factor
    //double percentageChange = generateRandomPercentageChange(-demandFactor, demandFactor);

    // Update stock prices based on demand
    updateStockPrices(stockId, demandFactor);
}

void StockService::updateStockPricesPeriodically() {
    while (!stopServiceThread) {
        // Update stock prices based on simulated demand for each stock
        adjustStockPricesBasedOnDemand();

        // Sleep for N seconds before the next update
        std::this_thread::sleep_for(std::chrono::seconds(10)); // Change 10 to the desired interval
    }
    while (!stopUpdateThread) {
        // Add a delay or other logic if needed
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void StockService::adjustStockPricesBasedOnDemand() {
    // Get a connection to the database
    sql::Connection *con = database.getConnection();

    if (!con) {
        std::cerr << "Error: Database connection is null." << std::endl;
        return;
    }

    // Example: Calculate net quantity for each stock
    std::string query = "SELECT StockID, SUM(CASE WHEN Status = 0 THEN Quantity ELSE -Quantity END) AS NetQuantity "
                        "FROM Orders GROUP BY StockID";

    sql::PreparedStatement *prepStmt = con->prepareStatement(query);
    sql::ResultSet *result = prepStmt->executeQuery();

    while (result->next()) {
        int stockId = result->getInt("StockID");
        int netQuantity = result->getInt("NetQuantity");

        // Adjust stock prices based on net quantity 
        //double demandFactor = static_cast<double>(netQuantity) / 400.0;
        double percentageChange = generateRandomPercentageChange(-0.1, 0.1);
        updateStockPricesBasedOnDemand(stockId, percentageChange);
    }
}

void StockService::handleGetStocks(web::http::http_request request) {
    // Get a connection to the database
    sql::Connection *con = database.getConnection();

    if (!con) {
        std::cerr << "Error: Database connection is null." << std::endl;
        request.reply(web::http::status_codes::InternalError);
        return;
    }

    // Prepare a SQL statement to fetch stocks from the Stock table
    std::string query = "SELECT * FROM Stock";
    sql::PreparedStatement *prepStmt = con->prepareStatement(query);

    // Execute the query
    sql::ResultSet *result = prepStmt->executeQuery();

    // Construct the JSON response
    web::json::value jsonResponse;
    auto& stocksArray = jsonResponse[U("stocks")];

    // Iterate through the result set and add each stock to the response
    while (result->next()) {
        web::json::value stock;
        stock[U("id")] = web::json::value::number(result->getInt("StockID"));
        stock[U("name")] = web::json::value::string(result->getString("stockName"));
        stock[U("price")] = web::json::value::number(static_cast<double>(result->getDouble("Price")));

        stocksArray[stocksArray.size()] = stock;
    }

    // Convert JSON to string
    std::string jsonString = utility::conversions::to_utf8string(jsonResponse.serialize());

    // Set the response body with the JSON string
    request.reply(web::http::status_codes::OK, jsonString, "application/json");
}

void StockService::handleGetOwnedStocks(web::http::http_request request, std::string email){
    try{
        // Get a connection to the database
        sql::Connection *con = database.getConnection();

        if (!con) {
            std::cerr << "Error: Database connection is null." << std::endl;
            request.reply(web::http::status_codes::InternalError);
            return;
        }

        // Prepare a SQL statement to fetch stocks from the Stock table
        std::string query = R"(
            WITH StockQuantities AS (
                SELECT
                    StockID,
                    SUM(CASE WHEN Status = 0 THEN Quantity ELSE 0 END) -
                    SUM(CASE WHEN Status = 1 THEN Quantity ELSE 0 END) AS NetQuantity
                FROM
                    Orders
                WHERE
                    Email = ?
                GROUP BY
                    StockID
                HAVING
                    NetQuantity > 0
            )

            SELECT
                SQ.StockID,
                SQ.NetQuantity,
                SI.StockName,
                SI.Price
            FROM
                StockQuantities SQ
            JOIN
                Stock SI ON SQ.StockID = SI.StockID
        )";

        sql::PreparedStatement *prepStmt = con->prepareStatement(query);

        // Set the parameters for the prepared statement
        prepStmt->setString(1, email);

        // Execute the query
        sql::ResultSet *result = prepStmt->executeQuery();

        // Check if query was successful
        if (!result) {
            std::cerr << "Error: Query failed." << std::endl;
            request.reply(web::http::status_codes::InternalError);
            return;
        }

        // Construct the JSON response
        web::json::value jsonResponse;
        auto& stocksArray = jsonResponse[U("stocks")];

        // Iterate through the result set and add each stock to the response
        while (result->next()) {
            web::json::value stock;
            stock[U("id")] = web::json::value::number(result->getInt("StockID"));
            stock[U("name")] = web::json::value::string(result->getString("StockName"));
            stock[U("price")] = web::json::value::number(static_cast<double>(result->getDouble("Price")));
            stock[U("quantity")] = web::json::value::number(result->getInt("NetQuantity"));

            stocksArray[stocksArray.size()] = stock;
        }
        std::cout << "Stocks array size: " << stocksArray.size() << std::endl;

        // Convert JSON to string
        std::string jsonString = utility::conversions::to_utf8string(jsonResponse.serialize());

        // Set the response body with the JSON string
        request.reply(web::http::status_codes::OK, jsonString, "application/json");
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        request.reply(web::http::status_codes::InternalError);
    }
}

void StockService::handleGetOrderHistory(web::http::http_request request, std::string email){
    try{
        // Get a connection to the database
        sql::Connection *con = database.getConnection();

        if (!con) {
            std::cerr << "Error: Database connection is null." << std::endl;
            request.reply(web::http::status_codes::InternalError);
            return;
        }

        // Prepare a SQL statement to fetch stocks from the Stock table
        std::string query = R"(
            SELECT
                Orders.OrderID,
                Stock.StockName,
                Orders.Quantity,
                Orders.Price,
                Orders.Status
            FROM
                Orders
            JOIN
                Stock ON Orders.StockID = Stock.StockID
            WHERE
                Orders.Email = ?
            ORDER BY
                Orders.OrderID DESC;   
            )";

    sql::PreparedStatement *prepStmt = con->prepareStatement(query);

    // Set the parameters for the prepared statement
    prepStmt->setString(1, email);

    // Execute the query
    sql::ResultSet *result = prepStmt->executeQuery();

    // Check if query was successful
    if (!result) {
        std::cerr << "Error: Query failed." << std::endl;
        request.reply(web::http::status_codes::InternalError);
        return;
    }

    // Construct the JSON response
    web::json::value jsonResponse;
    auto& stocksArray = jsonResponse[U("stocks")];

    // Iterate through the result set and add each stock to the response
    while (result->next()) {
        web::json::value stock;
        stock[U("id")] = web::json::value::number(result->getInt("OrderID"));
        stock[U("name")] = web::json::value::string(result->getString("StockName"));
        stock[U("price")] = web::json::value::number(static_cast<double>(result->getDouble("Price")));
        stock[U("quantity")] = web::json::value::number(result->getInt("Quantity"));
        stock[U("status")] = web::json::value::number(result->getInt("Status"));

        stocksArray[stocksArray.size()] = stock;
    }
    std::cout << "Stocks array size: " << stocksArray.size() << std::endl;

    // Convert JSON to string
    std::string jsonString = utility::conversions::to_utf8string(jsonResponse.serialize());

    // Set the response body with the JSON string
    request.reply(web::http::status_codes::OK, jsonString, "application/json");
} catch (const std::exception& ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    request.reply(web::http::status_codes::InternalError);
}

}
//create a function that sells a stock by creating an insert sql query that Contains an orderID which it has to create by taking the last order and doing orderid+1, stockid, price, quantity, email, and status which is 0 for bought and 1 for sold
void StockService::handleBuyStocks(web::http::http_request request) {
    int stockId = 0;
    std::string stockName = "";
    double stockPrice = 0;
    std::string userEmail = "";
    int quantity = 0;
    int orderID = 0;
    int status = 0;

    try {
        auto body = request.extract_string().get();
        // Get the JSON body from the request
        rapidjson::Document document;
        document.Parse(body.c_str());

        if (document.HasParseError()) {
            std::cerr << "Error parsing JSON: " << rapidjson::GetParseError_En(document.GetParseError()) << std::endl;
            request.reply(web::http::status_codes::BadRequest);
            return;
        }

        const rapidjson::Value& stockObject = document["stock"];
        if (stockObject.IsObject()) {
            stockId = stockObject["id"].GetInt();
            stockName = stockObject["name"].GetString();

            const rapidjson::Value& priceValue = stockObject["price"];
            if (priceValue.IsString()) {
                // Convert the string to a double
                try {
                    stockPrice = std::stod(priceValue.GetString());
                } catch (const std::exception& ex) {
                    std::cerr << "Error converting 'price' field to double: " << ex.what() << std::endl;
                    request.reply(web::http::status_codes::BadRequest);
                    return;
                }
            } else if (priceValue.IsNumber()) {
                stockPrice = priceValue.GetDouble();
            } else {
                std::cerr << "Error: 'price' field is not a valid number or string." << std::endl;
                request.reply(web::http::status_codes::BadRequest);
                return;
            }

            userEmail = document["email"].GetString();
            quantity = document["quantity"].GetInt();

            std::cout << "stockId: " << stockId << std::endl;
            std::cout << "stockName: " << stockName << std::endl;
            std::cout << "stockPrice: " << stockPrice << std::endl;
            std::cout << "userEmail: " << userEmail << std::endl;
            std::cout << "quantity: " << quantity << std::endl;

            // Perform the logic to buy stocks (e.g., update database, process the purchase)
            sql::Connection *con = database.getConnection();

            if (!con) {
                std::cerr << "Error: Database connection is null." << std::endl;
                request.reply(web::http::status_codes::InternalError);
                return;
            }

            // Insert into Orders table which Contains an orderID which it has to create by taking the last order and doing orderid+1, stockid, price, quantity, email, and status which is 0 for bought and 1 for sold
            std::string query = "INSERT INTO Orders (OrderID, StockID, Price, Quantity, Email, UserNatId, Status) "
                        "SELECT COALESCE(MAX(OrderID), 0) + 1, ?, ?, ?, ?, NULL, 0 FROM Orders";

            sql::PreparedStatement *prepStmt = con->prepareStatement(query);

            // Set the parameters for the prepared statement
            prepStmt->setInt(1, stockId);
            prepStmt->setDouble(2, stockPrice);
            prepStmt->setInt(3, quantity);
            prepStmt->setString(4, userEmail);

            // Execute the query
            sql::ResultSet *result = prepStmt->executeQuery();

            //print the query result
            std::cout << "Query result is " << result << std::endl;

            // Send back to the post request that the order was successful if the query was successful
            if (result) {
                std::cout << "Order was successful" << std::endl;
                request.reply(web::http::status_codes::OK);
                updateStockPricesBasedOnDemand(stockId, 0.1*quantity);
            } else {
                std::cerr << "Error: 'stock' field is not an object." << std::endl;
                request.reply(web::http::status_codes::InternalError);
            }
        } else {
            std::cerr << "Error: 'stock' field is not an object." << std::endl;
            request.reply(web::http::status_codes::BadRequest);
            return;
        }
    } catch (const web::http::http_exception& e) {
        std::cerr << "HTTP error in handlePostBuyStocks: " << e.what() << std::endl;
        request.reply(web::http::status_codes::InternalError);
    } catch (const std::exception& ex) {
        std::cerr << "Error in handlePostBuyStocks: " << ex.what() << std::endl;
        request.reply(web::http::status_codes::InternalError);
    }
}

void StockService::handleSellStocks(web::http::http_request request) {
    int stockId = 0;
    double stockPrice = 0;
    std::string userEmail = "";
    int quantity = 0;
    int orderID = 0;
    int status = 0;
    try{
        std::cout<< "In handleSellStocks" << std::endl;

        auto body = request.extract_string().get();
        // Get the JSON body from the request
        rapidjson::Document document;
        document.Parse(body.c_str());

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);

        std::cout << "The document received is " << buffer.GetString() << std::endl;

        if (document.HasParseError()) {
            std::cerr << "Error parsing JSON: " << rapidjson::GetParseError_En(document.GetParseError()) << std::endl;
            request.reply(web::http::status_codes::BadRequest);
            return;
        }

        userEmail = document["email"].GetString();
        std::cout << "userEmail: " << userEmail << std::endl;
        quantity = document["quantity"].GetInt();
        stockId = document["ownedStockId"].GetInt();
        stockPrice = document["price"].GetDouble();

        std::cout << "stockId: " << stockId << std::endl;
        std::cout << "stockPrice: " << stockPrice << std::endl;
        std::cout << "userEmail: " << userEmail << std::endl;
        std::cout << "quantity: " << quantity << std::endl;

        // Perform the logic to buy stocks (e.g., update database, process the purchase)
        sql::Connection *con = database.getConnection();

        if (!con) {
            std::cerr << "Error: Database connection is null." << std::endl;
            request.reply(web::http::status_codes::InternalError);
            return;
        }

        // Insert into Orders table which Contains an orderID which it has to create by taking the last order and doing orderid+1, stockid, price, quantity, email, and status which is 0 for bought and 1 for sold
        std::string query = "INSERT INTO Orders (OrderID, StockID, Price, Quantity, Email, UserNatId, Status) "
                    "SELECT COALESCE(MAX(OrderID), 0) + 1, ?, ?, ?, ?, NULL, 1 FROM Orders";

        sql::PreparedStatement *prepStmt = con->prepareStatement(query);

        // Set the parameters for the prepared statement
        prepStmt->setInt(1, stockId);
        prepStmt->setDouble(2, stockPrice);
        prepStmt->setInt(3, quantity);
        prepStmt->setString(4, userEmail);

        // Execute the query
        sql::ResultSet *result = prepStmt->executeQuery();

        //print the query result
        std::cout << "Query result is " << result << std::endl;

        // Send back to the post request that the order was successful if the query was successful
        if (result) {
            std::cout << "Order was successful" << std::endl;
            updateStockPricesBasedOnDemand(stockId, -0.1*quantity);
            request.reply(web::http::status_codes::OK);
        } else {
            std::cerr << "Error: 'stock' field is not an object." << std::endl;
            request.reply(web::http::status_codes::InternalError);
        }
    } catch (const web::http::http_exception& e) {
        std::cerr << "HTTP error in handlePostBuyStocks: " << e.what() << std::endl;
        request.reply(web::http::status_codes::InternalError);
    } catch (const std::exception& ex) {
        std::cerr << "Error in handlePostBuyStocks: " << ex.what() << std::endl;
        request.reply(web::http::status_codes::InternalError);
    }
}

void StockService::service_thread() {
    try {
        // Create an HTTP listener for handling requests
        web::http::experimental::listener::http_listener listener(U("http://localhost:8085"));

        listener.support(web::http::methods::GET, [this](web::http::http_request request) {
            auto path = request.request_uri().path();
            if (path == U("/stocks")) {
                std::cout << "Received GET request for stocks" << std::endl;
                handleGetStocks(request);
            } else if (path == U("/ownedStocks")) {
                std::cout << "Received GET request for owned stocks" << std::endl;
                auto query = request.request_uri().query();
                std::string email = query.substr(6);
                std::cout << "Email is " << email << std::endl;
                handleGetOwnedStocks(request, email);
            } else if(path == U("/orderHistory")){
                std::cout << "Received GET request for order history" << std::endl;
                auto query = request.request_uri().query();
                std::string email = query.substr(6);
                std::cout << "Email is " << email << std::endl;
                handleGetOrderHistory(request, email);
            }
            else {
                std::cout << "Received unsupported path: " << path << std::endl;
            }
        });

        listener.support(web::http::methods::POST, [this](web::http::http_request request) {
            auto path = request.request_uri().path();
            if (path == U("/stocks/buy")) {
                std::cout << "Received POST request for buying stocks" << std::endl;
                handleBuyStocks(request);
            } else if (path == U("/stocks/sell")) {
                std::cout << "Received POST request for selling stocks" << std::endl;
                handleSellStocks(request);
            } else {
                std::cout << "Received unsupported path: " << path << std::endl;
            }
        });

        listener.open().then([this](pplx::task<void> t) {
            try {
                // Check for exceptions in the preceding task
                t.get();
                std::cout << "Listening for requests..." << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Error opening listener: " << e.what() << std::endl;
            }
        }).wait();



        while (!stopServiceThread) {
            // Add logic or sleep if needed
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        listener.close().wait();
    } catch (web::http::http_exception const &e) {
        std::cout << "HTTP error in StockService: " << e.what() << std::endl;
    }
}
