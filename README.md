# Stock Management Web Application

## Overview

Stock Management Web Application is built using Next.js for the frontend and C++ for the backend server and service layers. Users can create an account, log in, view available stocks, buy and sell stocks, check their list of orders, and view their current stock.

## Table of Contents

- How to Use
- Compilation and Run Instructions
- Diagrams

## How to Use

1. **Create an Account:**
   - Click on the signup hyperlink on the login page.
   - Fill in the required details, ensuring all fields are completed, and the national ID is 14 characters long.
   - Upon successful account creation, you'll be redirected to the login page.

2. **Login:**
   - Log in to the account you just created.

3. **View Stocks:**
   - The main page displays a list of available stocks.

4. **Buy Stocks:**
   - Click on the "Buy" button next to a stock.
   - Update the quantity as needed.

5. **View Owned Stocks:**
   - Click on the "Owned Stocks" item in the navigation bar.
   - Sell buttons will be available next to each owned stock.

6. **View Transaction History:**
   - Click on the "Transaction History" tab to view all transactions.
   - Bought stocks are highlighted in green, and sold stocks are in red.

7. **Logout:**
   - Click on the logout button to log out.

## Compilation and Run instructions

You will need to have Next.js downlaoded and the following dependencies:
* net 1.0.2
* react 18
* react-dom 18
* ws 8.16.0

You will also need the following libraries and packages for C++:
* jsoncpp
* Threads
* cpprestsdk
* pplx
* rapidjson
* websocketpp
* httplib

To run the frontend run you need to run this command: 

npm run dev

To run the backend you need to run these three commands:

cmake ..

make

./ServerProject

## Diagrams

1. **Architecture Diagram**
<img width="950" alt="Screenshot 2024-01-19 at 11 17 10 PM" src="https://github.com/itissalma/StockManagement/assets/78408357/94588c63-c6e4-4f35-887a-3fb6d3327690">

2. **Class Diagram**
<img width="963" alt="Screenshot 2024-01-19 at 11 16 52 PM" src="https://github.com/itissalma/StockManagement/assets/78408357/8fae0525-e984-4290-b580-c28a98d89d0e">


