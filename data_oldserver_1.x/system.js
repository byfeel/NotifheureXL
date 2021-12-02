var langs = ['en', 'fr'];
//var langCode = 'en';
var langJS = null;


var translate = function (jsdata)
{	
	$("[bkey]").each (function (index)
	{
		var strTr = jsdata [$(this).attr ('bkey')];
	    $(this).html (strTr);
	});
   
		$.each( jsdata.color_name, function( key, value ) {
        couleurs[key]=value;
      	});

	  	$.each(jsdata.buzmusic, function ( key, value ) {
			console.log("jsdata: "+value);
			buzMusic[key]=value;
	  	});

	   $.each(jsdata.typeled, function ( key, value ) {
		typeLed[key]=value;
	   });
		$.each(jsdata.ZXL, function ( key, value ) {
		ZXL[key]=value;
		});
		$.each(jsdata.Z, function ( key, value ) {
		Z[key]=value;
		});
	$.each(jsdata.animation, function ( key, value ) {
		animation[key]=value;
	});
}


//langCode = navigator.language.substr (0, 2);

//if (langCode in langs)
//	$.getJSON('lang/'+langCode+'.json', translate);
//else
//	$.getJSON('lang/fr.json', translate);