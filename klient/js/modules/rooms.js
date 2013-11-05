var MODrooms = function(sb){
	var rooms;

	toggle = function() {
		sb.slideToggle();
	};
	return {
	    init: function() {
	    	rooms = sb.find(sb.CSSrooms)[0];
	    	sb.on('loggedIn', toggle);
	    	sb.on('loggedOut', toggle);
	    },
	    destroy: function() { 
	    	sb.off('loggedIn');
	    	sb.off('loggedOut');
	    	rooms = null;	
	    }
	};
};