cmake_minimum_required(VERSION 3.10)
project(ServerProject)

add_library(httplib INTERFACE)

set(CMAKE_CXX_STANDARD 11)

# Update these paths to the correct installation location
set(JsonCpp_DIR "/usr/local/lib/cmake/jsoncpp")
set(CMAKE_PREFIX_PATH "/usr/local/lib/cmake/jsoncpp")
set(PPLX_DIR "/home/parallels/cpprestsdk/build")

find_package(Threads REQUIRED)
find_package(cpprestsdk REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(OpenSSL REQUIRED)
find_library(PPLX_LIBRARY pplx HINTS ${PPLX_DIR}/Release/lib)

#include_directories(${JsonCpp_INCLUDE_DIRS})
include_directories("/usr/local/include/jsoncpp")  # Update this path if needed
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/websocketpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/rapidjson)
include_directories(${JSONCPP_INCLUDE_DIRS})

add_executable(ServerProject
    src/main.cpp
    src/server/MyServer.cpp
    src/server/database.cpp
    src/stocks_service/StockService.cpp
)

target_include_directories(httplib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ServerProject PRIVATE ${CPPREST_INCLUDE_DIRS})

target_link_libraries(ServerProject PRIVATE 
    mysqlcppconn Threads::Threads ${CPPREST_LIBRARIES} ${JSONCPP_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto
    "/usr/local/lib/libjsoncpp.so"
    "-lcpprest"  # This links the cpprestsdk library
)
