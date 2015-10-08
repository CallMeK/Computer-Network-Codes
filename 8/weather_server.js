/*
 * weather_server.js
 * 
 * Serve static pages from the current directory.
 * For GET requests to /weather?q=..., forward to api.openweathermap.org.
 */
var express = require('express');
var http = require('http');

var app = express( );
app.use(express.static('.'));

var cacheArray = new Array();

app.get('/weather/:zip', function(req, res) {

	var zip = req.params.zip;
	var zip_key = zip+"_"; 
	//seems like node JS will use integer in memory, leading to a waste of memory
	//even thought the key shows as string, as pointed by some in stackoverflow
	var now = new Date().getTime();
	res.set('Content-Type', 'application/json');
	if(!(zip_key in cacheArray) )
	{
		cacheArray[zip_key] = {}; 
		get_weather_res();
	}
	else if(cacheArray[zip_key].timecreated - now > 30*60*1000)
	{
		console.log('Previous record expired...');
		get_weather_res();
	}
	else
	{
		console.log('retrieving the cached result');
		res.send(cacheArray[zip_key].data_);
	}

	function get_weather_res(){
		http.get(
				"http://api.openweathermap.org/data/2.5/weather?q="+zip,
				function(weather_res) {
					var result = '';
					/* send all weather data to our client */
					weather_res.on('data',function(chunk){
						result += chunk;
						res.write(chunk);
					});

					// weather_res.pipe(res); //not using it, to make the code easier to understand
					weather_res.on('end',function(){
						res.end();
						cacheArray[zip_key].data_ = result;
						cacheArray[zip_key].timecreated = new Date().getTime();
						//console.log(cacheArray);
					});
					
				});
	}



});



var server = app.listen(3000);
