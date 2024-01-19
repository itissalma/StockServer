import React, { useEffect, useState, useRef } from 'react';
import io from 'socket.io-client';

const SocketConnection = ({ onDataReceived }) => {
  const [socket, setSocket] = useState(null);
  const isMounted = useRef(true);

  useEffect(() => {
    const newSocket = io('http://localhost:1234');
    console.log('Connecting to C++ server via WebSocket');
    console.log('newSocket', newSocket);
    newSocket.on('connect', () => {
      console.log('Connected to C++ server via WebSocket');
    });

    newSocket.on('data', (data) => {
      console.log('Data received from C++ server:', data);
      onDataReceived(data);
    });

    newSocket.on('disconnect', () => {
      console.log('Disconnected from C++ server via WebSocket');
    });

    newSocket.on('error', (error) => {
        console.error('WebSocket error:', error);
      });
      

    setSocket(newSocket);

    return () => {
      // Check if the component is still mounted before disconnecting
      if (isMounted.current) {
        newSocket.disconnect();
        console.log('Cleaned up WebSocket connection');
      }
    };
  }, [onDataReceived]);

  useEffect(() => {
    // Set the ref to false when the component unmounts
    return () => {
      isMounted.current = false;
    };
  }, []);

  return <></>;
};

export default SocketConnection;
