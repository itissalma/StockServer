#pragma once

#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <websocketpp/server.hpp>
#include <cpprest/http_listener.h>
#include <thread> 
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#include <cstdlib>  // for rand()
#include <ctime> 
#include <chrono>
#include "../server/database.h"

class StockService {
public:
    StockService();
    ~StockService();
    void service_thread();
    void handleGetStocks(web::http::http_request request);
    void handleBuyStocks(web::http::http_request request);
    void handleSellStocks(web::http::http_request request);
    void handleGetOwnedStocks(web::http::http_request request, std::string email);
    void handleGetOrderHistory(web::http::http_request request, std::string email);
    void updateStockPrices(int stockId, double percentageChange);

private:
    std::atomic<bool> stopServiceThread;
    std::atomic<bool> stopUpdateThread;
    Database database;
    std::thread serviceThread;
    std::thread updateThread;
    double generateRandomPercentageChange(double minChange, double maxChange);
    void updateStockPricesBasedOnDemand(int stockId, double demandFactor);
    void adjustStockPricesBasedOnDemand();
    void updateStockPricesPeriodically();
};
