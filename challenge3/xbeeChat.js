var SerialPort = require("serialport");
var app = require('express')();
var http = require('http').Server(app);

var portName = process.argv[2],
portConfig = {
	baudRate: 115200,
	parser: SerialPort.parsers.readline("\n")
};

app.get('/', function(req, res){
	  res.sendfile('index.html');
	});

var sp = new SerialPort.SerialPort(portName, portConfig);
sp.on("open", function() {

	app.get("/led_on", function(req, res) {

		id = req.query.id;
		password = req.query.password;

		sp.write("turn_on " + id + " " + password + "\n");
		res.send("LED " + id + " turned on");
	}); 

	app.get("/led_off", function(req, res) {
		id = req.query.id;
		password = req.query.password;
		sp.write("turn_off " + id + " " + password + "\n");
		res.send("LED " + id + " turned off");
	}); 

	app.get("/toggle", function(req, res) {
		console.log(req.query)
		id = req.query.id;
		password = req.query.password;
		sp.write("toggle " + id + " " + password + "\n");
		res.send("LED " + id + " toggled");
	}); 

	app.get("/get", function(req, res) {
		id = req.query.id;
		password = req.query.password;
		sp.on('data', function(data) {
			if(data == "1")
				res.send("LED " + id + " is currently on");
			else
				res.send("LED " + id + " is currently off");
			sp.removeAllListeners();
		});

		
		sp.write("get " + id + " " + password + "\n");
	}); 

	app.get("/set_pass", function(req, res) {
		console.log("HIII")
		id = req.query.id;
		password = req.query.password;
		newp = req.query.newp;
		sp.write("set_pass " + id + " " + password + " " + newp + "\n");
		res.send("Password updated for id:" + id + " password:" + password + " newp:" + newp);
	});

});

http.listen(3000, function(){
  console.log('listening on *:3000');
});