// Import necessary modules and styles
"use client";
import React, { useState, useEffect } from 'react';
import ReactDOM from 'react-dom';
import './buypopup.css';
import useWebSocket from '../hooks/webSocket';

const BuyPopup = ({ stock, onBuy, onCancel }) => {
  const [quantity, setQuantity] = useState(1);
  const [buyStatus, setBuyStatus] = useState(null);
  const { sendMessage, isConnected, addMessageListener } = useWebSocket();

  useEffect(() => {
    // Add a message listener for handling incoming messages
    const buyMessageListener = (message) => {
      console.log('Received message from server:', message);

      // Handle the message based on its type
        if (message.stock.status === 'OK') {
          // Set buy status to success
          setBuyStatus('success');
        } else {
          // Set buy status to failure
          setBuyStatus('failure');
        }
    };

    // Add the message listener
    const removeBuyMessageListener = addMessageListener(buyMessageListener);

    // Clean up the message listener on unmount
    return () => {
      removeBuyMessageListener();
    };
  }, [addMessageListener]);

  const handleBuyConfirmation = () => {
    if (!isConnected) {
      console.log('WebSocket not connected');
      return;
    }

    // Create buy data object
    const buyData = {
      type: 'buy',
      stock: {
        name: stock.name,
        price: stock.price.toFixed(2),
        id: stock.id,
      },
      quantity,
      email: localStorage.getItem('email'),
    };

    // Send the buy data to the server
    console.log('Sending buy data to server:', buyData);
    sendMessage(JSON.stringify(buyData));
  };

  const calculateTotalAmount = () => {
    return (quantity * stock.price).toFixed(2);
  };

  const handleClose = () => {
    // Reset buy status and close the popup
    setBuyStatus(null);
    onCancel();
  };

  return ReactDOM.createPortal(
    <div className="modal-overlay">
      <div className="modal">
        <h2 className="modal-title">Buy Stock</h2>
        {buyStatus === 'success' && (
          <>
            <p className="success-message">Buy successful!</p>
            <button className="close-button" onClick={handleClose}>
              Close
            </button>
          </>
        )}
        {buyStatus === 'failure' && (
          <>
            <p className="failure-message">Buy failed. Please try again.</p>
            <button className="close-button" onClick={handleClose}>
              Close
            </button>
          </>
        )}
        {buyStatus !== 'success' && buyStatus !== 'failure' && (
          <>
            <p className="modal-text">Stock Name: {stock.name}</p>
            <p className="modal-text">Stock Price: ${stock.price.toFixed(2)}</p>
            <div className="quantity-container">
              <label className="modal-label">Quantity:</label>
              <div className="quantity-input">
                <button
                  className="quantity-button"
                  onClick={() =>
                    setQuantity(quantity > 1 ? quantity - 1 : 1)
                  }
                >
                  -
                </button>
                <input
                  type="number"
                  value={quantity}
                  onChange={(e) => setQuantity(e.target.value)}
                  min="1"
                />
                <button
                  className="quantity-button"
                  onClick={() => setQuantity(quantity + 1)}
                >
                  +
                </button>
              </div>
            </div>
            <p className="modal-text">Total Amount: ${calculateTotalAmount()}</p>
            <div className="modal-buttons">
              <button className="buy-button" onClick={handleBuyConfirmation}>
                Buy
              </button>
              <button className="cancel-button" onClick={onCancel}>
                Cancel
              </button>
            </div>
          </>
        )}
      </div>
    </div>,
    document.body
  );
};

export default BuyPopup;
