// // next.config.js
// /** @type {import('next').NextConfig} */
// const nextConfig = {
//     webpack: (config, { isServer }) => {
//       if (!isServer) {
//         // Exclude fs module from being bundled on the client side
//         config.resolve.fallback = {
//           fs: false,
//         };
//       }
//       return config;
//     },
//   };
  
//   module.exports = nextConfig;
// next.config.js
const webpack = require('webpack');

module.exports = {
  webpack: (config, { isServer }) => {
    if (!isServer) {
      config.plugins.push(
        new webpack.IgnorePlugin({
          resourceRegExp: /^fs$/,
        })
      );
    }
    return config;
  },
};
