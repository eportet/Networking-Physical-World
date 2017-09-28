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
	console.log(req);
	con.query("SELECT id FROM temperatures ORDER BY ID DESC LIMIT 1", function (err, result, fields) {
		if (err) throw err;
	});
});

app.get("/current", function(req, res){
	console.log(req);
	con.query("SELECT t1.* FROM temperatures t1 WHERE t1.timestamp = (SELECT MAX(t2.timestamp) FROM temperatures t2 WHERE t2.id = t1.id)", function (err, result, fields) {
		if (err) throw err;
		var ans = [];
		for(var i = 0; i < result.length; i++) {
			ans.push({id: result[i].id, timestamp: result[i].timestamp, temp: result[i].temperature});
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

http.listen(3000, function(){
  console.log('listening on *:3000');
});
