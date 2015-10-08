var net = require('net');

var server = net.createServer(function(c) {
	console.log("client connected");
	c.setEncoding("utf8");
	c.on('data', function(data) {
		c.write(data.toUpperCase());
	});

	c.on('end', function() { 
		console.log("client disconnected");
		c.end();
	});
});

server.listen(4444);

