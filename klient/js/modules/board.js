var MODboard = function(sb){
	
	var show = function() {
		sb.addClass(sb.CSSchat, 'expanded');
		sb.toggleModule();
	}

	return {
	    init: function() {
	    	sb.on('loggedIn', show);
	    },
	    destroy: function() { 
	    	
	    }
	};
};