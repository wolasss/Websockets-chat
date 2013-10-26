var MODshouter = function(sb){
	var input, show, reactor;
	
	show = function() {
		sb.toggleModule();
	};

	reactor = function(e) {
		if(e.which === 13) {
			sb.emit('WSsendMessage', input.value);
			input.value='';
		}
	}

	return {
	    init: function() {
	    	input = sb.find(sb.CSSmessageField)[0];

	    	sb.on('loggedIn', show);
	    	sb.addEvent(input, 'keypress', reactor);
	    },
	    destroy: function() { 
	    	input = null;

	    	sb.off('loggedIn');
	    	sb.removeEvent(input, 'keypress', reactor);
	    }
	};
};