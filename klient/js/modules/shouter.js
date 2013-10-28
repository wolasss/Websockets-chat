var MODshouter = function(sb){
	var input, show, reactor, button, send;
	
	show = function() {
		sb.toggleModule();
	};

	send = function() {
		if(input.value.length!==0) {
			sb.emit('WSsendMessage', input.value);
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
	    	input = sb.find(sb.CSSmessageField)[0];
	    	button = sb.find('label')[0];

	    	sb.on('loggedIn', show);
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