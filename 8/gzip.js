var zlib = require('zlib');

var zipper = zlib.createGzip();

// We can simply do this
// process.stdin.pipe(zipper).pipe(process.stdout);

// Here's what's going on under the hood.
process.stdin.on('data', function(data) {
	console.log(data);
	var ret = zipper.write(data);
	if (ret == false) {
		console.error("pausing standard input");
		process.stdin.pause();
		zipper.once('drain', function() { 
			console.error("resuming standard input");
			process.stdin.resume();
		});
	}
});

process.stdin.on('end', function() {
	zipper.end();
});

zipper.on('data', function(data) {
	var ret = process.stdout.write(data);
	if (ret == false) {
		zipper.pause();
		process.stdout.once('drain', function() {
			zipper.resume();
		});
	}
});

