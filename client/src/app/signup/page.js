// pages/signup.js
"use client";
import React, { useState , useEffect} from 'react';
import useWebSocket from '../hooks/webSocket';

const SignupPage = () => {
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [username, setUsername] = useState('');
  const [nationalId, setNationalId] = useState(''); 
  const [message, setMessage] = useState('');
  const { sendMessage, isConnected, addMessageListener } = useWebSocket();

  useEffect(() => {
    const handleSignupResponse = (response) => {
      console.log('Received signup response from server:', response);

      if (response.type === 'signup' && response.success) {
        // Redirect to the login page 
        console.log('Signup successful. Redirecting to login...');
        // Create a popup saying user created successfully
        alert("User created successfully");
        window.location.href = '/';
      } else {
        console.log('Signup failed.');
        setMessage('Signup failed. Please try again.');
      }
    };

    const unsubscribeSignupResponse = addMessageListener(handleSignupResponse);

    return () => {
      // Cleanup: Unsubscribe from message listener on component unmount
      unsubscribeSignupResponse();
    };
  }, [addMessageListener]);

  const handleSignup = async (e) => {
    e.preventDefault();

    if (!isConnected) {
      console.log('WebSocket not connected');
      return;
    }

    // Validation checks
    if (!email || !password || !username || !nationalId) {
      setMessage('Please fill in all fields.');
      return;
    }

    // Validate National ID length
    if (nationalId.length !== 14) {
      setMessage('National ID must be 14 numbers long.');
      return;
    }

    // Clear any previous error messages
    setMessage('');

    // Create signup data object
    const signupData = {
      type: 'signup',
      email,
      password,
      username, // Added username
      nationalId, // Added nationalId
    };

    // Send the signup data to the server
    console.log('Sending signup data to server:', signupData);

    try {
      // Await the promise from sendMessage
      await sendMessage(JSON.stringify(signupData));
    } catch (error) {
      console.error('Error sending message:', error);
    }
  };

  return (
    <div className="flex items-center justify-center min-h-screen bg-gray-100">
      <div className="w-full max-w-md p-4 bg-white rounded-md shadow-md">
        <h2 className="text-2xl font-bold mb-4">Sign Up</h2>
        <form onSubmit={handleSignup}>
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
          <div className="mb-4">
            <label htmlFor="username" className="block text-sm font-medium text-gray-600">
              Username
            </label>
            <input
              type="text"
              id="username"
              name="username"
              value={username}
              onChange={(e) => setUsername(e.target.value)}
              className="mt-1 p-2 w-full border rounded-md"
              placeholder="Your username"
            />
          </div>
          <div className="mb-4">
            <label htmlFor="nationalId" className="block text-sm font-medium text-gray-600">
              National ID
            </label>
            <input
              type="text"
              id="nationalId"
              name="nationalId"
              value={nationalId}
              onChange={(e) => setNationalId(e.target.value)}
              className="mt-1 p-2 w-full border rounded-md"
              placeholder="Your national ID"
            />
          </div>
          {message && <p className="text-red-500">{message}</p>}
          <button
            type="submit"
            className="w-full bg-green-500 text-white p-2 rounded-md hover:bg-green-600"
          >
            Sign Up
          </button>
        </form>
      </div>
    </div>
  );
};

export default SignupPage;
