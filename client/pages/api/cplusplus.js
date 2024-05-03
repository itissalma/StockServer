// pages/api/cplusplus.js
export default async function handler(req, res) {
    try {
      const response = await fetch('http://localhost:1234/client/pages/api/cplusplus')      ; // Replace with your C++ server address
      const data = await response.json();
  
      res.status(200).json({ message: data.message });
    } catch (error) {
      console.error('Error fetching data:', error);
      res.status(500).json({ message: 'Error fetching data' });
    }
  }
  