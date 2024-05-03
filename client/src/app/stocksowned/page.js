// Import necessary modules and styles
"use client";
import React, { useState, useEffect } from 'react';
import './page.css'; 
import useWebSocket from '../hooks/webSocket';
import SellPopup from '../sellpopup/sellpopup'; 
import Navbar from '../Navbar';

const OwnedStockList = () => {
  const [ownedStocks, setOwnedStocks] = useState([]);
  const [loading, setLoading] = useState(true);
  const [sellPopupData, setSellPopupData] = useState(null);
  const { sendMessage, isConnected, addMessageListener } = useWebSocket();

  useEffect(() => {
    // Fetch owned stocks from the server when the WebSocket connection is established
    if (isConnected) {
      fetchOwnedStocks();
    }

    // Add a message listener for handling getOwnedStocks messages
    const handleGetOwnedStocks = (message) => {
      if (message.type === 'getOwnedStocks' && message.stocks) {
        console.log('Received owned stocks from server:', message.stocks);
        const sortedStocks = message.stocks.sort((a, b) => a.id - b.id);
        setOwnedStocks(sortedStocks);
      }
    };

    const unsubscribeGetOwnedStocks = addMessageListener(handleGetOwnedStocks);

    // Clean up the message listener on unmount
    return () => {
      unsubscribeGetOwnedStocks();
    };
  }, [isConnected]); // Dependency array includes isConnected

  const fetchOwnedStocks = async () => {
    try {
      console.log('Sending message to server:');
      if (!isConnected) {
        console.error('WebSocket not connected');
        return;
      }

      // Create a message object
      const message = {
        type: 'getOwnedStocks',
        email: localStorage.getItem('email'),
      };

      // Set loading state to true when starting to fetch
      setLoading(true);

      // Send the message to the server and wait for the response
      const response = await sendMessage(JSON.stringify(message));

      if (response && response.type === 'getOwnedStocks' && response.stocks) {
        console.log('Received owned stocks from server:', response.stocks);
        const sortedStocks = response.stocks.sort((a, b) => a.id - b.id);
        setOwnedStocks(sortedStocks);
      } else {
        console.error('Failed to fetch owned stocks from the server.');
      }
    } catch (error) {
      console.error('Error fetching owned stocks:', error);
    } finally {
      // Set loading state to false when fetching is completed
      setLoading(false);
    }
  };

  const handleSellStock = (ownedStock) => {
    // Set the data for the sell pop-up
    setSellPopupData({ ownedStock });
  };

  const handleSellConfirmation = (ownedStock, quantity) => {
    // Add your logic for selling the stock here
    console.log(`Selling stock with ID ${ownedStock.stock.id} and quantity ${quantity}`);
    // Close the pop-up after selling
    setSellPopupData(null);
  };

  const handleSellPopupCancel = () => {
    // Close the pop-up when canceled
    setSellPopupData(null);
  };

  return (<>
    <Navbar/>
    <div className="container">
      <h1>Owned Stock List</h1>
      {loading ? (
        <p>Loading...</p>
      ) : (
        <div className="table-container">
          <table>
            <thead>
              <tr>
                <th>Stock ID</th>
                <th>Stock Name</th>
                <th>Quantity</th>
                <th>Price</th>
                <th>Action</th>
              </tr>
            </thead>
            <tbody>
              {ownedStocks.map((ownedStock) => (
                <tr key={ownedStock.id}>
                  <td>{ownedStock.id}</td>
                  <td>{ownedStock.name}</td>
                  <td>{ownedStock.quantity}</td>
                  <td>${ownedStock.price.toFixed(2)}</td>
                  <td>
                    <button onClick={() => handleSellStock(ownedStock)}>Sell</button>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}

      {/* Sell Pop-up */}
      {sellPopupData && (
        <SellPopup
          ownedStock={sellPopupData.ownedStock}
          onSell={handleSellConfirmation}
          onCancel={handleSellPopupCancel}
        />
      )}
    </div>
    </>
  );
};

export default OwnedStockList;
