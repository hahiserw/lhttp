/*
 * author: Maurycy Skier
 * description: changes object to animated, coloured text
 *
 */
 

var Rainbow = function( id_handle, colors, lightness, fps ) {
	
	if( id_handle == undefined || id_handle == null ) throw new Error( "lol wrong element id" );
	var id = id_handle;
	
	if( id.innerHTML ) var text = id.innerHTML;
	//else if(id.value) this.text = id.value;
	else throw new Error( "lol no innerHTML?" );
	
	if( lightness != undefined && typeof lightness != "number" ) throw new Error( "lol wrong variable type" );
	
	switch( typeof colors ) {
	
		case "undefined":
			colors = [ "red", "orange", "yellow", "green", "blue", "indigo", "violet" ];
			break;
		
		case "number":
			lightness = lightness? lightness : 50;
			var colors_temp = [];
			var i = 0;
			for( var r = 0; r < 360; r += 360 / colors )
				colors_temp[i++] = "hsl("+ Math.floor( r ) +",100%,"+ lightness +"%)";
			colors = colors_temp;
			break;
		
		case "array":
			if( colors.length < 2 ) throw new Error( "lol add more colors" );
			
		default:
			throw new Error( "lol put here number of rainbow colors, array with colors or leave it empty for defaults" );
		
	}
	
	var dfps = 10;
	if( fps != undefined && typeof fps != "number" ) throw new Error( "lol put number as fps" );
	if( fps ) dfps = fps;
	
	this.start = function() {
		start();
	}
	
	this.toggle = function() {
		if( t ) {
			clearTimeout( t );
			t = false;
		}
		else start();
	}
	
	var t;
	var c = 0;
	var tc = 0;
	var temp = "";
	
	function start() {
		t = setInterval( next, 1000 / dfps );
		//i heard setInterval is bad
		//but its not 100fps ;p amirite? ;o
	}
	
	function next() {
		c = tc;
		
		for( var i in text ) {
			if( ++c == colors.length ) c = 0;
			temp += "<span style=\"color:"+ colors[c] +";\">"+ text[i] +"</span>";
		}
		
		id.innerHTML = temp;
		temp = "";
		
		if( --tc < 0 ) tc = colors.length-1;
	}
	
	id.addEventListener( "click", this.toggle, false );

}
