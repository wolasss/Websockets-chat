var MODlogoutbutton = function(sb){
	var button, logout, show;
	
	show = function() {
		sb.toggleModule();
	};
	logout = function() {
		sb.emit('WSsendMessage', "/logout");
	};
	return {
	    init: function() {
	    	button = sb.find(sb.CSSlogoutButton)[0];
	    	sb.addEvent(button, 'click', logout);
	    	sb.on('loggedIn', show);
	    },
	    destroy: function() { 
	    	button = null;
	    }
	};
};