
/*
	expr like example
*/

// global varables and code
var expression;
var v = new Array();
setexpr(jsarguments[1]);
inlets = Math.max(1,jsarguments[2]);
outlets = Math.max(1,jsarguments[3]);

function msg_float(f)
{
	v[inlet] = f;
	if (inlet==0) {
		doexpr();
	}
}

function anything()
{
	v = arrayfromargs(messagename,arguments);
	doexpr();
}

function doexpr()
{
	var rv;
	
	// use Math object to reduce clutter from expression.
	// e.g.  "sin(v[0])" instead of "Math.sin(v[0])"
	with (Math) { 
		rv = eval(expression);
	}
	outlet(0,rv);
}

function setexpr(s)
{
	expression = s.toString().replace(/\"/g,""); // strip quotes if present
}
