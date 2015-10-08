var http = require('http');
var url = require('url');
var req_url = process.argv[2];

// here is the data we want to put
var object_to_put = { "hello" : "world" };

// let's stringify it and put the result in a Buffer.
// (this is important because we need the Content-Length in bytes,
// and the string might contain multi-byte characters.)
var data_to_put = new Buffer(JSON.stringify(object_to_put));

// build our request options, starting by parsing the request URL
var req_options = url.parse(req_url);
req_options.method = "POST";
req_options.headers = {
	"Content-Type": "application/json",
	"Content-Length": data_to_put.length
};

// make request and write the body
var req = http.request(req_options, handleResponse);
req.write(data_to_put);
// make sure to end so we get a response
req.end();

// handle the response (just pipe it to stdout)
function handleResponse(res) {
	console.error(res.statusCode);
	if (res.statusCode != 200) {
		console.error("(warning: request did not succeed)");
	}
	res.pipe(process.stdout);
}
