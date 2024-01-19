// MyServer.cpp
#include "MyServer.h"
#include "../stocks_service/StockService.h"
#include <iostream>

MyServer::MyServer() : stopServerThread(false) {
    // Initialize the server
    echo_server.init_asio();

    database.connect();

    std::cout << "Server initialized" << std::endl;

    // Register the message handler
    echo_server.set_message_handler(std::bind(&MyServer::on_message, this, std::placeholders::_1, std::placeholders::_2));

    std::cout << "Message handler registered" << std::endl;

    // Set up CORS headers
    echo_server.set_http_handler([this](websocketpp::connection_hdl hdl) {
        server::connection_ptr con = echo_server.get_con_from_hdl(hdl);
        con->append_header("Access-Control-Allow-Origin", "*");
        con->append_header("Access-Control-Allow-Methods", "GET, POST");
        con->append_header("Access-Control-Allow-Headers", "Content-Type");
    });

    // Listen on port 9062
    echo_server.listen(9062);

    std::cout << "Database connection status: " << (database.isConnectionValid(database.getConnection()) ? "Connected" : "Not Connected") << std::endl;

    // Start the server accept loop
    echo_server.start_accept();

    echo_server.set_close_handler(std::bind(&MyServer::on_close, this, std::placeholders::_1));
}


MyServer::~MyServer() {
    // Perform cleanup or shutdown logic if needed
    //database.closeConnection();
}

void MyServer::on_close(websocketpp::connection_hdl hdl) {
    server::connection_ptr con = echo_server.get_con_from_hdl(hdl);
    std::cout << "Connection closed: " << con->get_ec().message() << std::endl;
}

void MyServer::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    std::cout << "First stop: Received message: " << msg->get_payload() << std::endl;
    try {
        std::cout << "Received message: " << msg->get_payload() << std::endl;

        // Parse the JSON message
        rapidjson::Document document;

        // Check if the payload is not empty
        if (msg->get_payload().empty()) {
            std::cerr << "Error: Empty message payload." << std::endl;
            return;
        }

        document.Parse(msg->get_payload().c_str());

        // Check if the parsing was successful
        if (document.HasParseError()) {
            std::cerr << "Error parsing JSON message. Parse error code: " << document.GetParseError() << std::endl;
            return;
        }

        // Check if the message has a "type" field
        if (document.HasMember("type") && document["type"].IsString()) {
            std::string messageType = document["type"].GetString();

            // Handle different message types
            if (messageType == "login") {
                std::cout << "login message received is " << msg->get_payload() << std::endl;
                handle_login(hdl, document);
            } else if (messageType == "signup") {
                handle_signup(hdl, document); // Added handling for signup
            }else if (messageType == "getStocks") {
                //make an api call to the stocks_service to get the stocks
                web::http::http_request request;
                getStocksFromService(request, hdl);
            }else if(messageType == "buy"){
                //make an api call to the stocks_service to buy the stocks
                web::http::http_request request;
                buyStocksFromService(request, hdl, document);
            } else if(messageType == "getOwnedStocks"){
                web::http::http_request request;
                getOwnedStocksFromService(request, hdl, document);                
            } else if(messageType == "sell"){
                web::http::http_request request;
                sellStocksFromService(request, hdl, document);
            } else if(messageType == "getOrderHistory"){
                web::http::http_request request;
                getOrderHistoryFromService(request, hdl, document);
            }
            else if (messageType == "other_type") {
                // Call another function for "other_type"
                // handle_other_type(hdl, document);
            } else {
                std::cerr << "Unknown message type: " << messageType << std::endl;
            }
        } else {
            std::cerr << "Message does not have a 'type' field." << std::endl;
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error handling message: " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "An unknown error occurred while handling the message." << std::endl;
    }
}


void MyServer::handle_login(websocketpp::connection_hdl hdl, const rapidjson::Document& document) {
    if (document.HasMember("email") && document["email"].IsString() &&
        document.HasMember("password") && document["password"].IsString()) {
        
        std::string email = document["email"].GetString();
        std::string password = document["password"].GetString();

        // Replace this with your actual authentication logic
        if (is_valid_credentials(email, password)) {
            // Send a success message to the client
            rapidjson::Document response;
            response.SetObject();
            response.AddMember("type", "login", response.GetAllocator());
            response.AddMember("success", true, response.GetAllocator());

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            response.Accept(writer);

            echo_server.send(hdl, buffer.GetString(), websocketpp::frame::opcode::text);
        } else {
            // Send a failure message to the client
            rapidjson::Document response;
            response.SetObject();
            response.AddMember("type", "login", response.GetAllocator());
            response.AddMember("success", false, response.GetAllocator());

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            response.Accept(writer);

            echo_server.send(hdl, buffer.GetString(), websocketpp::frame::opcode::text);
        }
    }
}

void MyServer::handle_signup(websocketpp::connection_hdl hdl, const rapidjson::Document& document) {
    if (document.HasMember("email") && document["email"].IsString() &&
        document.HasMember("password") && document["password"].IsString() &&
        document.HasMember("username") && document["username"].IsString() &&
        document.HasMember("nationalId") && document["nationalId"].IsString()) {

        std::string email = document["email"].GetString();
        std::string password = document["password"].GetString();
        std::string username = document["username"].GetString();
        std::string nationalId = document["nationalId"].GetString();

        // Replace this with your actual signup logic
        if (perform_signup(email, password, username, nationalId)) {
            // Send a success message to the client
            rapidjson::Document response;
            response.SetObject();
            response.AddMember("type", "signup", response.GetAllocator());
            response.AddMember("success", true, response.GetAllocator());

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            response.Accept(writer);

            echo_server.send(hdl, buffer.GetString(), websocketpp::frame::opcode::text);
        } else {
            // Send a failure message to the client
            rapidjson::Document response;
            response.SetObject();
            response.AddMember("type", "signup", response.GetAllocator());
            response.AddMember("success", false, response.GetAllocator());

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            response.Accept(writer);

            echo_server.send(hdl, buffer.GetString(), websocketpp::frame::opcode::text);
        }
    }
}

bool MyServer::perform_signup(const std::string& email, const std::string& password, const std::string& username, const std::string& nationalId) {
    try {
        // Get a connection to the database
        sql::Connection *con = database.getConnection();

        if (!con) {
            std::cerr << "Error: Database connection is null." << std::endl;
            return false;
        }

        // Prepare a SQL statement to insert a new user
        std::string query = "INSERT INTO User (Email, userPassword, userName, NatID) VALUES (?, ?, ?, ?)";
        sql::PreparedStatement *prepStmt = con->prepareStatement(query);
        prepStmt->setString(1, email);
        prepStmt->setString(2, password);
        prepStmt->setString(3, username);
        prepStmt->setString(4, nationalId);

        // Execute the query
        prepStmt->executeUpdate();

        // Cleanup
        delete prepStmt;

        return true;

    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return false;
    }
}

bool MyServer::is_valid_credentials(const std::string& email, const std::string& password) {
    try {
        // Get a connection to the database
        sql::Connection *con = database.getConnection();
        
        if (!con) {
            std::cerr << "Error: Database connection is null." << std::endl;
            return false;
        }

        std::cout<<"before here"<<std::endl;

        // Prepare a SQL statement to check if the user exists
        std::string query = "SELECT COUNT(*) FROM User WHERE Email = ? AND userPassword = ?";
        sql::PreparedStatement *prepStmt = con->prepareStatement(query);
        prepStmt->setString(1, email);
        prepStmt->setString(2, password);
        std::cout<<"after here"<<std::endl;

        // Execute the query
        sql::ResultSet *result = prepStmt->executeQuery();

        std::cout<<"is_valid_credentials"<<std::endl;
        // Check the result
        if (result->next() && result->getInt(1) > 0) {
            // User exists
            std::cout<<"User exists"<<std::endl;
            delete result;
            delete prepStmt;
            return true;
        }

        // User does not exist
        delete result;
        delete prepStmt;
        return false;

    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return false;
    }
}

void MyServer::getStocksFromService(web::http::http_request request, websocketpp::connection_hdl hdl) {
try {
    // Define the thread function
    auto threadFunction = [this, hdl]() {
        try {
            while (true) {
                // Create an HTTP client
                web::http::client::http_client client(U("http://localhost:8085"));

                // Make a GET request to the StocksService
                auto response = client.request(web::http::methods::GET, U("/stocks")).get();

                if (response.status_code() == web::http::status_codes::OK) {
                    auto jsonValue = response.extract_json().get();

                    rapidjson::Document responseMessage;
                    responseMessage.SetObject();
                    responseMessage.AddMember("type", "getStocks", responseMessage.GetAllocator());

                    rapidjson::Value stocksValue(rapidjson::kArrayType);

                    auto stocksArray = jsonValue[U("stocks")].as_array();

                    for (const auto& stock : stocksArray) {
                        rapidjson::Value stockValue(rapidjson::kObjectType);
                        stockValue.AddMember("id", stock.at(U("id")).as_integer(), responseMessage.GetAllocator());
                        const std::string nameString = utility::conversions::to_utf8string(stock.at(U("name")).as_string());
                        stockValue.AddMember("name", rapidjson::Value().SetString(nameString.c_str(), responseMessage.GetAllocator()), responseMessage.GetAllocator());
                        stockValue.AddMember("price", stock.at(U("price")).as_double(), responseMessage.GetAllocator());

                        stocksValue.PushBack(stockValue, responseMessage.GetAllocator());
                    }

                    responseMessage.AddMember("stocks", stocksValue, responseMessage.GetAllocator());

                    rapidjson::StringBuffer buffer;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    responseMessage.Accept(writer);

                    echo_server.send(hdl, buffer.GetString(), websocketpp::frame::opcode::text);
                } else {
                    // Handle the error directly without involving handle_stocks_request
                    rapidjson::Document errorMessage;
                    errorMessage.SetObject();
                    errorMessage.AddMember("type", "error", errorMessage.GetAllocator());
                    errorMessage.AddMember("message", "Failed to get stocks from StocksService", errorMessage.GetAllocator());

                    rapidjson::StringBuffer errorBuffer;
                    rapidjson::Writer<rapidjson::StringBuffer> errorWriter(errorBuffer);
                    errorMessage.Accept(errorWriter);

                    echo_server.send(hdl, errorBuffer.GetString(), websocketpp::frame::opcode::text);
                }

                // Wait for 1 seconds before making the next request
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        } catch (const std::exception& ex) {
            // Handle errors here
            std::cerr << "Error: " << ex.what() << std::endl;
        }
    };

    // Start the thread
    std::thread(threadFunction).detach();
} catch (const std::exception& ex) {
    // Handle the error directly without involving handle_stocks_request
    rapidjson::Document errorMessage;
    errorMessage.SetObject();
    errorMessage.AddMember("type", "error", errorMessage.GetAllocator());
    errorMessage.AddMember("message", "Error getting stocks from StocksService", errorMessage.GetAllocator());

    rapidjson::StringBuffer errorBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> errorWriter(errorBuffer);
    errorMessage.Accept(errorWriter);

    echo_server.send(hdl, errorBuffer.GetString(), websocketpp::frame::opcode::text);
}
}

//make a POST request to the StocksService to buy a stock
void MyServer::buyStocksFromService(web::http::http_request request, websocketpp::connection_hdl hdl, const rapidjson::Document& document) {
    // Create an HTTP client
    web::http::client::http_client client(U("http://localhost:8085"));

    // Serialize the RapidJSON document to a string
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    std::string jsonString = buffer.GetString();

    // Make a POST request to the StocksService
    client.request(web::http::methods::POST, U("/stocks/buy"), jsonString, U("application/json"))
        .then([this, hdl](web::http::http_response response) -> pplx::task<web::json::value> {
            if (response.status_code() == web::http::status_codes::OK) {
                std::cout << "Stocks bought successfully" << std::endl;
                return response.extract_json();
            } else {
                // Handle the error directly without involving handle_stocks_request
                rapidjson::Document errorMessage;
                errorMessage.SetObject();
                errorMessage.AddMember("type", "error", errorMessage.GetAllocator());
                errorMessage.AddMember("message", "Failed to buy stocks from StocksService", errorMessage.GetAllocator());

                rapidjson::StringBuffer errorBuffer;
                rapidjson::Writer<rapidjson::StringBuffer> errorWriter(errorBuffer);
                errorMessage.Accept(errorWriter);

                echo_server.send(hdl, errorBuffer.GetString(), websocketpp::frame::opcode::text);

                return pplx::task_from_result(web::json::value()); // Return an empty JSON task
            }
        })
        .then([this, hdl](pplx::task<web::json::value> previousTask) {
            std::cout << "Sending response to client" << std::endl;
            auto stock = previousTask.get();

            // Process the stock data here

            rapidjson::Document responseMessage;
            responseMessage.SetObject();
            responseMessage.AddMember("type", "buyStocks", responseMessage.GetAllocator());

            rapidjson::Value stockValue(rapidjson::kObjectType);

            //send OK or NOT OK under the "status" key
            stockValue.AddMember("status", "OK", responseMessage.GetAllocator());

            responseMessage.AddMember("stock", stockValue, responseMessage.GetAllocator());

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            responseMessage.Accept(writer);

            std::cout << "Sending response to client" << std::endl;
            echo_server.send(hdl, buffer.GetString(), websocketpp::frame::opcode::text);
        })
        .then([hdl]() {
            std::cout << "Finished sending response to client" << std::endl;
        });
}

void MyServer::sellStocksFromService(web::http::http_request request,  websocketpp::connection_hdl hdl, const rapidjson::Document& document){
    // Create an HTTP client
    web::http::client::http_client client(U("http://localhost:8085"));

    // Serialize the RapidJSON document to a string
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    std::string jsonString = buffer.GetString();

    // Make a POST request to the StocksService
    client.request(web::http::methods::POST, U("/stocks/sell"), jsonString, U("application/json"))
        .then([this, hdl](web::http::http_response response) -> pplx::task<web::json::value> {
            if (response.status_code() == web::http::status_codes::OK) {
                std::cout << "Stocks sold successfully" << std::endl;
                return response.extract_json();
            } else {
                // Handle the error directly without involving handle_stocks_request
                rapidjson::Document errorMessage;
                errorMessage.SetObject();
                errorMessage.AddMember("type", "error", errorMessage.GetAllocator());
                errorMessage.AddMember("message", "Failed to sell stocks from StocksService", errorMessage.GetAllocator());

                rapidjson::StringBuffer errorBuffer;
                rapidjson::Writer<rapidjson::StringBuffer> errorWriter(errorBuffer);
                errorMessage.Accept(errorWriter);

                echo_server.send(hdl, errorBuffer.GetString(), websocketpp::frame::opcode::text);

                return pplx::task_from_result(web::json::value()); // Return an empty JSON task
            }
        })
        .then([this, hdl](pplx::task<web::json::value> previousTask) {
            std::cout << "Sending response to client" << std::endl;
            auto stock = previousTask.get();

            // Process the stock data here

            rapidjson::Document responseMessage;
            responseMessage.SetObject();
            responseMessage.AddMember("type", "sellStocks", responseMessage.GetAllocator());

            rapidjson::Value stockValue(rapidjson::kObjectType);

            //send OK or NOT OK under the "status" key
            stockValue.AddMember("status", "OK", responseMessage.GetAllocator());

            responseMessage.AddMember("stock", stockValue, responseMessage.GetAllocator());

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            responseMessage.Accept(writer);

            std::cout << "Sending response to client" << std::endl;
            echo_server.send(hdl, buffer.GetString(), websocketpp::frame::opcode::text);
        })
        .then([hdl]() {
            std::cout << "Finished sending response to client" << std::endl;
        });
}

void MyServer::getOwnedStocksFromService(web::http::http_request request,  websocketpp::connection_hdl hdl, const rapidjson::Document& document){
    try {
        web::http::client::http_client client(U("http://localhost:8085"));

        // Serialize the RapidJSON document to a string
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string jsonString = buffer.GetString();

        // Make a GET request to the StocksService, send the email in the query string
        client.request(web::http::methods::GET, U("/ownedStocks?email=") + utility::conversions::to_string_t(document["email"].GetString()))
            .then([this, hdl](web::http::http_response response) -> pplx::task<web::json::value> {
                if (response.status_code() == web::http::status_codes::OK) {
                    std::cout << "Owned stocks retrieved successfully" << std::endl;
                    return response.extract_json();
                } else {
                    // Handle the error directly without involving handle_stocks_request
                    rapidjson::Document errorMessage;
                    errorMessage.SetObject();
                    errorMessage.AddMember("type", "error", errorMessage.GetAllocator());
                    errorMessage.AddMember("message", "Failed to get owned stocks from StocksService", errorMessage.GetAllocator());

                    rapidjson::StringBuffer errorBuffer;
                    rapidjson::Writer<rapidjson::StringBuffer> errorWriter(errorBuffer);
                    errorMessage.Accept(errorWriter);

                    echo_server.send(hdl, errorBuffer.GetString(), websocketpp::frame::opcode::text);

                    return pplx::task_from_result(web::json::value()); // Return an empty JSON task
                }
            })
            .then([this, hdl](pplx::task<web::json::value> previousTask) {
                try {
                    auto stocksArray = previousTask.get()[U("stocks")].as_array();

                    rapidjson::Document responseMessage;
                    responseMessage.SetObject();
                    responseMessage.AddMember("type", "getOwnedStocks", responseMessage.GetAllocator());

                    rapidjson::Value stocksValue(rapidjson::kArrayType);

                    for (const auto& stock : stocksArray) {
                        rapidjson::Value stockValue(rapidjson::kObjectType);
                        stockValue.AddMember("id", stock.at(U("id")).as_integer(), responseMessage.GetAllocator());
                        const std::string nameString = utility::conversions::to_utf8string(stock.at(U("name")).as_string());
                        stockValue.AddMember("name", rapidjson::Value().SetString(nameString.c_str(), responseMessage.GetAllocator()), responseMessage.GetAllocator());
                        stockValue.AddMember("price", stock.at(U("price")).as_double(), responseMessage.GetAllocator());
                        stockValue.AddMember("quantity", stock.at(U("quantity")).as_integer(), responseMessage.GetAllocator());

                        stocksValue.PushBack(stockValue, responseMessage.GetAllocator());
                    }

                    responseMessage.AddMember("stocks", stocksValue, responseMessage.GetAllocator());

                    rapidjson::StringBuffer buffer;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    responseMessage.Accept(writer);

                    std::cout << "Sending response to client" << std::endl;
                    std::cout << buffer.GetString() << std::endl;

                    echo_server.send(hdl, buffer.GetString(), websocketpp::frame::opcode::text);
                } catch (const std::exception& ex) {
                    // Handle the error directly without involving handle_stocks_request
                    rapidjson::Document errorMessage;
                    errorMessage.SetObject();
                    errorMessage.AddMember("type", "error", errorMessage.GetAllocator());
                    errorMessage.AddMember("message", "Error processing JSON data", errorMessage.GetAllocator());

                    rapidjson::StringBuffer errorBuffer;
                    rapidjson::Writer<rapidjson::StringBuffer> errorWriter(errorBuffer);
                    errorMessage.Accept(errorWriter);

                    echo_server.send(hdl, errorBuffer.GetString(), websocketpp::frame::opcode::text);
                }
            })
            .wait();
    } catch (const std::exception& ex) {
        // Handle the error directly without involving handle_stocks_request
        rapidjson::Document errorMessage;
        errorMessage.SetObject();
        errorMessage.AddMember("type", "error", errorMessage.GetAllocator());
        errorMessage.AddMember("message", "Error getting stocks from StocksService", errorMessage.GetAllocator());

        rapidjson::StringBuffer errorBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> errorWriter(errorBuffer);
        errorMessage.Accept(errorWriter);

        echo_server.send(hdl, errorBuffer.GetString(), websocketpp::frame::opcode::text);
    }
}

void MyServer::getOrderHistoryFromService(web::http::http_request request,  websocketpp::connection_hdl hdl, const rapidjson::Document& document){
    try {
        web::http::client::http_client client(U("http://localhost:8085"));

        // Serialize the RapidJSON document to a string
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string jsonString = buffer.GetString();

        // Make a GET request to the StocksService, send the email in the query string
        client.request(web::http::methods::GET, U("/orderHistory?email=") + utility::conversions::to_string_t(document["email"].GetString()))
            .then([this, hdl](web::http::http_response response) -> pplx::task<web::json::value> {
                if (response.status_code() == web::http::status_codes::OK) {
                    std::cout << "Order History retrieved successfully" << std::endl;
                    return response.extract_json();
                } else {
                    // Handle the error directly without involving handle_stocks_request
                    rapidjson::Document errorMessage;
                    errorMessage.SetObject();
                    errorMessage.AddMember("type", "error", errorMessage.GetAllocator());
                    errorMessage.AddMember("message", "Failed to get owned stocks from StocksService", errorMessage.GetAllocator());

                    rapidjson::StringBuffer errorBuffer;
                    rapidjson::Writer<rapidjson::StringBuffer> errorWriter(errorBuffer);
                    errorMessage.Accept(errorWriter);

                    echo_server.send(hdl, errorBuffer.GetString(), websocketpp::frame::opcode::text);

                    return pplx::task_from_result(web::json::value()); // Return an empty JSON task
                }
            })
            .then([this, hdl](pplx::task<web::json::value> previousTask) {
                try {
                    auto stocksArray = previousTask.get()[U("stocks")].as_array();

                    rapidjson::Document responseMessage;
                    responseMessage.SetObject();
                    responseMessage.AddMember("type", "getOrderHistory", responseMessage.GetAllocator());

                    rapidjson::Value stocksValue(rapidjson::kArrayType);

                    for (const auto& stock : stocksArray) {
                        rapidjson::Value stockValue(rapidjson::kObjectType);
                        stockValue.AddMember("id", stock.at(U("id")).as_integer(), responseMessage.GetAllocator());
                        const std::string nameString = utility::conversions::to_utf8string(stock.at(U("name")).as_string());
                        stockValue.AddMember("name", rapidjson::Value().SetString(nameString.c_str(), responseMessage.GetAllocator()), responseMessage.GetAllocator());
                        stockValue.AddMember("price", stock.at(U("price")).as_double(), responseMessage.GetAllocator());
                        stockValue.AddMember("quantity", stock.at(U("quantity")).as_integer(), responseMessage.GetAllocator());
                        stockValue.AddMember("status", stock.at(U("status")).as_integer(), responseMessage.GetAllocator());

                        stocksValue.PushBack(stockValue, responseMessage.GetAllocator());
                    }

                    responseMessage.AddMember("stocks", stocksValue, responseMessage.GetAllocator());

                    rapidjson::StringBuffer buffer;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    responseMessage.Accept(writer);

                    std::cout << "Sending response to client" << std::endl;
                    std::cout << buffer.GetString() << std::endl;

                    echo_server.send(hdl, buffer.GetString(), websocketpp::frame::opcode::text);
                } catch (const std::exception& ex) {
                    // Handle the error directly without involving handle_stocks_request
                    rapidjson::Document errorMessage;
                    errorMessage.SetObject();
                    errorMessage.AddMember("type", "error", errorMessage.GetAllocator());
                    errorMessage.AddMember("message", "Error processing JSON data", errorMessage.GetAllocator());

                    rapidjson::StringBuffer errorBuffer;
                    rapidjson::Writer<rapidjson::StringBuffer> errorWriter(errorBuffer);
                    errorMessage.Accept(errorWriter);

                    echo_server.send(hdl, errorBuffer.GetString(), websocketpp::frame::opcode::text);
                }
            })
            .wait();
    } catch (const std::exception& ex) {
        // Handle the error directly without involving handle_stocks_request
        rapidjson::Document errorMessage;
        errorMessage.SetObject();
        errorMessage.AddMember("type", "error", errorMessage.GetAllocator());
        errorMessage.AddMember("message", "Error getting stocks from StocksService", errorMessage.GetAllocator());

        rapidjson::StringBuffer errorBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> errorWriter(errorBuffer);
        errorMessage.Accept(errorWriter);

        echo_server.send(hdl, errorBuffer.GetString(), websocketpp::frame::opcode::text);
    }
}

void MyServer::server_thread() {
    try {
        // Start the server
        echo_server.run();

        // Wait for the server to stop
        while (!stopServerThread) {
            // Add a delay or other logic if needed
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Stop the server
        echo_server.stop();
    } catch (websocketpp::exception const &e) {
        std::cout << "WebSocketPP exception: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "An unknown exception occurred in the server thread." << std::endl;
    }
}

void MyServer::start() {
    std::cout << "Starting server..." << std::endl;

    // Run the server in a separate thread
    std::thread serverThread(&MyServer::server_thread, this);

    // Wait for the server thread to finish
    serverThread.join();

    // Set the flag to stop the server thread
    stopServerThread = true;

}