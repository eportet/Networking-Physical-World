var SerialPort = require("serialport");
var mysql = require("mysql");
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

var sp = new SerialPort.SerialPort(portName, portConfig);

var con = mysql.createConnection({
  host: "192.168.1.126",
  user: "user",
  password: "user",
  database: "challenge2"
});

con.query("SELECT * FROM id ORDER BY ID DESC LIMIT 1", function (err, result, fields) {
	if (err) throw err;
	var id = 0;
	console.log(result);
	if(result.length) {
		id = result[0].id + 1;
	}
	console.log("HI");
	console.log(id);
});

/**
 * You first need to create a formatting function to pad numbers to two digits…
 **/
function twoDigits(d) {
    if(0 <= d && d < 10) return "0" + d.toString();
    if(-10 < d && d < 0) return "-0" + (-1*d).toString();
    return d.toString();
}

/**
 * …and then create the method to output the date string as desired.
 * Some people hate using prototypes this way, but if you are going
 * to apply this to more than one Date object, having it as a prototype
 * makes sense.
 **/
Date.prototype.toMysqlFormat = function() {
    return this.getFullYear() + "-" + twoDigits(1 + this.getMonth()) + "-" + twoDigits(this.getDate()) + " " + twoDigits(this.getHours()) + ":" + twoDigits(this.getMinutes()) + ":" + twoDigits(this.getSeconds());
};


sp.on("open", function () {
  console.log("Listening on Serial");
  sp.on('data', function(data) {
	console.log(data);
	if(data.substring(0,3) == "GET") {
		
		var valid = false;
		  con.query("SELECT id FROM temperatures ORDER BY ID DESC LIMIT 1;", function (err, result, fields) {
			if (err) return;
			var id = 0;
			console.log(result);
			if(result.length == 0) {
				id = 0;
			}
			else {
				id = result[0].id + 1;
			}
			data = data.substring(4);
			sp.write("GIVE " + data + " " + id + "\n");
			
			con.query("INSERT INTO id VALUES(" + id + ",0,0);", function (err, result) {
				if (err) return;
				console.log("Inserted into db");
			});
		  });

	}
	else {
		console.log("me run");
		var id = data.substring(0, data.indexOf(" "));
		var temp = data.substring(data.indexOf(" ") + 1);

		con.query("INSERT INTO temperatures VALUES(" + id + ",'" + (new Date(Date.now()).toMysqlFormat())  + "'," + parseFloat(temp) + ");", function (err, result) {
				if (err) {
					console.log(err);
					return;
				}
				console.log("Inserted into db");
			});
	}
  });
}); 



