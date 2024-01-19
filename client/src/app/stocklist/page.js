"use client";
// StockList.js
import React, { useState, useEffect } from 'react';
import './page.css';
import useWebSocket from '../hooks/webSocket';
import BuyPopup from '../buypopup/buypopup';
import Navbar from '../Navbar';

const StockList = () => {
  const [stocks, setStocks] = useState([]);
  const [loading, setLoading] = useState(true);
  const [buyPopupData, setBuyPopupData] = useState(null);
  const { sendMessage, isConnected, addMessageListener } = useWebSocket();

  useEffect(() => {
    if (isConnected) {
      fetchStocks(); // Initial fetch
    }
  }, [isConnected]);

  useEffect(() => {
    const handleWebSocketMessage = (message) => {
      if (message.type === 'getStocks' && message.stocks) {
        console.log('Received SOME STOCKS');
        setStocks(message.stocks);
      }
    };

    if (isConnected) {
      const removeMessageListener = addMessageListener(handleWebSocketMessage);

      return () => {
        removeMessageListener();
      };
    }
  }, [isConnected, addMessageListener]);

  const fetchStocks = async () => {
    try {
      setLoading(true);

      const message = {
        type: 'getStocks',
      };

      await sendMessage(JSON.stringify(message));
    } catch (error) {
      console.error('Error fetching stocks:', error);
    } finally {
      setLoading(false);
    }
  };

  const handleBuyStock = (stock) => {
    setBuyPopupData({ stock, quantity: 1 });
  };

  const handleBuyConfirmation = (stock, quantity) => {
    console.log(`Buying stock with ID ${stock.id} and quantity ${quantity}`);
    setBuyPopupData(null);
  };

  const handleBuyPopupCancel = () => {
    setBuyPopupData(null);
  };

  return (
    <>
    <Navbar/>
    <div className="container">
      <h1>Stock Management System</h1>
      {loading ? (
        <p>Loading...</p>
      ) : (
        <div className="table-container">
          <table>
            <thead>
              <tr>
                <th>Stock ID</th>
                <th>Stock Name</th>
                <th>Stock Price</th>
                <th>Action</th>
              </tr>
            </thead>
            <tbody>
              {stocks.map((stock) => (
                <tr key={stock.id}>
                  <td>{stock.id}</td>
                  <td>{stock.name}</td>
                  <td>${stock.price.toFixed(2)}</td>
                  <td>
                    <button onClick={() => handleBuyStock(stock)}>Buy</button>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}

      {buyPopupData && (
        <BuyPopup
          stock={buyPopupData.stock}
          onBuy={handleBuyConfirmation}
          onCancel={handleBuyPopupCancel}
        />
      )}
    </div>
    </>
  );
};

export default StockList;
