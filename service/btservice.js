var Bean = require('ble-bean'),
	stringify = require('json-stringify-safe'),
	util = require('util'),
	EE = require("events").EventEmitter,
	_ = require('lodash');

/**
	A command is of the form:
	{ 	name: '<command>', // command is one of A,B,M,P,S,T,W
		sequence: <number>, // the sequence number of the command
		params: {data}, // optional, whataver that command needs
		copy: <response>, // {true, false, or null, meaning message received, failed, or no response},
		time: <time> // time the command was sent
	}
 */


var commandNumber = 0,
	recentCommands = [];

function Comm() {

	EE.call(this);

	this.clock = null;

	this.serialData = '';

	this.setup = function() {
		console.log("starting Bluetooth comms");
	};

	this.connect = function(cb) {

		var beanDiscovered = _.bind(function(bean) {
			clock = bean;
			console.log(stringify(bean));

			bean.on("disconnect", function() {
				process.exit();
			});

			bean.on("serial", function(data, valid) {
				this.serialData += data;
				var nIdx = this.serialData.indexOf('\n');
				if(-1 != nIdx) {
					var incomingMessage = this.serialData.substring(0, nIdx+1);
					this.serialData = this.serialData.substring(nIdx+1);
					console.log("Received: " + incomingMessage);
					this.emit("data", incomingMessage);
				}
			});

			bean.connectAndSetup(_.bind(function(){
				console.log("connectAndSetup");

				bean.notifyTwo(function(data) {
					console.log("notifyTwo.readCallback: " + JSON.stringify(data))
				}, function(error) {
					console.log("notifyTwo.callback" + (error?(" error: "+JSON.stringify(data)):" success"));
				});

				bean.notifyThree(function(data) {
					console.log("notifyThree.readCallback: " + JSON.stringify(data))
				}, function(error) {
					console.log("notifyThree.callback" + (error?(" error: "+JSON.stringify(data)):" success"));
				});

				bean.notifyFour(function(data) {
					console.log("notifyFour.readCallback: " + JSON.stringify(data))
				}, function(error) {
					console.log("notifyFour.callback" + (error?(" error: "+JSON.stringify(data)):" success"));
				});

				bean.notifyFive(_.bind(function(data) {
					console.log("notifyFive.readCallback: " + JSON.stringify(data))

					var segmentNum = data[0],
						resend = data[1];

					if(resend) {
						console.log("resending segment " + segmentNum);
						this.sendPhotoSegment(segmentNum);
					} else if(segmentNum == 0xFF) {
						console.log("sending segment 00");
						this.sendPhotoSegment(0);
					} else if(segmentNum < 13) {
						console.log("sending segment " + (segmentNum+1).toString());
						this.sendPhotoSegment(segmentNum+1);
					} else {
						console.log("sent all segments");
					}

				}, this), function(error) {
					console.log("notifyFive.callback" + (error?(" error: "+JSON.stringify(data)):" success"));
				});

				this.setTime();

				cb && cb();

			}, this));;

		}, this);

		console.log("Connecting...");

		Bean.discover(function(peripheral){		

			console.log("testing bean");
			console.log(stringify(peripheral._characteristics));

			var ps = peripheral._peripheral._noble._bindings._peripherals;
			for(var i in ps) {
				if(ps[i].advertisement.localName == "Tesla Clock") {
					console.log("Found Tesla Clock at " + peripheral.uuid);
					beanDiscovered(peripheral);
					break;
				}
			}	
		});

	};

	this.triedToExit = false;
	this.disconnect = function(cb) {

		if (this.clock && !triedToExit) {
		    triedToExit = true;
		    console.log('Disconnecting from Device...');
		    clock.disconnect.bind(clock, function(){});
		}
		cb && setTimeout(cb, 1000);

	};

	this.areYouThere = function(cb) {

	};

	this.startNormalProgram = function() {
		command = {
			name: 'B',
			time: new Date()
		}
		this.sendCommand(command);
	};

	this.weather = function(type) {
		this.sendCommand({
			name: 'W',
			params: type.charCodeAt(0),
			time: new Date()
		});
	};

	this.level = function() {
		console.log("sending L:");
		command = {
			name: 'L',
			time: new Date()
		}
		this.sendCommand(command);
	};

	this.setTime = function() {
		var now = new Date(),
	    	midnight = new Date(
		        now.getFullYear(),
		        now.getMonth(),
		        now.getDate(),
		        0,0,0),
		    diff = now.getTime() - midnight.getTime(), // difference in milliseconds
			command = {
				name: 'T',
				params: diff,
				time: now
			};

		this.sendCommand(command);
	};

	this.setMood = function(mood) {
		this.sendCommand({
			name: 'M',
			params: mood,
			time: new Date()
		});
	};

	this.getStatus = function() {
		this.sendCommand({
			name: 'S',
			time: new Date()
		});
	};

	this.sendPhoto = function(image) {
		this.sendCommand({
			name: 'P',
			time: new Date()
		});
		this.transferImage = image;
	};

	this.sendPhotoSegment = function(segmentNum) {
		var segmentBuffer = new Buffer(20);
		segmentBuffer.writeUInt8(segmentNum,0);
		segmentBuffer.writeUInt8(0,1);
		for(var i = 0; i < 6; i++) {
			var pixel = this.transferImage[segmentNum*6+i];
			segmentBuffer.writeUInt8(pixel.r, 2+i*3);
			segmentBuffer.writeUInt8(pixel.g, 2+i*3+1);
			segmentBuffer.writeUInt8(pixel.b, 2+i*3+2);
		}
		console.log("sendPhotoSegment sending " + JSON.stringify(segmentBuffer));
		clock.writeTwo(segmentBuffer, function(){});
	};

	this.sendCommand = function(command) {
		command.sequence = commandNumber++;
		console.log("Sending: " + JSON.stringify(command));
		var buffer = new Buffer(_.includes(['T','M'], command.name)?8:('W'==command.name?5:4));
		buffer.writeUInt16LE(command.sequence, 0);
		buffer.writeUInt8(0, 2);
		buffer.writeUInt8(command.name.charCodeAt(0), 3);
		if(_.includes(['T','M'], command.name)) {
			buffer.writeUInt32LE(command.params, 4);
		} else if('W' == command.name) {
			buffer.writeUInt8(command.params, 4);
		}
		clock.writeOne(buffer, function() {
			recentCommands[command.sequence] = command;
		});
	};

}

util.inherits(Comm, EE);


module.exports = Comm;





var triedToExit = false;

//turns off led before disconnecting
var exitHandler = function exitHandler() {

  var self = this;
};

