// MyServer.h
#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/connection_hdl.hpp>
#include <thread>
#include <functional>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <cpprest/http_msg.h>
#include "database.h"

typedef websocketpp::server<websocketpp::config::asio> server;

class MyServer {
public:
    MyServer();
    ~MyServer();

    void start();
    std::chrono::seconds fetchInterval{10};

private:
    server echo_server;
    Database database;
    bool stopServerThread = false;
    void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg);
    void on_close(websocketpp::connection_hdl hdl);
    void server_thread();
    void handle_login(websocketpp::connection_hdl hdl, const rapidjson::Document& document);
    void handle_signup(websocketpp::connection_hdl hdl, const rapidjson::Document& document);
    bool perform_signup(const std::string& email, const std::string& password, const std::string& username, const std::string& nationalId);
    void getStocksFromService(web::http::http_request request,  websocketpp::connection_hdl hdl);
    void getOwnedStocksFromService(web::http::http_request request,  websocketpp::connection_hdl hdl, const rapidjson::Document& document);
    void getOrderHistoryFromService(web::http::http_request request,  websocketpp::connection_hdl hdl, const rapidjson::Document& document);
    void sellStocksFromService(web::http::http_request request,  websocketpp::connection_hdl hdl, const rapidjson::Document& document);
    void buyStocksFromService(web::http::http_request request,  websocketpp::connection_hdl hdl, const rapidjson::Document& document);
    bool is_valid_credentials(const std::string& email, const std::string& password);
};
