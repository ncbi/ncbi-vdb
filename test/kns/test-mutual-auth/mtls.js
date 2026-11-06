const https = require('https');
const fs = require('fs');
const hostname = 'localhost';
const port = 3001;
const options = { 
    ca: fs.readFileSync('ca.crt'), 
    cert: fs.readFileSync('server.crt'), 
    key: fs.readFileSync('server.key'), 
    rejectUnauthorized: true,
    requestCert: true, 
}; 
const server = https.createServer(options, (req, res) => {
  res.statusCode = 200;
  res.setHeader('Content-Type', 'text/plain');
  res.setHeader('Content-Type', 'text/plain');
  res.setHeader('Content-Length', '17');
  res.setHeader('Accept-Ranges', 'bytes');
  res.end('Hello cert World\n');
});
server.listen(port, hostname, () => {
  console.log(`Server running at http://${hostname}:${port}/`);
});