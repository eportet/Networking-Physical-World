var SerialPort = require("serialport");
var app = require('express')();
var http = require('http').Server(app);

var attached = [];
var count = 0;
var portName = process.argv[2],
portConfig = {
	baudRate: 9600,
	parser: SerialPort.parsers.readline("\n")
};
var sp;
sp = new SerialPort.SerialPort(portName, portConfig);

app.get('/', function(req, res){
  res.sendfile('index.html');
});

sp.on("open", function () {
  console.log('open');
  sp.on('data', function(data) {
    console.log('data received: ' + data);
	if(data.substring(0,3) == "GET") {
		console.log("I RUN");
		attached.push(0);
		var id = count;
		count += 1;
		data = data.substring(4);
		sp.write("GIVE " + data + " " + id +"\n")
	}
  });
});

