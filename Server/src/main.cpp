// main.cpp
#include "server/MyServer.h"
#include "server/database.h"
#include "stocks_service/StockService.h"

int main() {
    try {
        MyServer myServer;
        StockService stockService;

        // Create threads for MyServer and StocksService
        std::thread serverThread(&MyServer::start, &myServer);
        //std::thread stockServiceThread(&StockService::service_thread, &stockService);

        // Join the threads to run concurrently
        serverThread.join();
        //stockServiceThread.join();

        // Sleep or do other work as needed
        //std::this_thread::sleep_for(std::chrono::seconds(10));

    }catch (const std::exception& ex) {
        std::cerr << "Exception in main: " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "An unknown exception occurred in main." << std::endl;
    }

    return 0;
}