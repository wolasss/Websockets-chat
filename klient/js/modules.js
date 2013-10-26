var loginForm = function(sb){

	var data = {
		username: '',
		port: '',
		hostname: ''
	},
	loginButton, 
	usernameField, 
	portField, 
	portContainer,
	hostnameField,
	hostnameContainer, 
	errorContainer,
	defaultServerField, 
	defaultServer = 0;

	var parsePort = function (port) {
		var p = -1, m=(""+port).match(/^\d{1,5}$/gi);
		if(m && m.length!==0) {
			p = parseInt(m[0], 10);
		}
		return p;
	}; 

	//message handler
	var reactor = function( data, topic ){
	  	switch( topic ){
		    case "somethingHappend":
		    	sb.emit( "myEventTopic", processData(data) );
		    	break;
		    case "aNiceTopic":
		    	justProcess( data );
		      	break;
		  	}
	},
	logIn = function (e) {

		NProgress.start();

        NProgress.done(function(){
        	$('.chat').addClass('expanded');
        	$('.chat-container').html('');
        }); 
	},
	defaultServerChange = function(e) {
		console.log('change');
		var checked = defaultServerField.checked;
		if(checked) {
			defaultServer = 1;
			sb.slideUp(portContainer);
			sb.slideUp(hostnameContainer);
		} else {
			defaultServer = 0;
			sb.slideDown(portContainer);
			sb.slideDown(hostnameContainer);
		}
	};

	return {
	    init: function() {
	    	sb.toggleModule();
	    	loginButton = sb.find(sb.CSSloginButton)[0];
	    	usernameField = sb.find(sb.CSSusernameField)[0];
	    	portField = sb.find(sb.CSSportField)[0];
	    	portContainer = sb.parent(portField);
	    	hostnameField = sb.find(sb.CSShostnameField)[0];
	    	hostnameContainer = sb.parent(hostnameField);
	    	defaultServerField = sb.find(sb.CSSdefaultServerField)[0];

	    	sb.addEvent(loginButton, 'click', logIn);
	    	sb.addEvent(defaultServerField, 'change', defaultServerChange);

	    },
	    destroy: function() { 
	    	sb.toggleModule();

			loginButton = null; 
	    	usernameField = null; 
	    	portField = null; ;
	    	portContainer = null; 
	    	hostnameField = null; 
	    	hostnameContainer = null; 
	    	defaultServerField = null;  


	    }
	};
};