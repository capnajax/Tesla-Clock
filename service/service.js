#!/usr/local/bin/node

var http = require('http'),
	comm,
	stringify = require('json-stringify-safe'),
	fs = require('fs'),
	_ = require('lodash'),
	photos = require('./photos'),
	images = [];

console.log(JSON.stringify(process.argv));

if(-1 != _.indexOf(process.argv, "-serial")) {
	comm = new (require('./serialservice'))();
} else if(-1 != _.indexOf(process.argv, "-nullcomm")) {
	comm = new (require('./nullcomm'))();
} else {
	comm = new (require('./btservice'))();
}
comm.setup();


(function() {
	var imagesLoaded = 0,
		imgInterval = setInterval(function() {
			if(imagesLoaded == 3) {
				clearInterval(imgInterval);
				console.log("images:");
				console.log(JSON.stringify(images));
			}
		});

	photos.loadPhoto('./photos/img1.png', function(img) {
		images[0] = img;
		imagesLoaded++;
	}),
	photos.loadPhoto('./photos/img2.png', function(img) {
		images[1] = img;
		imagesLoaded++;
	}),
	photos.loadPhoto('./photos/img3.png', function(img) {
		images[2] = img;
		imagesLoaded++;
	})
})();
	

// 
// service
//
(function() {
	var serviceHTML,
		server = http.createServer(function(request, response) {
		if(request.url) {
			var index = request.url.indexOf('+');
			if(-1 != index && request.url.length > index+1) {
				var message = request.url.substring(index+1);

				if(message.charAt(0)=='B') {
					comm.startNormalProgram();
				} else if(message.charAt(0)=='S') {
					comm.getStatus();
				} else if(message.charAt(0)=='T') {
					comm.setTime();
				} else if(message.charAt(0)=='M') {
					comm.setMood(parseInt(message.substring(1)));
				} else if(message.charAt(0)=='W') {
					comm.weather(message.charAt(1));
				} else if(message.charAt(0)=='P') {
					comm.sendPhoto(images[message.charAt(1)]);
				} else {
// TODO re-implement!
//					comm.sendMessage(message);
				}
				response.writeHead(200, {"Content-Type": "text/plain"});
				response.end("OK .");
			} else {
				response.end(serviceHTML);
			}
		}
	});
	fs.readFile('service.html', function(err, data) {
		serviceHTML = data;
	});
	server.listen(7979);
})();

function attemptConnection() {
	comm.connect(function(err) {
		console.log("connect attempt result:");
		if(err) {
			console.log("Connection failed, retrying in 5 seconds");
			this.setTimeout(attemptConnection, 5000);
		} else {
			console.log("Connected");
		}
	});	
}
attemptConnection();


comm.on('open', function(data) {
	comm.setTime();
});

comm.on('data', function(data) {
	console.log('data: ' + data);
	if(data.length > 1) {
		switch(data.charAt(0)) {
		case 'B': 
			comm.setTime();
			break;
		case 'K':
			comm.sendMessage('A');
			break;
		}
	}
});

setInterval(function() {
	comm.setTime();
}, 5*60*1000);


