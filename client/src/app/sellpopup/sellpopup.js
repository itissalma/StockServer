// sellpopup.js
"use client";
import React, { useState, useEffect } from 'react';
import ReactDOM from 'react-dom';
import './sellpopup.css';
import useWebSocket from '../hooks/webSocket';

const SellPopup = ({ ownedStock, onSell, onCancel }) => {
  const [quantity, setQuantity] = useState(1);
  const [sellStatus, setSellStatus] = useState(null);
  const { sendMessage, isConnected, addMessageListener } = useWebSocket();

  useEffect(() => {
    // Add a message listener for handling incoming messages
    const sellMessageListener = (message) => {
      console.log('Received message from server:', message);

      // Handle the message based on its type
        if (message.stock.status === 'OK') {
          // Set sell status to success
          setSellStatus('success');
        } else {
          // Set sell status to failure
          setSellStatus('failure');
        }
    };

    // Add the message listener
    const removeSellMessageListener = addMessageListener(sellMessageListener);

    // Clean up the message listener on unmount
    return () => {
      removeSellMessageListener();
    };
  }, [addMessageListener]);

  const handleSellConfirmation = () => {
    if (!isConnected) {
      console.log('WebSocket not connected');
      return;
    }

    // Create sell data object
    const sellData = {
      type: 'sell',
      ownedStockId: ownedStock.id,
      quantity,
      price: ownedStock.price,
      email: localStorage.getItem('email'),
    };

    // Send the sell data to the server
    console.log('Sending sell data to server:', sellData);
    sendMessage(JSON.stringify(sellData));
  };

  const calculateTotalAmount = () => {
    return (quantity * ownedStock.price).toFixed(2);
  };

  const handleClose = () => {
    // Reset sell status and close the popup
    setSellStatus(null);
    onCancel();
  };

  return ReactDOM.createPortal(
    <div className="modal-overlay">
      <div className="modal">
        <h2 className="modal-title">Sell Stock</h2>
        {sellStatus === 'success' && (
          <>
            <p className="success-message">Sale successful!</p>
            <button className="close-button" onClick={handleClose}>
              Close
            </button>
          </>
        )}
        {sellStatus === 'failure' && (
          <>
            <p className="failure-message">Sale failed. Please try again.</p>
            <button className="close-button" onClick={handleClose}>
              Close
            </button>
          </>
        )}
        {sellStatus !== 'success' && sellStatus !== 'failure' && (
          <>
            <p className="modal-text">Stock Name: {ownedStock.name}</p>
            <p className="modal-text">Stock Price: ${ownedStock.price.toFixed(2)}</p>
            <div className="quantity-container">
              <label className="modal-label">Quantity:</label>
              <div className="quantity-input">
                <button
                  className="quantity-button"
                  onClick={() => setQuantity(quantity > 1 ? quantity - 1 : 1)}
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
              <button className="sell-button" onClick={handleSellConfirmation}>
                Sell
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

export default SellPopup;