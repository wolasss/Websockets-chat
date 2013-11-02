var MODloginForm = function(sb){

	var data = {
		username: '',
		port: '',
		hostname: ''
	},
	username,
	port,
	errors = 0,
	hostname,
	loginButton, 
	usernameField, 
	portField, 
	portContainer,
	hostnameField,
	hostnameContainer, 
	errorContainer,
	defaultServerField, 
	defaultServer = 0,
	parsePort,
	connectionError,
	connectionSucess,
	reactor,
	showError,
	logIn,
	defaultServerChange,
	parseResponse,
	cancelLogin,
	loginSuccess,
	keyPressed;

	parsePort = function (port) {
		var p = -1, m=(""+port).match(/^\d{1,5}$/gi);
		if(m && m.length!==0) {
			p = parseInt(m[0], 10);
		}
		return p;
	};

	connectionError = function(msg) {
		sb.clear(errorContainer);
		showError(msg);
		NProgress.done(function(){
	    	sb.show(errorContainer);
		});
	};
	connectionSuccess = function(msg) {
		NProgress.set(0.9);
		NProgress.configure({speed:50});
	};
	loginSuccess = function(data){
		NProgress.done(function(){
			setTimeout(sb.toggleModule, 200);
			sb.emit('loggedIn', data.message);
		});
	};
	parseResponse = function(response) {
		if(response.status==101) {
			//ok
			loginSuccess(response);
		} else if(response.status>500){
			//failure
			connectionError(response.message);
		}
	};
	//message handler
	reactor = function( data, topic ){
	  	switch( topic ){
		    case "connectionError":
		    	connectionError('Error. Server is not responding: '+data);
		    	break;
		    case "connectionSuccess":
		    	connectionSuccess(data);
		      	break;
		  	case "WSresponse":
		  		parseResponse(data);
		  		break;
		  	case "loggedOut":
		  		sb.toggleModule();
		  		break;
		  	}
	};
	showError = function(Msg) {
		sb.append(errorContainer, '<p>'+Msg+'</p>');
	};
	logIn = function (e) {
		sb.hide(errorContainer);
		sb.clear(errorContainer);
		errors=0;

		NProgress.start();

		if(!defaultServer) {
			port = parsePort(portField.value);
			if(port===-1) {
				showError('Wrong port number');
				errors = 1;
			}
			if(hostnameField.value.length!==0) {
				hostname = hostnameField.value;
			} else {
				showError('Empty hostname field');
				errors = 1;
			}
		} else {
			data.hostname = 'adamwolski.com';
			data.port = '12345';
		} 

		if(usernameField.value.length!==0) {
			username = usernameField.value;
		} else {
			showError('Empty username field');
			errors = 1;
		}
		if(errors) {
			NProgress.done(function() {
				sb.show(errorContainer);
			});
		} else {
			data.port = port;
			data.username = username;
			data.hostname = hostname;
			sb.emit('loginRequest', data);
		}
	};
	defaultServerChange = function(e) {
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
	keyPressed = function(e) {
		if(e.which==13) {
			logIn();
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
	    	errorContainer = sb.find(sb.CSSerror)[0];

	    	sb.addEvent(loginButton, 'click', logIn);
	    	sb.addEvent(usernameField, 'keyup', keyPressed);
	    	sb.addEvent(portField, 'keyup', keyPressed);
	    	sb.addEvent(hostnameField, 'keyup', keyPressed);
	    	sb.addEvent(defaultServerField, 'change', defaultServerChange);

	    	sb.on(['connectionError', 'connectionSuccess', 'WSresponse', 'loggedOut'], reactor);

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

	    	sb.removeEvent(loginButton, 'click', logIn);
	    	sb.removeEvent(defaultServerField, 'change', defaultServerChange);
	    }
	};
};

CHAT.register("loginForm", MODloginForm);
CHAT.start('loginForm');