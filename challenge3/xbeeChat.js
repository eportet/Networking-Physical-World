var SerialPort = require("serialport");
var app = require('express')();
var http = require('http').Server(app);

var portName = process.argv[2],
portConfig = {
	baudRate: 115200,
	parser: SerialPort.parsers.readline("\n")
};

//var sp = new SerialPort.SerialPort(portName, portConfig);
var sp = new SerialPort.SerialPort(portName, portConfig);
sp.on("open", function() {

	app.get("/led_on", function(req, res) {
		console.log("RECV")
		sp.write("turn on\n");
		res.send("led turned on");
	}); 

	app.get("/led_off", function(req, res) {
		sp.write("turn off\n");
		res.send("led turned off");
	}); 

	app.get("/get", function(req, res) {
		sp.on('data', function(data) {
			res.send("The led is currently: " + data)
		});
		sp.write("get\n");	
	}); 

	http.listen(3000, function(){
	  console.log('listening on *:3000');
	});
});

