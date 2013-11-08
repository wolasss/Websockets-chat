var MODwebsocket = function(sb){
	"use strict";
	var _sock = null, 
		timeout = 4000, 
		connected = false,
		alreadyFailed = false,
		username = "",
		connectionError,
		sendMessage,
		connectionSuccess,
		processMessage,
		onCloseConnection,
		sendLoginRequest,
		connect,
		processLoginRequest,
		reactor,
		closeConnection,
		checkConnection,
		_timeout;

	connectionError = function() {
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
			clearTimeout(_timeout);
			connected = true;
			alreadyFailed = false;
			sb.emit('connectionSuccess', msg);
		}
		sendLoginRequest(username);
	};
	processMessage = function(e) {
		try {
			var message = JSON.parse(""+e.data);
			console.log(message);
			if(message.message) {
				if(typeof(message.message)==="string") {
					message.message = decodeURIComponent(message.message);
				}
			}
			if(message.status==198) {
				sb.emit('WSreceivedMessage', message);
			} else if(message.status==199) { 
				sb.emit('WSreceivedPrivMessage', message);
			} else if(message.status==195) {
				sb.emit('WSreceivedNotification', message);
			} else if(message.status==103) {
				sb.emit('WSnewPublicRoom', message.message);
			} else if(message.status==105) {
				sb.emit('WSleftRoom', message.message);
			} else if(message.status && !message.sender) {
				sb.emit('WSresponse', message);
			}
		} catch(exc) {
			//ignore message it's not json
			console.log("[DEBUG]: not json:", e.data);
		}
	};
	onCloseConnection = function() {
		clearTimeout(_timeout);
		if(connected) {
			sb.emit('loggedOut', 'Connection closed');
			connected = false;
			alreadyFailed = false;
		}
	};
	checkConnection = function() {
		if(!connected && !alreadyFailed) {
			sb.emit('connectionError', 'timeout');
		}
	};
	sendLoginRequest = function(username) {
		if(connected) {
			sendMessage(encodeURIComponent("/login "+username));
		}
	};
	connect = function(hostname, port) {
		try {
			_sock = new WebSocket('ws://'+hostname+':'+port, ['chat']);
			alreadyFailed=false;
			_timeout = setTimeout(checkConnection, timeout);
			_sock.onerror = connectionError;
			_sock.onopen = connectionSuccess;
			_sock.onmessage = processMessage;
			_sock.onclose = onCloseConnection;
		} catch(e) {
			connectionError(e.message);
		}
	};
	processLoginRequest = function(data) {
		username = data.username;
		connect(data.hostname, data.port);
	};
	closeConnection = function() {
		_sock.close();
		sb.emit('logout');
	};
	reactor = function( data, topic ){
		switch( topic ){
			case "loginRequest":
				processLoginRequest(data);
				break;
			case "WSsendMessage":
				sendMessage(data);
					break;
			case "WSlogout":
				closeConnection();
			}
	};

	return {
	    init: function() {
			sb.on( ['loginRequest', 'WSsendMessage', 'WSlogout'], reactor );
	    },
	    destroy: function() { 
	    }
	};
};