var app = require('express')();
var http = require('http').Server(app);
var mysql = require("mysql");


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



var con = mysql.createConnection({
  host: "192.168.1.126",
  user: "user",
  password: "user",
  database: "challenge2"
});


app.get('/', function(req, res){
  res.sendfile('index.html');
});

app.get('/hist_temps', function(req, res){
	var query = req.query;
	if(query.startTime.length < 5 || query.startTime.indexOf(":") < 1)
		res.send({"Error": "Bad Start time"});
	if(query.endTime.length < 5 || query.endTime.indexOf(":") < 1)
		res.send({"Error": "Bad End time"});
	if(query.startDate.length < 5)
		res.send({"Error": "Bad Start Date"});
	if(query.endDate.length < 5)
		res.send({"Error": "Bad End Date"});
	
	var startTime = query.startTime;
	var endTime = query.endTime;
	
	var start = new Date(query.startDate);
	var end = new Date(query.endDate);	
	var sH = parseInt(startTime.substring(0, startTime.indexOf(":")));
	var sM = parseInt(startTime.substring(startTime.indexOf(":") + 1));
	
	var eH = parseInt(endTime.substring(0, endTime.indexOf(":")));
	var eM = parseInt(endTime.substring(endTime.indexOf(":") + 1));
	try {
		start.setHours(sH);
		start.setMinutes(sM);
		
		end.setHours(eH);
		end.setMinutes(eM);
	} catch (e) {
		res.send({"Error": "Bad input"});
		console.log(e);
		return;
	}
	console.log(start.toMysqlFormat());
	console.log(end.toMysqlFormat());
	
	con.query("SELECT * FROM temperatures WHERE timestamp BETWEEN \'" + start.toMysqlFormat() + "\' AND \'" + end.toMysqlFormat() + "\';", function (err, result, fields) {
		if (err) throw err;
		var ans = [];
		for(var i = 0; i < result.length; i++) {
			ans.push({id: result[i].id, timestamp: result[i].timestamp, temp: result[i].temperature});
		}
		res.send(ans);
	});
});

app.get("/current", function(req, res){
	con.query("SELECT t1.*, id.x, id.y FROM temperatures t1 INNER JOIN id ON id.id = t1.id WHERE t1.timestamp = (SELECT MAX(t2.timestamp) FROM temperatures t2 WHERE t2.id = t1.id)", function (err, result, fields) {
		if (err) throw err;
		var ans = [];
		for(var i = 0; i < result.length; i++) {
			ans.push({id: result[i].id, timestamp: result[i].timestamp, temp: result[i].temperature, x: result[i].x, y: result[i].y});
		}
		res.send(ans);
	});
});

app.get("/devices", function(req, res) {
	con.query("SELECT * FROM id", function (err, result, fields) {
		if (err) throw err;
		var ans = [];
		for(var i = 0; i < result.length; i++) {
			ans.push({id: result[i].id, x: result[i].x, y: result[i].y});
		}
		res.send(ans);
	});
});

app.get("/set_device", function(req, res) {
	var x = req.query.x;
	var y = req.query.y;
	var id = req.query.id;
	con.query("UPDATE id SET x=" + x + ", y=" + y + " WHERE id="+id+";", function (err, result, fields) {
		if (err) throw err;
	});
	console.log("Donezo");
});


http.listen(3000, function(){
  console.log('listening on *:3000');
});
