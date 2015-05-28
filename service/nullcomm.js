
/**
	
	This is a mock service used when I don't have a Bean handy, for example, I want to work on the 
	service from the coffee shop.

 */

var stringify = require("json-stringify-safe"),
	util = require('util'),
	_ = require('lodash'),
	EE = require("events").EventEmitter;

function Comm() {

	EE.call(this);
	
	this.setup = function() {
		console.log("starting null comms");
	};

	this.connect = function(cb) {
		cb && _.defer(cb)
	};

	this.disconnect = function(cb) {
		cb && _.defer(cb)
	};

	this.areYouThere = function(cb) {
		cb && _.defer(cb)
	};

	this.sendMessage = function(message) {

		console.log("message:" + stringify(message));

	};


}

util.inherits(Comm, EE);


module.exports = Comm;
