const https = require('https');
const fs = require('fs');
const hostname = 'localhost';
const port = 3002;
const options = { 
    ca: fs.readFileSync('ca.crt'), 
    cert: fs.readFileSync('server.crt'), 
    key: fs.readFileSync('server.key'), 
    rejectUnauthorized: true,
}; 
const server = https.createServer(options, (req, res) => {
  res.statusCode = 200;
  res.setHeader('Content-Type', 'text/plain');
  res.setHeader('Content-Type', 'text/plain');
          res.setHeader('Content-Length', '18');
          res.setHeader('Accept-Ranges', 'bytes');
        res.end('Hello https World\n');
});
server.listen(port, hostname, () => {
  console.log(`Server running at http://${hostname}:${port}/`);
});