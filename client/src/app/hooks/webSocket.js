// hooks/useWebSocket.js
"use client";
import { useEffect, useState, useRef } from 'react';

const useWebSocket = () => {
  const [socket, setSocket] = useState(null);
  const [isConnected, setIsConnected] = useState(false);
  const messageListenersRef = useRef([]);

  useEffect(() => {
    const newSocket = new WebSocket('ws://localhost:9062');

    newSocket.onopen = () => {
      console.log('WebSocket connection established');
      setIsConnected(true);
    };

    newSocket.onmessage = (event) => {
      console.log('Received message from server:', event.data);
      const message = JSON.parse(event.data);
      messageListenersRef.current.forEach((listener) => listener(message));
    };

    newSocket.onclose = (event) => {
      console.log('WebSocket connection closed:', event);
      setIsConnected(false);
    };

    setSocket(newSocket);

    // Clean up the socket connection on unmount
    return () => {
      newSocket.close();
    };
  }, []);

  const sendMessage = (message) => {
    if (socket && isConnected) {
      console.log('Sending message to server:', message);
      socket.send(message);
    } else {
      console.log('WebSocket not connected');
    }
  };

  const addMessageListener = (listener) => {
    messageListenersRef.current.push(listener);

    return () => {
      messageListenersRef.current = messageListenersRef.current.filter(
        (existingListener) => existingListener !== listener
      );
    };
  };

  return { sendMessage, isConnected, addMessageListener };
};

export default useWebSocket;