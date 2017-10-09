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

app.get("/led_on", function(req, res) {
	sp.write("on\n");
	
});

http.listen(3000, function(){
  console.log('listening on *:3000');
});
