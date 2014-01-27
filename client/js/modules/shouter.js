var MODshouter = function(sb){
	"use strict";

	var input, toggle, reactor, button, send, switchRoom, currentRoom;
	
	toggle = function() {
		sb.toggleModule();
	};

	send = function() {
		var message;

		if(input.value.match(/^\/\w+/)) {
			//command
			if(input.value.match(/\/join\s\w+\s/)) {
				console.log("nieprawidlowa nazwa pokoju");
			}
			message = input.value;
		} else if(input.value[0]=='@') {
			//private
			message = input.value;
		} else {
			//public
			message = currentRoom+" "+input.value;
		}
		//!(equals 0 or only spaces)...
		if(input.value.length!==0 && (!input.value.match(/^\s+$/)) ) {
			sb.emit('WSsendMessage', encodeURIComponent(message));
			input.value='';
		}
	};
	reactor = function(e) {
		if(e.which === 13) {
			//enter key
			send();
		}
	};
	switchRoom = function(data) {
		var command = (data.type === "private") ? '@' : '%';
		currentRoom = command+data.name;
		input.focus();
	};
	return {
	    init: function() {
			currentRoom = '%main';
			input = sb.find(sb.CSSmessageField)[0];
			button = sb.find('label')[0];
			sb.on('loggedIn', toggle);
			sb.on('loggedOut', toggle);
			sb.on('switchRoom', switchRoom);
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