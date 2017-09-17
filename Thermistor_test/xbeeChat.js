var SerialPort = require("serialport");
var app = require('express')();
var http = require('http').Server(app);

var attached = [];
var temperatures = [];
var last_received = [];
var proposed = [];
var count = 0;
var portName = process.argv[2],
portConfig = {
	baudRate: 115200,
	parser: SerialPort.parsers.readline("\n")
};

var sp;
sp = new SerialPort.SerialPort(portName, portConfig);

app.get('/', function(req, res){
  res.sendfile('index.html');
});

sp.on("open", function () {
  console.log("Listening on Serial");
  sp.on('data', function(data) {
	  console.log(data);
	if(data.substring(0,3) == "GET") {

		var id = 0;
		
		count += 1;

		id = count;
		data = data.substring(4);
		sp.write("GIVE " + data + " " + id +"\n");
		proposed.push(id);
	}
	else {
		var id = data.substring(0, data.indexOf(" "));
		var temp = data.substring(data.indexOf(" ") + 1);
		
		var prop = proposed.indexOf(parseInt(id));
		if(prop >= 0 && attached.indexOf(parseInt(id)) < 0) {
			proposed.slice(prop, 1);
			attached.push(parseInt(id));
			temperatures.push([]);
			last_received.push(0);
			console.log("ADDED" + id);
			
		}
		
		var index = attached.indexOf(parseInt(id));
		if(index < 0)
			return;
		last_received[index] = 0;
		temperatures[index].push(parseFloat(temp));
	}
  });
});

function update() {
	if(attached.length == 0) {
		console.log("No thermostats!");
		return;
	}
	
	console.log(attached);
	console.log(last_received);
	for(var i = 0; i < attached.length; i++) {
		last_received[i] += 1;
		if(last_received[i] > 10) {
			attached.splice(i, 1);
			temperatures.splice(i, 1);
			last_received.splice(i, 1);
			i -= 1;
		}
	}
	var avg = 0;
	for(var i = 0; i < attached.length; i++) {
		if(temperatures[i].length > 0)
			avg += temperatures[i][temperatures[i].length - 1];
	}
	avg /= attached.length;
	if(isNaN(avg)) {
		avg = "No data yet";
	}
	console.log("------");
	console.log(attached.length + " thermostats connected");
	console.log("Average temp: " + avg);
}
setInterval(update, 1000);

app.get('/temps', function(req, res){
  res.send(temperatures);
});

http.listen(3000, function(){
  console.log('listening on *:3000');
});