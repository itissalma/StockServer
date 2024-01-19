// Navbar.js
import React from 'react';
import Link from 'next/link';
import './Navbar.css';

const Navbar = () => {

    const handleLogout = () => {
        // Clear the email from local storage
        localStorage.removeItem('email');
        window.location.href = '/';
      };
      
  return (
    <nav className="navbar">
      <div className="navbar-container">
        <Link href="/stocklist" className="nav-link">
          Stock List
        </Link>
        <Link href="/stocksowned" className="nav-link">
          Owned Stocks
        </Link>
        <Link href="/transactions" className="nav-link">
          Transaction History
        </Link>
        <button className="logout-button" onClick={handleLogout}>
          Logout
        </button>
        </div>
    </nav>
  );
};

export default Navbar;
