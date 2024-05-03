// Import necessary modules and styles
"use client";
import React, { useState, useEffect } from 'react';
import './page.css'; 
import useWebSocket from '../hooks/webSocket';
import Navbar from '../Navbar';

const OrderHistory = () => {
  const [orderHistory, setOrderHistory] = useState([]);
  const [loading, setLoading] = useState(true);
  const { sendMessage, isConnected, addMessageListener } = useWebSocket();

  useEffect(() => {
    // Fetch order history from the server when the WebSocket connection is established
    if (isConnected) {
      fetchOrderHistory();
    }

    // Add a message listener for handling getOrderHistory messages
    const handleGetOrderHistory = (message) => {
      if (message.type === 'getOrderHistory' && message.stocks) {
        console.log('Received order history from server:', message.stocks);
        const sortedOrders = message.stocks.sort((a, b) => a.id - b.id);
        setOrderHistory(sortedOrders);
      }
    };

    const unsubscribeGetOrderHistory = addMessageListener(handleGetOrderHistory);

    // Clean up the message listener on unmount
    return () => {
      unsubscribeGetOrderHistory();
    };
  }, [isConnected]); // Dependency array includes isConnected

  const fetchOrderHistory = async () => {
    try {
      console.log('Sending message to server:');
      if (!isConnected) {
        console.error('WebSocket not connected');
        return;
      }

      // Create a message object
      const message = {
        type: 'getOrderHistory',
        email: localStorage.getItem('email'),
      };

      // Set loading state to true when starting to fetch
      setLoading(true);

      // Send the message to the server and wait for the response
      const response = await sendMessage(JSON.stringify(message));

      if (response && response.type === 'getOrderHistory') {
        console.log('Received order history from server:', response.stocks);
        const sortedOrders = response.stocks.sort((a, b) => a.id - b.id);
        setOrderHistory(sortedOrders);
      } else {
        console.error('Failed to fetch order history from the server.');
      }
    } catch (error) {
      console.error('Error fetching order history:', error);
    } finally {
      // Set loading state to false when fetching is completed
      setLoading(false);
    }
  };

  return (<>
    <Navbar />
    <div className="container">
      <h1>Order History</h1>
      {loading ? (
        <p>Loading...</p>
      ) : (
        <div className="table-container">
          <table>
            <thead>
              <tr>
                <th>Order ID</th>
                <th>Stock Name</th>
                <th>Quantity</th>
                <th>Price</th>
                <th>Status</th>
              </tr>
            </thead>
            <tbody>
              {orderHistory.map((order) => (
                <tr key={order.id} className={order.status === 0 ? 'bought-row' : 'sold-row'}>
                <td>{order.id}</td>
                  <td>{order.name}</td>
                  <td>{order.quantity}</td>
                  <td>${order.price.toFixed(2)}</td>
                  <td>{order.status === 0 ? 'Bought' : 'Sold'}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}
    </div>
    </>
  );
};

export default OrderHistory;
