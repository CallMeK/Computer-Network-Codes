var http = require('http');
var req_url = process.argv[2];

http.get(req_url, function(res) {
	res.setEncoding('utf8');
	res.on('data', function(data) {
		console.error("received " + data.length + " bytes");
		process.stdout.write(data);
	});
	//res.pipe(process.stdout);
});


