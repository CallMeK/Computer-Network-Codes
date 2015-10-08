var http = require('http');

// a simple server that echoes back the request body as the response body
var server = http.createServer(function(req, res) {
	console.log(req);
	res.setHeader("Content-Type", req.headers['content-type']);
	req.pipe(res);
});

server.listen(3000);
