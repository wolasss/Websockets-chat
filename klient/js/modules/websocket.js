var MODwebsocket = function(sb){
	var _sock = null, timeout = 4000, connected = false;
	var connectionError = function(error) {
		console.log(error);
		sb.emit('connectionError', error);
	},
	connectionSuccess = function(msg) {
		if(_sock.readyState === WebSocket.OPEN) {
			connected = true;
			sb.emit('connectionSuccess', msg);
		}
		//now login to chat
	},
	processMessage = function() {

	},
	closeConnection = function() {
		connected = false;
	},
	checkConnection = function() {
		if(!connected) {
			sb.emit('connectionError', 'timeout');
		}
	},
	connect = function(hostname, port) {
		try {
			_sock = new WebSocket('ws://'+hostname+':'+port, ['chat']);
			setTimeout(checkConnection, timeout);
			_sock.onerror = connectionError;
			_sock.onopen = connectionSuccess;
			_sock.onmessage = processMessage;
			_sock.onclose = closeConnection;
		} catch(e) {
			connectionError(e.message);
		}
	},
	processLoginRequest = function(data) {
		connect(data.hostname, data.port);
	},
	sendMessage = function(data) {
		if(_sock.readyState === WebSocket.OPEN) {
			_sock.send(data);
		}
	},
	reactor = function( data, topic ){
	  	switch( topic ){
		    case "loginRequest":
		    	processLoginRequest(data);
		    	break;
		    case "WSsendMessage":
		    	sendMessage(data);
		      	break;
		  	}
	};

	return {
	    init: function() {
	    	sb.on( ['loginRequest', 'WSsendMessage'], reactor );
	    },
	    destroy: function() { 
	    	
	    }
	};
};