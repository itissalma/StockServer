// components/LoginPage.js
"use client";
import React, { useState, useEffect } from 'react';
import useWebSocket from './hooks/webSocket';

const LoginPage = () => {
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [message, setMessage] = useState('');
  
  const { sendMessage, isConnected, addMessageListener } = useWebSocket();

  useEffect(() => {
    const handleLoginResponse = (response) => {
      console.log('Received login response from server:', response);

      if (response.type === 'login' && response.success) {
        // Redirect to the stock list page or perform other actions
        console.log('Login successful. Redirecting...');
        // Save email to local storage
        localStorage.setItem('email', email);
        window.location.href = '/stocklist';
      } else {
        console.log('Login failed.');
        setMessage('Login failed. Please try again.');
      }
    };

    const unsubscribeLoginResponse = addMessageListener(handleLoginResponse);

    return () => {
      // Cleanup: Unsubscribe from message listener on component unmount
      unsubscribeLoginResponse();
    };
  }, [addMessageListener, email]);

  const handleLogin = async () => {
    if (!isConnected) {
      console.log('WebSocket not connected');
      return;
    }

    // Validation checks
    if (!email || !password) {
      setMessage('Please fill in all fields.');
      return;
    }

    // Clear any previous error messages
    setMessage('');

    // Create login data object
    const loginData = {
      type: 'login',
      email,
      password,
    };

    // Send the login data to the server
    console.log('Sending login data to server:', loginData);

    try {
      // Await the promise from sendMessage
      const serverResponse = await sendMessage(JSON.stringify(loginData));
      console.log('Server response:', serverResponse);
    } catch (error) {
      console.error('Error sending message:', error);
    }
  };

  return (
    <div className="flex items-center justify-center min-h-screen bg-gray-100">
      <div className="w-full max-w-md p-4 bg-white rounded-md shadow-md">
        <h2 className="text-2xl font-bold mb-4">Login</h2>
        <form>
          <div className="mb-4">
            <label htmlFor="email" className="block text-sm font-medium text-gray-600">
              Email
            </label>
            <input
              type="email"
              id="email"
              name="email"
              value={email}
              onChange={(e) => setEmail(e.target.value)}
              className="mt-1 p-2 w-full border rounded-md"
              placeholder="Your email"
            />
          </div>
          <div className="mb-4">
            <label htmlFor="password" className="block text-sm font-medium text-gray-600">
              Password
            </label>
            <input
              type="password"
              id="password"
              name="password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              className="mt-1 p-2 w-full border rounded-md"
              placeholder="Your password"
            />
          </div>
          {message && <p className="text-red-500">{message}</p>}
          <button
            type="button"
            className="w-full bg-blue-500 text-white p-2 rounded-md hover:bg-blue-600"
            onClick={handleLogin}
          >
            Login
          </button>
        </form>
        <p className="mt-4 text-sm text-gray-600">
          Don't have an account? <a href="/signup" className="text-blue-500">Create one</a>
        </p>
      </div>
    </div>
  );
};

export default LoginPage;
