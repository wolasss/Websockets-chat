var MODwebsocket = function(sb){
	var _sock = null, 
		timeout = 4000, 
		connected = false,
		alreadyFailed = false,
		username = "",
		connectionError,
		sendMessage,
		connectionSuccess,
		processMessage,
		closeConnection,
		sendLoginRequest,
		connect,
		processLoginRequest,
		reactor;

	connectionError = function(error) {
		alreadyFailed = true;
		sb.emit('connectionError', '');
	};
	sendMessage = function(data) {
		if(_sock.readyState === WebSocket.OPEN) {
			_sock.send(data);
		}
	};
	connectionSuccess = function(msg) {
		if(_sock.readyState === WebSocket.OPEN) {
			connected = true;
			alreadyFailed = false;
			sb.emit('connectionSuccess', msg);
		}
		sendLoginRequest(username);
	};
	processMessage = function(e) {
		try {
			var message = JSON.parse(""+e.data);
			if(message.status) {
				sb.emit('WSresponse', message);
			}
		} catch(e) {
			//ignore message it's not json
		}
	};
	closeConnection = function() {
		connected = false;
	};
	checkConnection = function() {
		if(!connected && !alreadyFailed) {
			sb.emit('connectionError', 'timeout');
		}
	};
	sendLoginRequest = function(username) {
		if(connected) {
			sendMessage("/login "+username);
		}
	};
	connect = function(hostname, port) {
		try {
			_sock = new WebSocket('ws://'+hostname+':'+port, ['chat']);
			alreadyFailed=false;
			setTimeout(checkConnection, timeout);
			_sock.onerror = connectionError;
			_sock.onopen = connectionSuccess;
			_sock.onmessage = processMessage;
			_sock.onclose = closeConnection;
		} catch(e) {
			connectionError(e.message);
		}
	};
	processLoginRequest = function(data) {
		username = data.username;
		connect(data.hostname, data.port);
	};
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