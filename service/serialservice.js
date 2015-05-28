
const SERIAL_DEV = '/tmp/cu.LightBlue-Bean';

var serialport = require("serialport"),
	stringify = require("json-stringify-safe"),
	util = require('util'),
	EE = require("events").EventEmitter;

function Comm() {

	EE.call(this);
	
	this.serialPort = new (serialport.SerialPort)(SERIAL_DEV, {
			baudrate: 9600,
			parser: serialport.parsers.readline("\n")
		}, false);

	this.setup = function() {
		console.log("starting serial comms");
	};

	this.connect = function(cb) {

		this.serialPort.open(function(error) {
			if(error) {
				cb && cb(error);
			} else {
				this.serialPort.on('data', function(data) {
					this.emit('data', data);
				}.bind(this));
				this.emit('open');
				cb && cb(null, {success: true});
			}
		}.bind(this));
	};

	this.disconnect = function(cb) {

		this.serialPort.close();
		cb && cb();
	};

	this.areYouThere = function(cb) {

		this.aytMessage = {
			timeout: setTimeout(function() {
					if(this.aytMessage) {
						cb({error: "NO_RESPONSE"});
					}
				}, 5000),
			cb: cb
		};
	};

	this.sendMessage = function(message) {

		console.log("message:" + stringify(message));
		this.serialPort.write(message);

	};


}

util.inherits(Comm, EE);


module.exports = Comm;
