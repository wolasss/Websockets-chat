var MODshouter = function(sb){
	var input, toggle, reactor, button, send, changeRoom, currentRoom;
	
	toggle = function() {
		sb.toggleModule();
	};

	send = function() {
		var message;

		if(input.value.match(/^\/\w+/)) {
			//command
			message = input.value;
		} else if(input.value[0]=='@') {
			//private
			message = input.value;
		} else {
			//public
			message = "%"+currentRoom+" "+input.value;
		}

		//!(equals 0 or only spaces)...
		if(input.value.length!==0 && (!input.value.match(/^\s+$/)) ) {
			sb.emit('WSsendMessage', message);
			input.value='';
		}
	}

	reactor = function(e) {
		if(e.which === 13) {
			send();
		}
	}

	return {
	    init: function() {
	    	currentRoom = 'main';
	    	input = sb.find(sb.CSSmessageField)[0];
	    	button = sb.find('label')[0];

	    	sb.on('loggedIn', toggle);
	    	sb.on('loggedOut', toggle);
	    	sb.on('roomChange', changeRoom)
	    	sb.addEvent(input, 'keypress', reactor);
	    	sb.addEvent(button, 'click', send);
	    },
	    destroy: function() { 
	    	input = null;

	    	sb.off('loggedIn');
	    	sb.removeEvent(input, 'keypress', reactor);
	    }
	};
};