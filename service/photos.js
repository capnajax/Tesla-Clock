var Canvas = require('canvas'),
	fs = require('fs');


function px9(ctx, origin, r, θ) {

	x = origin.x + Math.round(r*Math.cos(θ));
	y = origin.y + Math.round(r*Math.sin(θ));

	var imgData = ctx.getImageData(x-1,y-1,3,3),
		px = {r:0,g:0,b:0},
		totalAlpha = 0;

	for(var i = 0; i < imgData.data.length; i+=4) {
		if(imgData.data[i+3] > 0) {
			totalAlpha += imgData.data[i+3];
			px.r += imgData.data[i  ]*imgData.data[i+3];
			px.g += imgData.data[i+1]*imgData.data[i+3];
			px.b += imgData.data[i+2]*imgData.data[i+3];
		}
	}

	trim = function(cv) {
		cv /= totalAlpha;
		cv = Math.min(255, Math.max(0, Math.round(cv)));
		return cv;
	}

	if(totalAlpha > 0) {
		px.r = trim(px.r);
		px.g = trim(px.g);
		px.b = trim(px.b);
	}; // else {r,g,b} will all still be zero

	return px;
}

function loadPhoto(filename, cb) {

	// get the size
	var dimensions = (require('image-size'))(filename);
	console.log(JSON.stringify(dimensions));

	// load the image file
	var image = new (Canvas.Image),
		canvas = new Canvas(dimensions.width, dimensions.height),
		ctx = canvas.getContext('2d')

	fs.readFile(filename, {}, function(err, data) {

		if(err) {
			console.log("FAILED to read " + filename);
			cb && cb("error");

		} else {

			console.log("read " + filename);
			
			image.src = data;

			ctx.drawImage(image,0,0);

			// get the relevant pixels

			// to simplify the math, we will use r,θ coordinates against an x,y origin. 
			// the outer circle has its origin in the centre of the image and the radium 90% of the image
			var resultMinutes = [],
				resultHours = [],
				origin = {x:dimensions.width/2,y:dimensions.height/2},
				r = Math.min(origin.x, origin.y) * 0.9,
				x, y;

			for(var θ = 90; θ > -270; θ-=6) {
				resultMinutes.push(px9(ctx, origin, r, θ));
			}

			origin.x -= Math.min(origin.x, origin.y) * 0.36;
			r = 0.4*r;

			for(var θ = 82.5; θ > -270; θ-=15) {
				resultHours.push(px9(ctx, origin, r, θ));
			}

			var result = resultHours.concat(resultMinutes);

			// print to console
			var logStr = "\t{\t";
			for(var i = 0; i < result.length; i++) {
				if(i != 0) {
					logStr += ", ";
					if(0 == i % 7) {
						logStr += "\n\t\t";
					}
				}
				logStr += JSON.stringify(result[i]);
			}
			logStr += "}"
			console.log(logStr);

			cb && cb(result);
		}

	});

		
	
}

module.exports = {
	loadPhoto: loadPhoto
};
